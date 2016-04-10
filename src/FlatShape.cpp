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
#include <CGAL/linear_least_squares_fitting_2.h>
#include <CGAL/connect_holes.h>
#include <CGAL/arrange_offset_polygons_2.h>
#include <CGAL/create_offset_polygons_from_polygon_with_holes_2.h>
#include "GeometryHelpers.h"


using namespace ci;

void FlatShape::fixOrientation()
{
    if ( mOutline.isClockwise() ) mOutline.reverse();
    for ( auto &hole : mHoles ) {
        if ( hole.isCounterClockwise() ) hole.reverse();
    }
}

vec2 FlatShape::centroid() const
{
    CGAL::Polygon_2<InexactK> p( polygonFrom<InexactK>( mOutline ) );
    InexactK::Point_2 centroid = InexactK::Point_2(0, 0);
    InexactK::Line_2 line;
    CGAL::Dimension_tag<0> dt;
    CGAL::linear_least_squares_fitting_2( p.vertices_begin(), p.vertices_end(), line, centroid, dt);

    return vecFrom( centroid );
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

CGAL::Polygon_2<InexactK> FlatShape::polygonWithConnectedHoles() const
{
    std::vector<ExactK::Point_2> points;

    CGAL::connect_holes( polygonWithHoles<ExactK>(), std::back_inserter( points ) );

    CGAL::Polygon_2<InexactK> result;
    for ( auto &p : points ) {
        result.push_back( InexactK::Point_2( p.x().floatValue(), p.y().floatValue() ) );
    }
    return result;
}

ci::PolyLine2f FlatShape::polyLineWithConnectedHoles() const
{
    std::vector<CGAL::Point_2<ExactK>> points;

    CGAL::connect_holes( polygonWithHoles<ExactK>(), std::back_inserter( points ) );

    return polyLineFrom<ExactK>( points );
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
    std::vector<Segment_2> results;

    std::list<Segment_2> outlineSegments = contiguousSegmentsFrom( mOutline.getPoints() );
    for ( const auto &hole : mHoles ) {
        auto holeSegments = contiguousSegmentsFrom( hole.getPoints() );
        outlineSegments.insert( outlineSegments.end(), holeSegments.begin(), holeSegments.end() );
    }

    // Walking across the shape and find the portion of the divider that is
    // inside the shape.
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

FlatShape FlatShape::contract( double offset ) const
{
    if ( mOutline.size() < 3 ) return *this;
    if ( offset <= 0 ) return *this;

    PolyWithHoles input = polygonWithHoles<InexactK>();
    printPolygon( input );
    std::vector<PolyPtr> outline = CGAL::create_interior_skeleton_and_offset_polygons_with_holes_2( offset, input );
    return FlatShape( *outline.back() );
}
