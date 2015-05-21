//
//  FlatShape.cpp
//  Cityscape
//
//  Created by andrew morton on 5/20/15.
//
//

#include "FlatShape.h"

#include "CinderCGAL.h"
#include <CGAL/linear_least_squares_fitting_2.h>

const ci::Vec2f FlatShape::centroid()
{
    Polygon_2 p( polygonFrom( mOutline ) );
    K::Point_2 centroid = K::Point_2(0, 0);
    K::Line_2 line;
    CGAL::Dimension_tag<0> dt;
    CGAL::linear_least_squares_fitting_2( p.vertices_begin(), p.vertices_end(), line, centroid, dt);

    return vecFrom( centroid );
}

const Polygon_with_holes_2 FlatShape::polygon_with_holes() {
    Polygon_with_holes_2 poly( polygonFrom( mOutline ) );
    for ( auto it = mHoles.begin(); it != mHoles.end(); ++it ) {
        poly.add_hole( polygonFrom( *it ) );
    }
    return poly;
}

const ci::TriMesh2d FlatShape::makeMesh() {
    // TODO might be good to lazily create this when they first ask for the mesh.
    ci::Triangulator triangulator;
    triangulator.addPolyLine( mOutline );
    for( auto it = mHoles.begin(); it != mHoles.end(); ++it ) {
        triangulator.addPolyLine( *it );
    }

    return triangulator.calcMesh();
}
