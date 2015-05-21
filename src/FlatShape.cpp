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