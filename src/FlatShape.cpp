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

using namespace ci;

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

const TriMesh FlatShape::makeMesh()
{
    // TODO might be good to lazily create this when they first ask for the mesh.
    Triangulator triangulator( mOutline );
    for( auto it = mHoles.begin(); it != mHoles.end(); ++it ) {
        triangulator.addPolyLine( *it );
    }

    return triangulator.calcMesh();
}

const CGAL::Polygon_2<InexactK> FlatShape::polygonWithConnectedHoles() const
{
    std::vector<ExactK::Point_2> points;

    CGAL::connect_holes( polygonWithHoles<ExactK>(), std::back_inserter( points ) );

    CGAL::Polygon_2<InexactK> result;
    for ( auto &p : points ) {
        result.push_back( InexactK::Point_2( p.x().floatValue(), p.y().floatValue() ) );
    }
    std::cout << "is simple" << result.is_simple() << "\n";
    return result;
}

ci::PolyLine2f FlatShape::polyLineWithConnectedHoles() const
{
    std::vector<CGAL::Point_2<ExactK>> points;

    CGAL::connect_holes( polygonWithHoles<ExactK>(), std::back_inserter( points ) );

    return polyLineFrom<ExactK>( points );
}
