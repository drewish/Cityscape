//
//  CinderCGAL.cpp
//  Cityscape
//
//  Created by Andrew Morton on 2/24/15.
//
//

#include "CinderCGAL.h"

#include <CGAL/exceptions.h>
#include <CGAL/linear_least_squares_fitting_2.h>

using namespace ci;

Polygon_2 polygonFrom(const ci::PolyLine2f &p, bool forceClockwise)
{
    Polygon_2 poly;
    auto begin = p.begin(),
        end = p.end(),
        last = --p.end(),
        i = begin;

    if (p.size() < 3) return poly;

    // if this is closed (first == last) we can skip the last one
    if (*begin == *last) {
        end = last;
    }
    do {
        poly.push_back(pointFrom(*i++));
    } while (i != end);

    // Ensure counter clockwise order
    if (forceClockwise && poly.is_clockwise_oriented()) {
        poly.reverse_orientation();
    }
    return poly;
}

Polygon_with_holes_2 polygonFrom(const PolyLine2f &outline, const std::vector<ci::PolyLine2f> &holes)
{
    Polygon_with_holes_2 poly( polygonFrom( outline ) );

    for( auto it = holes.begin(); it != holes.end(); ++it ) {
        poly.add_hole( polygonFrom( *it ) );
    }

    return poly;
}


ci::PolyLine2f polyLineFrom(const Polygon_2 &p)
{
    ci::PolyLine2f poly;
    for ( auto it = p.vertices_begin(); it != p.vertices_end(); ++it) {
        poly.push_back( vecFrom( *it ) );
    }
    // Close it... I'm not sure I love doing this...
    if (poly.size() > 2) {
        poly.push_back( *poly.begin() );
        poly.setClosed();
    }

    return poly;
}

K::Point_2 getCentroid( Polygon_2 p )
{
    K::Point_2 centroid = K::Point_2(0, 0);
    K::Line_2 line;
    CGAL::Dimension_tag<0> dt;
    CGAL::linear_least_squares_fitting_2( p.vertices_begin(), p.vertices_end(), line, centroid, dt);

    return centroid;
}
