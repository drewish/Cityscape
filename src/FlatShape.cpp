//
//  FlatShape.cpp
//  Cityscape
//
//  Created by andrew morton on 5/20/15.
//
//

#include "FlatShape.h"
#include "cinder/Rand.h"
#include <CGAL/linear_least_squares_fitting_2.h>

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

const TriMesh FlatShape::makeMesh()
{
    // TODO might be good to lazily create this when they first ask for the mesh.
    Triangulator triangulator( mOutline );
    for( auto it = mHoles.begin(); it != mHoles.end(); ++it ) {
        triangulator.addPolyLine( *it );
    }

    return triangulator.calcMesh();
}

