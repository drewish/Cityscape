//
//  FlatShape.cpp
//  Cityscape
//
//  Created by andrew morton on 5/20/15.
//
//

#include "FlatShape.h"
#include "cinder/Rand.h"
#include "cinder/Triangulate.h"
#include <CGAL/Sweep_line_2_algorithms.h>
#include <CGAL/arrange_offset_polygons_2.h>
#include <CGAL/create_offset_polygons_from_polygon_with_holes_2.h>
#include "GeometryHelpers.h"


using namespace ci;

void FlatShape::fixUp()
{
    // Look for the same point repeated...
    std::vector<vec2> &points = mOutline.getPoints();
    if ( std::adjacent_find( points.begin(), points.end() ) != points.end() ) {
        // ...and remove duplicates since CGAL wants simple polygons.
        auto newEnd = std::unique_copy( points.begin(), points.end(), points.begin() );
        points.resize( std::distance( points.begin(), newEnd ) );
    }

    // Make sure the orientation is correct.
    if ( mOutline.isClockwise( nullptr ) ) mOutline.reverse();
    for ( auto &hole : mHoles ) {
        if ( hole.isCounterclockwise() ) hole.reverse();
    }
}

vec2 FlatShape::centroid() const
{
    return mOutline.calcCentroid();
}

vec2 FlatShape::randomPoint() const
{
    Rectf bounds = boundingBox();
    vec2 point;
    do {
        point = vec2( randFloat( bounds.x1, bounds.x2 ), randFloat( bounds.y1, bounds.y2 ) );
    } while ( ! mOutline.contains( point ) );
    return point;
}

bool FlatShape::contains( const ci::vec2 point ) const
{
    if ( !mOutline.contains( point ) ) {
        return false;
    }

    for ( const auto &hole : mHoles ) {
        if ( hole.contains( point ) ) {
            return false;
        }
    }

    return true;
}

const ci::TriMeshRef FlatShape::mesh() const
{
    if ( mMesh ) return mMesh;

    Triangulator triangulator( mOutline );
    for( const auto &hole : mHoles ) {
        triangulator.addPolyLine( hole );
    }

    return mMesh = triangulator.createMesh();
}

float FlatShape::calcArea() const
{
    float area = mOutline.calcArea();
    for ( const auto &hole : mHoles ) {
        area -= hole.calcArea();
    }
    return area;
}

std::vector<seg2> FlatShape::dividerSeg2s( float angle, float spacing ) const
{
    std::vector<seg2> results;

    for ( const auto &segment : dividerSegment_2s( angle, spacing ) ) {
        results.push_back( seg2( vecFrom( segment.source() ), vecFrom( segment.target() ) ) );
    }

    return results;
}

std::vector<Segment_2> FlatShape::dividerSegment_2s( float angle, float spacing ) const
{
    assert( spacing > 0 );

    std::list<Segment_2> outlineSegments = contiguousSegmentsFrom( mOutline.getPoints() );
    for ( const auto &hole : mHoles ) {
        auto holeSegments = contiguousSegmentsFrom( hole.getPoints() );
        outlineSegments.insert( outlineSegments.end(), holeSegments.begin(), holeSegments.end() );
    }

    // Walking across the shape and find the portion of the divider that is
    // inside the shape.
    std::vector<Segment_2> results;
    for ( const seg2 &divider : computeDividers( mOutline.getPoints(), angle, spacing ) ) {
        outlineSegments.push_back( Segment_2( pointFrom( divider.first ), pointFrom( divider.second ) ) );

        // The intersection points come back in a sorted order so we can just
        // create a series of segments from those points.
        std::vector<Point_2> dividerPoints;
        CGAL::compute_intersection_points( outlineSegments.begin(), outlineSegments.end(), std::back_inserter( dividerPoints ) );
        std::list<Segment_2> segments = segmentsFrom( dividerPoints );
        results.insert( results.end(), segments.begin(), segments.end() );

        outlineSegments.pop_back();
    }

    return results;
}

typedef CGAL::Polygon_with_holes_2<InexactK> PolyWithHoles;
typedef boost::shared_ptr<PolyWithHoles> PolyPtr;

// For concave shapes contraction might split the shape into multiple smaller shapes:
//   +-----+
//    \    |   +-+
//     \   |    \|
//      \  |
//      /  |
//     /   |    /|
//    /    |   +-+
//   +-----+
std::vector<FlatShape> FlatShape::contract( double offset ) const
{
    // Negative or zero offset is a noop
    if ( offset <= 0 ) {
        return std::vector<FlatShape>( { *this } );
    }
    // Avoid computing it for small shapes
    if ( mOutline.size() < 3 || mArea < offset * offset ) {
        return std::vector<FlatShape>();
    }

    PolyWithHoles input = polygonWithHoles<InexactK>();
    std::vector<PolyPtr> outline = CGAL::create_interior_skeleton_and_offset_polygons_with_holes_2( offset, input );
    std::vector<FlatShape> results;
    for ( const auto &p : outline ) {
        results.push_back( FlatShape( *p ) );
    }
    return results;
}
