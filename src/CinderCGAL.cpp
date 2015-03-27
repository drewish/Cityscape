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

Polygon_2 polygonFrom(const ci::PolyLine2f &p)
{
    auto begin = p.begin(),
        end = p.end(),
        last = --p.end(),
        i = begin;
    Polygon_2 poly;

    if (p.size() < 3) return poly;

    // if this is closed (first == last) we can skip the last one
    if (*begin == *last) {
        end = last;
    }
    do {
        poly.push_back(pointFrom(*i++));
    } while (i != end);

    // Ensure counter clockwise order
    if (poly.is_clockwise_oriented()) {
        poly.reverse_orientation();
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


void drawSkeleton(const SsPtr &ss)
{
    if (!ss) return;

    float col = 0;

    gl::lineWidth(2);
    // Draw the faces
    for( auto face = ss->faces_begin(); face != ss->faces_end(); ++face ) {
        PolyLine2f line;

        Ss::Halfedge_const_handle begin = face->halfedge();
        Ss::Halfedge_const_handle edge = begin;
        do {
            line.push_back(vecFrom(edge->vertex()->point()));
            edge = edge->next();
        } while (edge != begin);
        gl::color( ColorA( 1.0-col,0,col,0.5) );
        col += 0.1;
        gl::drawSolid( line );
        gl::color( ColorA( 1,0,0,0.5 ) );
        gl::draw( line );
        gl::drawSolidCircle(vecFrom(begin->vertex()->point()), 5);
    }

    // Then the outline
    PolyLine2f outline;
    Ss::Halfedge_const_handle begin = ss->faces_begin()->halfedge()->opposite();
    Ss::Halfedge_const_handle edge = begin;
    do {
        outline.push_back(vecFrom(edge->vertex()->point()));
        edge = edge->prev();
    } while (edge != begin);

    gl::lineWidth(1);
    gl::color( ColorA( 0,1,0,0.5 ) );
    gl::draw( outline );
    gl::drawSolidCircle(vecFrom(begin->vertex()->point()), 2);
}

K::Point_2 getCentroid( Polygon_2 p )
{
    K::Point_2 centroid = K::Point_2(0, 0);
    K::Line_2 line;
    CGAL::Dimension_tag<0> dt;
    CGAL::linear_least_squares_fitting_2( p.vertices_begin(), p.vertices_end(), line, centroid, dt);

    return centroid;
}
