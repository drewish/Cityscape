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

using namespace ci;

namespace Cityscape {

void subdivideNotReally( BlockRef block, const Options &options );
void subdivideSkeleton( BlockRef block, const Options &options );

// in Blocks
// out Lots
void subdivideBlocks( CityModel &city )
{
    for ( const auto &district : city.districts ) {
        for ( const auto &block : district->blocks ) {

            // Don't bother dividing small blocks
            if ( block->shape->area() < 100 ) {
                subdivideNotReally( block, city.options );
            }
            else if ( city.options.block.division == BlockOptions::BLOCK_DIVIDED ) {
                subdivideSkeleton( block, city.options );
            }
            else { // BlockOptions::NO_BLOCK_DIVISION
                subdivideNotReally( block, city.options );
            }
        }
    }
}

// Use the entire block for a lot.
void subdivideNotReally( BlockRef block, const Options &options )
{
    block->lots.push_back( Lot::create( block->shape ) );
}

// TODO: This needs work to handle the holes correctly
Arrangement_2 arrangementSubdividing( const FlatShape &shape, const int16_t lotWidth )
{
    Arrangement_2 arrangement;

    // TODO: find out why 4? because it's closed?
    if (shape.outline().size() < 4) return arrangement;

    // Build straight skeleton...
    // Create with holes
    SsPtr skel = CGAL::create_interior_straight_skeleton_2( shape.polygonWithHoles<InexactK>() );
//    // Avoid holes do we don't have to skip the face for the hole when reading out the lots
//    SsPtr skel = CGAL::create_interior_straight_skeleton_2( shape.polygonWithConnectedHoles(), InexactK() );

    std::list<Segment_2> outlineSegments;
    std::list<Segment_2> skeletonSegments;

    float dividerAngle = 0;
    float maxLength = 0;

    for ( auto edge = skel->halfedges_begin(); edge != skel->halfedges_end(); ++edge ) {
        const auto &currVert = edge->vertex(),     &nextVert = edge->next()->vertex();
        const auto &currPoint = currVert->point(), &nextPoint = nextVert->point();
        Segment_2 seg( Point_2( currPoint.x(), currPoint.y() ), Point_2( nextPoint.x(), nextPoint.y() ) );

        if ( edge->is_border() ) {
            // Use the skeleton's outline since, unlike mOutline, will always
            // be a closed polygon.
            outlineSegments.push_back( seg );
        }

        if ( currVert->is_skeleton() && nextVert->is_skeleton() ) {
            // Find the angle of the longest skeleton segment edge.
            vec2 vec = vecFrom( currPoint ) - vecFrom( nextPoint );
            float length = glm::length2( vec );
            if ( length > maxLength ) {
                // Find the perpendicular angle.
                dividerAngle = atan2( vec.y, -vec.x );
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

    // Adjust the skeleton so it intersects with the outline rather than doing
    // it's normal split thing.

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

    // TODO: would be good to adjust the dividers to:
    // - create lots in specific size ranges (avoid tiny or mega lots)

    // Then start walking across the outline adding dividers.
    std::list<Segment_2> dividerSegments;
    std::vector<vec2> dividers = computeDividers( shape.outline().getPoints(), dividerAngle, lotWidth );
    for ( const Segment_2 &divider : segmentsFrom( dividers ) ) {
        outlineSegments.push_back( divider );

        // The intersection points come back in a sorted order so we can just
        // create a series of segments from those points.
        std::vector<Point_2> dividerPoints;
        CGAL::compute_intersection_points( outlineSegments.begin(), outlineSegments.end(), std::back_inserter( dividerPoints ) );
        for ( const Segment_2 &dividerChunk : segmentsFrom( dividerPoints ) ) {
            dividerSegments.push_back( dividerChunk );
        }

        outlineSegments.pop_back();
    }

    // Put the outline, adjusted skeleton, and new dividers into the the
    // arrangment.
    insert_empty( arrangement, outlineSegments.begin(), outlineSegments.end() );
    insert( arrangement, skeletonSegments.begin(), skeletonSegments.end() );
    insert( arrangement, dividerSegments.begin(), dividerSegments.end() );

    return arrangement;
}

void subdivideSkeleton( BlockRef block, const Options &options )
{
    Arrangement_2 mArr = arrangementSubdividing( *block->shape, options.block.lotWidth );

    // TODO: this doesn't exclude the faces that are holes in the initial shape
    for ( auto face = mArr.faces_begin(); face != mArr.faces_end(); ++face ) {
        for ( auto j = face->outer_ccbs_begin(); j != face->outer_ccbs_end(); ++j ) {
            PolyLine2f lotOutline;
            Arrangement_2::Ccb_halfedge_circulator cc = *j;
            do {
                Arrangement_2::Halfedge_handle he = cc;
                lotOutline.push_back( vecFrom( he->target()->point() ) );
            } while ( ++cc != *j );

            block->lots.push_back( Lot::create( FlatShape::create( lotOutline ) ) );
        }
    }
}


} // Cityscape namespace
