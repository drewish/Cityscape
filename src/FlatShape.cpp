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

const ci::vec2 FlatShape::centroid()
{
    CGAL::Polygon_2<InexactK> p( polygonFrom<InexactK>( mOutline ) );
    InexactK::Point_2 centroid = InexactK::Point_2(0, 0);
    InexactK::Line_2 line;
    CGAL::Dimension_tag<0> dt;
    CGAL::linear_least_squares_fitting_2( p.vertices_begin(), p.vertices_end(), line, centroid, dt);

    return vecFrom( centroid );
}

const ci::TriMesh FlatShape::makeMesh() {
    // TODO might be good to lazily create this when they first ask for the mesh.
    ci::Triangulator triangulator( mOutline );
    for( auto it = mHoles.begin(); it != mHoles.end(); ++it ) {
        triangulator.addPolyLine( *it );
    }

    return triangulator.calcMesh();
}
