//
//  CinderCGAL.cpp
//  Cityscape
//
//  Created by Andrew Morton on 2/24/15.
//
//

#include "CinderCGAL.h"

#include <CGAL/exceptions.h>


using namespace ci;

Polygon_2 polyFrom(ci::PolyLine2f p)
{
    auto begin = p.begin(),
        end = p.end(),
        last = --p.end(),
        i = begin;
    Polygon_2 poly;

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
