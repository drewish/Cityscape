//
//  Block.cpp
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#include "cinder/Rand.h"

#include "Block.h"
#include "Lot.h"
#include "GeometryHelpers.h"

#include "CinderCGAL.h"
#include <CGAL/create_straight_skeleton_from_polygon_with_holes_2.h>
#include <CGAL/Polygon_set_2.h>

using namespace ci;


typedef CGAL::Straight_skeleton_2<InexactK> Ss;
typedef boost::shared_ptr<Ss> SsPtr;

typedef std::map<std::pair<float, float>, vec3> OffsetMap;

void Block::layout( const Options &options )
{
    mLots.clear();

    // Don't bother dividing small blocks
    if ( options.block.division == BlockOptions::BLOCK_DIVIDED && mShape.outline().calcArea() >= 100 ) {
        subdivideSkeleton( options );
    }
    else { // BlockOptions::NO_BLOCK_DIVISION
        subdivideNotReally( options );
    }

    for ( auto &lot : mLots ) {
        lot.get()->layout( options );
    }
}

void Block::draw( const Options &options ) const
{
    if ( options.drawBlocks ) {
        gl::color( ColorA( 0.0f, 0.8f, 0.2f, 0.5f ) );
        gl::draw( mShape.mesh() );
    }

    // Two passes is tacky but the goal is to stack the drawing such that lots
    // are atop blocks...
    for ( auto &lot : mLots ) {
        lot.get()->drawGround( options );
    }
    // ...and buildings are on top of everything else.
    for ( auto &lot : mLots ) {
        lot.get()->drawStructures( options );
    }
}

// Use the entire block for a lot.
void Block::subdivideNotReally( const Options &options )
{
    ColorA color( CM_HSV, 1, 1.0, 0.75, 0.5 );
    LotRef lot = LotRef( new SingleBuildingLot( mShape.outline(), color ) );
    mLots.push_back( lot );
}

Arrangement_2 Block::arrangementSubdividing( const FlatShape &shape, const int16_t lotWidth )
{
    Arrangement_2 arrangement;

    if (shape.outline().size() < 4) return arrangement;

    float angle = 0;

    // Build straight skeleton
    // TODO figure out why we need to reverse this...
    CGAL::Polygon_2<InexactK> poly = shape.polygon<InexactK>();
    if ( poly.is_clockwise_oriented() ) {
        poly.reverse_orientation();
    }
    SsPtr skel = CGAL::create_interior_straight_skeleton_2( poly, InexactK() );

    std::list<Segment_2> outlineSegments;
    std::list<Segment_2> skeletonSegments;

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
            vec2 v = vecFrom( nextPoint ) - vecFrom( currPoint );
            // TODO: see if we can get a lengthSquared() to avoid the sqrt() call.
            float newLen = glm::length( v );
            if ( newLen > maxLength ) {
                maxLength = newLen;
                // Reverse the x and y to get the perpendicular angle.
                angle = atan2( v.y , v.x );
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
    for ( const Segment_2 &divider : segmentsFrom( computeDividers( shape.outline().getPoints(), angle, lotWidth ) ) ) {
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

void Block::subdivideSkeleton( const Options &options )
{
    float hue = 0.0;

    mArr = arrangementSubdividing( mShape, options.block.lotWidth );

    for( auto face = mArr.faces_begin(); face != mArr.faces_end(); ++face ) {
        for ( auto j = face->outer_ccbs_begin(); j != face->outer_ccbs_end(); ++j ) {
            PolyLine2f lotOutline;
            Arrangement_2::Ccb_halfedge_circulator cc = *j;
            do {
                Arrangement_2::Halfedge_handle he = cc;
                lotOutline.push_back( vecFrom( he->target()->point() ) );
            } while ( ++cc != *j );

            // Skip small lots
            if ( lotOutline.calcArea() < 10 ) continue;

            ColorA c( CM_HSV, hue, 1.0, 0.75, 0.5 );
            LotRef l;
            // TODO this weight should become an option
            if ( randInt(5) == 0 ) {
                l = LotRef( new EmptyLot( lotOutline, c ) );
            }
            // TODO these options need to be moved and renamed
            else if ( options.lot.buildingPlacement == LotOptions::BUILDING_FILL_LOT ) {
                l = LotRef( new FilledLot( lotOutline, c ) );
            }
            else if ( options.lot.buildingPlacement == LotOptions::BUILD_PARK ) {
                l = LotRef( new ParkLot( lotOutline, c ) );
            }
            else {
                l = LotRef( new SingleBuildingLot( lotOutline, c ) );
            }
            mLots.push_back( l );

            hue += 0.17;
            if (hue > 1) hue -= 1.0;
        }
    }
}