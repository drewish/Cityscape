//
//  CgalPolygon.hpp
//  Cityscape
//
//  Created by Andrew Morton on 2/24/16.
//
//

#pragma once

#include "CgalKernel.h"

#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>

template<class K>
CGAL::Polygon_2<K> polygonFrom( const ci::PolyLine2f &p )
{
    CGAL::Polygon_2<K> poly;
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
        poly.push_back(pointFrom<K>(*i++));
    } while (i != end);

    return poly;
}

template<class K>
CGAL::Polygon_with_holes_2<K> polygonFrom(const ci::PolyLine2f &outline, const std::vector<ci::PolyLine2f> &holes)
{
    CGAL::Polygon_with_holes_2<K> poly( polygonFrom<K>( outline ) );

    for( auto it = holes.begin(); it != holes.end(); ++it ) {
        poly.add_hole( polygonFrom<K>( *it ) );
    }

    return poly;
}

template<class K>
ci::PolyLine2f polyLineFrom(const CGAL::Polygon_2<K> &p)
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

template<class K>
ci::PolyLine2f polyLineFrom( const std::vector< CGAL::Point_2<K> > &points )
{
    ci::PolyLine2f poly;
    for ( auto &p : points ) {
        poly.push_back( vecFrom( p ) );
    }
    // Close it... I'm not sure I love doing this...
    if (poly.size() > 2) {
        poly.push_back( *poly.begin() );
        poly.setClosed();
    }

    return poly;
}

template<class K>
void printPolygon( const CGAL::Polygon_with_holes_2<K> &s )
{
    std::cout << "\nouter is ";
    if ( s.is_unbounded() ) {
        std::cout << "unbounded\n";
    } else {
        if ( s.outer_boundary().is_clockwise_oriented() ) { std::cout << "clockwise:\n"; }
        else { std::cout << "counter-clockwise:\n"; }
        for ( auto p = s.outer_boundary().vertices_begin(); p != s.outer_boundary().vertices_end(); ++p ) {
            std::cout << p->x() << ", " << p->y() << "\n";
        }
    }
    if ( s.number_of_holes() > 0 ) {
        std::cout << "num holes: " << s.number_of_holes() << "\n";
        for ( auto h = s.holes_begin(); h != s.holes_end(); ++h ) {
            std::cout << "hole with " << h->size() << " points going ";
            if ( h->is_clockwise_oriented() ) { std::cout << "clockwise:\n"; }
            else { std::cout << "counter-clockwise:\n"; }
            for ( auto p = h->vertices_begin(); p != h->vertices_end(); ++p ) {
                std::cout << p->x() << ", " << p->y() << "\n";
            }
        }
    }
}
