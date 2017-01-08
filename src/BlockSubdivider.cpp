//
//  BlockSubdivider.cpp
//  Cityscape
//
//  Created by Andrew Morton on 3/6/16.
//
//

#include "BlockSubdivider.h"

#include "FlatShape.h"
#include "CgalArrangement.h"
#include "CgalStraightSkeleton.h"
#include "GeometryHelpers.h"

#include "cinder/Rand.h"

#include <CGAL/Arr_overlay_2.h>

using namespace ci;

namespace Cityscape {

LotRef lotFrom( const Arrangement_2::Face_iterator &face ) {
    LotRef lot = Lot::create( FlatShape::create( face ) );

    // TODO: should look at holes too
    Arrangement_2::Ccb_halfedge_circulator start = face->outer_ccb(), cc = start;
    do {
        if ( cc->data() != EdgeRole::Exterior ) continue;

        seg2 s = seg2( vecFrom( cc->source()->point() ), vecFrom( cc->target()->point() ) );
        lot->streetFacingSides.push_back( s );
    } while ( ++cc != start );

    return lot;
}

void printArrangement( const Arrangement_2 &arr ) {
    std::cout << "\n\nArrangement\n";
    for ( auto e = arr.edges_begin(); e != arr.edges_end(); ++e ) {
        std::cout << "(" << e->source()->point() << ", " << e->target()->point() << ") = " << static_cast<uint>(e->data()) <<  "\n";
    }
}

std::vector<LotRef> slice( const Arrangement_2 &arrShape, const Arrangement_2 &arrDividers ) {
    // Overlay the two arrangements
    Arrangement_2 arrOverlay;
    Arr_extended_overlay_traits overlay_traits;
    overlay( arrShape, arrDividers, arrOverlay, overlay_traits );

    // Extract lots
    std::vector<LotRef> ret;
    for ( auto face = arrOverlay.faces_begin(); face != arrOverlay.faces_end(); ++face ) {
        if ( !face->is_unbounded() && face->data() != FaceRole::Hole ) {
            ret.push_back( lotFrom( face ) );
        }
    }
    return ret;
}

LotRef lotFilling( const BlockRef &block ) {
    LotRef lot = Lot::create( block->shape );
    contiguousSeg2sFrom( block->shape->outline(), std::back_inserter( lot->streetFacingSides ) );
    for ( auto &hole : block->shape->holes() ) {
        contiguousSeg2sFrom( hole, std::back_inserter( lot->streetFacingSides ) );
    }
    return lot;
}

Arrangement_2 lotArrangement( const LotRef &lot ) {
    Arrangement_2 arrLot = lot->shape->arrangement();
    // Default edge role to divider, the override outside
    setEdgeRoles( arrLot, EdgeRole::Divider );

    for ( auto e = arrLot.halfedges_begin(); e != arrLot.halfedges_end(); ++e ) {
        seg2 needle = seg2( vecFrom( e->source()->point() ), vecFrom( e->target()->point() ) );
        bool inThere = std::any_of( lot->streetFacingSides.begin(), lot->streetFacingSides.end(), [&needle]( seg2 hay ) {
            return ( hay.first == needle.first && hay.second == needle.second );
        } );
        if ( inThere ) {
            e->set_data( EdgeRole::Exterior );
            e->twin()->set_data( EdgeRole::Exterior );
        }
    }

    return arrLot;
}

void noopSubdivide( const ZoningPlan::BlockOptions &options, BlockRef &block )
{
    block->lots.push_back( lotFilling( block ) );
}

// Does a poor job of implementing the OOB algorithm described in:
// Procedural Generation of Parcels in Urban Modeling
// Carlos A. Vanegas, Tom Kelly, Basil Weber, Jan Halatsch, Daniel G. Aliaga, Pascal Müller
// http://www.twak.co.uk/2011/12/procedural-generation-of-parcels-in.html
void oobSubdivide( const ZoningPlan::BlockOptions &options, BlockRef &block )
{
    std::queue<LotRef> toSplit;

    if ( block->shape->area() > options.lotAreaMax ) {
        toSplit.push( lotFilling( block ) );
    } else {
        block->lots.push_back( lotFilling( block ) );
    }

    while ( !toSplit.empty() ) {
        LotRef lot = toSplit.front();
        toSplit.pop();

        Arrangement_2 arrLot = lotArrangement( lot );

        // Figure out some oriented bounding boxes so we can try a few divisions
        const ci::PolyLine2f &outline = lot->shape->outline();
        std::vector< std::pair<float, ci::Rectf> > oobs = oobsFor( outline );

        // TODO: we should make a few tries with one OOB and if it doesn't work
        // try the next couple.
        std::pair<float, ci::Rectf> oob = oobs.front();
        int tries = 4;
        bool tooSmall = true;
        std::vector<LotRef> splitLots;
        do {
            float fraction = randFloat( 0.45, 0.5 );

            // TODO: Move to a function
            Segment_2 divider = segmentFrom( oobDivider( oob.second, oob.first, fraction ) );
            Arrangement_2 arrDiv;
            auto edge = insert_non_intersecting_curve( arrDiv, divider );
            edge->set_data( EdgeRole::Divider );
            edge->twin()->set_data( EdgeRole::Divider );

            splitLots = slice( arrLot, arrDiv );
            tooSmall = std::any_of( begin( splitLots ), end( splitLots ), [&]( const LotRef &lot ) {
                return lot->shape->area() < options.lotAreaMin;
            } );
        } while ( tooSmall && --tries > 0 );

        if ( tooSmall ) {
            block->lots.push_back( lot );
        } else {
            for ( auto &part : splitLots ) {
                if ( part->shape->area() > options.lotAreaMax ) {
                    toSplit.push( part );
                } else {
                    block->lots.push_back( part );
                }
            }
        }
    }
}

void skeletonSubdivide( const ZoningPlan::BlockOptions &options, BlockRef &block )
{
    // For closed outlines, we need at least 4 points.
    if ( block->shape->outline().size() < 4 ) return;

    // Build straight skeleton with holes
    SsPtr skel = CGAL::create_interior_straight_skeleton_2( block->shape->polygonWithHoles<InexactK>() );

    // Find the segments that make up the skeleton (ignoring edges connecting
    // to the outline).
    std::vector<Segment_2> skeletonSegments;
    for ( auto edge = skel->halfedges_begin(); edge != skel->halfedges_end(); ++edge ) {
        const auto &currVert = edge->vertex(),     &nextVert = edge->next()->vertex();
        const auto &currPoint = currVert->point(), &nextPoint = nextVert->point();

        if ( currVert->is_skeleton() && nextVert->is_skeleton() ) {
            Segment_2 seg( Point_2( currPoint.x(), currPoint.y() ), Point_2( nextPoint.x(), nextPoint.y() ) );

            // Interating will visit halfedges going both ways in the skeleton,
            // but we don't want twins. So before adding a->b, check that b->a
            // isn't already in there.
            auto isReverseOf = [seg]( const Segment_2 &other ) {
                return other.source() == seg.target() && other.target() == seg.source();
            };
            if ( none_of( skeletonSegments.begin(), skeletonSegments.end(), isReverseOf ) ) {
                skeletonSegments.push_back( seg );
            }
        }
    }

    // Rather than two      |  |  |  We want one segment    |  |  |
    // segments forking out |  *  |  going to the outline:  |  *  |
    // to the corners:      | / \ |                         |  |  |
    //                      |/   \|                         |  |  |
    //                      *-----*                         *--*--*

    // Find faces with 3 edges: 1 skeleton and 2 contour
    for ( auto face = skel->faces_begin(); face != skel->faces_end(); ++face ) {
        // Move around the face until we get to an edge with a skeleton
        // (seems to be the second edge).
        Ss::Halfedge_handle skelEdge = face->halfedge();
        do {
            skelEdge = skelEdge->next();
        } while ( !skelEdge->vertex()->is_skeleton() );

        // Bail if we don't have two contour verts followed by the skeleton vert.
        Ss::Halfedge_handle contourA = skelEdge->next();
        Ss::Halfedge_handle contourB = contourA->next();
        if ( !contourA->vertex()->is_contour() ) continue;
        if ( !contourB->vertex()->is_contour() ) continue;
        if ( contourB->next() != skelEdge ) continue;

        // Find point where skeleton vector intersects contour edge.
        // TODO: would be better to extend the skeleton out to the edge and find
        // that interesection point rather than picking the midpoint of the edge.
        auto a = contourA->vertex()->point();
        auto b = contourB->vertex()->point();
        Point_2 adj( ( b.x() + a.x() ) / 2.0, ( b.y() + a.y() ) / 2.0 );

        // Create a segment for the adjusted edge
        auto c = skelEdge->vertex()->point();
        skeletonSegments.push_back( Segment_2( Point_2( c.x(), c.y() ), adj ) );
    }

    Arrangement_2 arrBlock = block->shape->arrangement();
    setEdgeRoles( arrBlock, EdgeRole::Exterior );

    // Find the angle of the longest skeleton segment edge.
    float dividerAngle = 0;
    float maxLength = 0;
    for ( const Segment_2 &seg : skeletonSegments ) {
        vec2 vec = vecFrom( seg.source() ) - vecFrom( seg.target() );
        float length = glm::length2( vec );
        if ( length > maxLength ) {
            // Find the perpendicular angle.
            dividerAngle = -atan2( vec.y, vec.x );
            maxLength = length;
        }
    }

    // Then start walking across the outline adding dividers.
    // TODO: would be good to adjust the dividers to:
    // - create lots in specific size ranges (avoid tiny or mega lots)
    Arrangement_2 arrDividers;
    for ( const seg2 &divider : computeDividers( block->shape->outline().getPoints(), dividerAngle, options.lotWidth ) ) {
        Segment_2 segment = Segment_2( pointFrom( divider.first ), pointFrom( divider.second ) );
        insert_non_intersecting_curve( arrDividers, segment );
    }
    insert( arrDividers, skeletonSegments.begin(), skeletonSegments.end() );
    setEdgeRoles( arrDividers, EdgeRole::Divider );

    block->lots = slice( arrBlock, arrDividers );
}

// in Blocks
// out Lots
void subdivideBlocks( CityModel &city )
{
    for ( auto &district : city.districts ) {
        ZoningPlanRef zoning = district->zoningPlan;

        for ( auto &block : district->blocks ) {
            ZoningPlan::LotDivision d = zoning->block.lotDivision;

            // Don't bother dividing small blocks
            if ( block->shape->area() < 100 ) {
                d = ZoningPlan::LotDivision::NO_LOT_DIVISION;
            }

            if ( d == ZoningPlan::LotDivision::OOB_LOT_DIVISION ) {
                oobSubdivide( zoning->block, block );
            }
            else if ( d == ZoningPlan::LotDivision::SKELETON_LOT_DIVISION ) {
                skeletonSubdivide( zoning->block, block );
            }
            else {
                noopSubdivide( zoning->block, block );
            }
        }
    }
}

} // Cityscape namespace
