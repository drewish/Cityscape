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

#include <CGAL/Arr_observer.h>
#include <CGAL/Sweep_line_2_algorithms.h>

using namespace ci;

namespace Cityscape {

/*
 * These classes track an arrangement, noting holes, and tracks its division. When
 * faces are split they checks if the face was a hole or not and marks the newly
 * split faces accordingly.
 */

struct OutlineObserver : public CGAL::Arr_observer<Arrangement_2> {
    OutlineObserver( Arrangement_2& arr ) : CGAL::Arr_observer<Arrangement_2>( arr ) {
        arr.unbounded_face()->set_data( FaceRole::Hole );
    };
    virtual void after_split_face( Face_handle oldFace, Face_handle newFace, bool isHoleInOld ) {
        newFace->set_data( FaceRole::Lot );
    }
};

struct HoleObserver : public CGAL::Arr_observer<Arrangement_2> {
    HoleObserver( Arrangement_2& arr ) : CGAL::Arr_observer<Arrangement_2>( arr ) {};
    // in before_split_face mark all new edges as street facing
    virtual void before_split_face ( Face_handle oldFace, Halfedge_handle newEdge ) {
        newEdge->set_data( EdgeRole::Street );
        newEdge->twin()->set_data( newEdge->data() );
    }
    // in after_split_face mark new faces as holes
    virtual void after_split_face( Face_handle oldFace, Face_handle newFace, bool isHoleInOld ) {
        newFace->set_data( FaceRole::Hole );
    }
};

struct DividerObserver : public CGAL::Arr_observer<Arrangement_2> {
    DividerObserver( Arrangement_2& arr ) : CGAL::Arr_observer<Arrangement_2>( arr ) {};
    // in before_split_face mark all new edges as interior
    virtual void before_split_face ( Face_handle oldFace, Halfedge_handle newEdge ) {
        newEdge->set_data( EdgeRole::Interior );
        newEdge->twin()->set_data( newEdge->data() );
    }
    // in after_split_face mark new faces' data = old faces'
    virtual void after_split_face( Face_handle oldFace, Face_handle newFace, bool isHoleInOld ) {
        newFace->set_data( oldFace->data() );
    }
};

Arrangement_2 arrangementFor( const FlatShapeRef shape ) {
    Arrangement_2 arr;

    // add outline to arrangement
    OutlineObserver outObs( arr );
    std::list<Segment_2> outlineSegments = contiguousSegmentsFrom( shape->outline().getPoints() );
    insert_empty( arr, outlineSegments.begin(), outlineSegments.end() );
    outObs.detach();

    if ( shape->holes().empty() ) return arr;

    // add holes to arrangement
    HoleObserver holeObs( arr );
    std::vector<Segment_2> holeSegments;
    for ( const auto &hole : shape->holes() ) {
        contiguousSegmentsFrom( hole.getPoints(), back_inserter( holeSegments ) );
    }
    insert( arr, holeSegments.begin(), holeSegments.end() );
    holeObs.detach();

    return arr;
}

LotRef lotFrom( const Arrangement_2::Face_iterator &face ) {
    LotRef lot = Lot::create( FlatShape::create( face ) );

    // Start going around looking for a edge that faces a road.
    Arrangement_2::Ccb_halfedge_circulator edge = face->outer_ccb();
    Arrangement_2::Ccb_halfedge_circulator cc = edge;
    bool leftStart = false;
    while ( !( leftStart && cc == edge ) && ( cc->twin()->face()->data() == FaceRole::Lot ) ) {
        ++cc;
        leftStart = true;
    }
    // If we find a road copy all we find until we get back to the
    // start.
    if ( cc->twin()->face()->data() == FaceRole::Hole ) {
        do {
            lot->streetFacingSides.push_back( seg2(
                vecFrom( cc->source()->point() ),
                vecFrom( cc->target()->point() )
            ) );
        } while ( ++cc != edge && cc->twin()->face()->data() == FaceRole::Hole );
    }

    return lot;
}

std::vector<LotRef> slice( const LotRef lot, const seg2 &divider ) {
    Arrangement_2 arr = arrangementFor( lot->shape );

    // add divider
    DividerObserver divObs( arr );
    insert( arr, segmentFrom( divider ) );
    divObs.detach();

    // extract lots
    std::vector<LotRef> ret;
    for ( auto face = arr.faces_begin(); face != arr.faces_end(); ++face ) {
        if ( face->is_unbounded() || face->data() == FaceRole::Hole ) continue;

        ret.push_back( lotFrom( face ) );
    }
    return ret;
}


void noopSubdivide( const ZoningPlan::BlockOptions &options, BlockRef &block )
{
    LotRef lot = Lot::create( block->shape );

    contiguousSeg2sFrom( block->shape->outline().getPoints(), std::back_inserter( lot->streetFacingSides ) );
    for ( auto &hole : block->shape->holes() ) {
        contiguousSeg2sFrom( hole.getPoints(), std::back_inserter( lot->streetFacingSides ) );
    }

    block->lots.push_back( lot );
}


// Does a poor job of implementing the OOB algorithm described in:
// Procedural Generation of Parcels in Urban Modeling
// Carlos A. Vanegas, Tom Kelly, Basil Weber, Jan Halatsch, Daniel G. Aliaga, Pascal Müller
// http://www.twak.co.uk/2011/12/procedural-generation-of-parcels-in.html
void oobSubdivide( const ZoningPlan::BlockOptions &options, BlockRef &block )
{
    std::queue<LotRef> toSplit;
    if ( block->shape->area() > options.lotAreaMax ) {
        toSplit.push( Lot::create( block->shape ) );
    } else {
        block->lots.push_back( Lot::create( block->shape ) );
    }

std::cout << "need to be between " << options.lotAreaMin << " - " << options.lotAreaMax << "\n";
    while ( !toSplit.empty() ) {
        LotRef lot = toSplit.front();
std::cout << "\n";

        // Figure out the minimum bounding box so we can try a few divisions
        const ci::PolyLine2f &outline = lot->shape->outline();
        ci::Rectf bounds;
        float rotate = 0;
        minimumOobFor( outline, bounds, rotate );

        int tries = 4;
        bool tooSmall = true;
        std::vector<LotRef> splitLots;
        do {
            float fraction = randFloat( 0.45, 0.5 );
            seg2 divider = oobDivider( bounds, rotate, fraction );
            splitLots = slice( lot, divider );
            tooSmall = std::any_of( begin( splitLots ), end( splitLots ), [&]( const LotRef &lot ) {
std::cout << "part area: " << lot->shape->area() << "\n";
                return lot->shape->area() < options.lotAreaMin;
            } );
        } while ( tooSmall && --tries > 0 );

        if ( tooSmall ) {
std::cout << "failed on area: " << lot->shape->area() << "\t" << tries << "\n";
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

        toSplit.pop();
    }
}

void skeletonSubdivide( const ZoningPlan::BlockOptions &options, BlockRef &block )
{
    // For closed outlines, we need at least 4 points.
    if ( block->shape->outline().size() < 4 ) return;

    // Build straight skeleton with holes
    SsPtr skel = CGAL::create_interior_straight_skeleton_2( block->shape->polygonWithHoles<InexactK>() );

    std::list<Segment_2> skeletonSegments;

    float dividerAngle = 0;
    float maxLength = 0;

    for ( auto edge = skel->halfedges_begin(); edge != skel->halfedges_end(); ++edge ) {
        const auto &currVert = edge->vertex(),     &nextVert = edge->next()->vertex();
        const auto &currPoint = currVert->point(), &nextPoint = nextVert->point();

        if ( currVert->is_skeleton() && nextVert->is_skeleton() ) {
            Segment_2 seg( Point_2( currPoint.x(), currPoint.y() ), Point_2( nextPoint.x(), nextPoint.y() ) );

            // Find the angle of the longest skeleton segment edge.
            vec2 vec = vecFrom( currPoint ) - vecFrom( nextPoint );
            float length = glm::length2( vec );
            if ( length > maxLength ) {
                // Find the perpendicular angle.
                dividerAngle = -atan2( vec.y, vec.x );
                maxLength = length;
            }

            // The skeleton has half edges going both directions for each segment in
            // the skeleton. We only need one so before putting a->b in check that
            // b->a isn't already in there.
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


    Arrangement_2 arr = arrangementFor( block->shape );

    DividerObserver divObs( arr );

    insert( arr, skeletonSegments.begin(), skeletonSegments.end() );

    // TODO: would be good to adjust the dividers to:
    // - create lots in specific size ranges (avoid tiny or mega lots)

    // Then start walking across the outline adding dividers.
    std::vector<Segment_2> dividerSegments = block->shape->dividerSegment_2s( dividerAngle, options.lotWidth );

    // Put the adjusted skeleton, and new dividers into the the arrangment.
    insert( arr, dividerSegments.begin(), dividerSegments.end() );

    divObs.detach();

    for ( auto face = arr.faces_begin(); face != arr.faces_end(); ++face ) {
        if ( face->is_unbounded() || face->data() == FaceRole::Hole ) continue;

        block->lots.push_back( lotFrom( face ) );
    }
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
