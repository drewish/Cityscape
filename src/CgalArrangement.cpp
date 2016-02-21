//
//  CgalArrangement.cpp
//  Cityscape
//
//  Created by Andrew Morton on 7/18/15.
//
//

#include "CgalArrangement.h"
#include "CgalPolygon.h"

ci::PolyLine2f polyLineFrom( const Arrangement_2::Ccb_halfedge_circulator &circulator )
{
    ci::PolyLine2f result;
    Arrangement_2::Ccb_halfedge_circulator cc = circulator;
    do {
        result.push_back( vecFrom( cc->target()->point() ) );
    } while ( ++cc != circulator );
    return result;
}

void findIntersections(const std::list<Segment_2> &input, std::list<Segment_2> &newEdges )
{
    std::vector<Point_2> pts;
    CGAL::compute_intersection_points( input.begin(), input.end(), std::back_inserter( pts ) );

    // Even numbers of intersections become segments
    for ( int i = pts.size() - 1; i > 0; i -= 2 ) {
        newEdges.push_back( Segment_2( pts[i - 1], pts[i] ) );
    }
}

std::list<Segment_2> contiguousSegmentsFrom( const std::vector<ci::vec2> &points )
{
    std::list<Segment_2> result;
    if ( points.empty() ) return result;

    for ( auto prev = points.begin(), curr = prev + 1; curr != points.end(); ++curr ) {
        result.push_back( Segment_2( Point_2( prev->x, prev->y ), Point_2( curr->x, curr->y ) ) );
        prev = curr;
    }
    return result;
}

std::list<Segment_2> contiguousSegmentsFrom( const std::vector<Point_2> &points )
{
    std::list<Segment_2> result;
    if ( points.empty() ) return result;

    for ( auto prev = points.begin(), curr = prev + 1; curr != points.end(); ++curr ) {
        result.push_back( Segment_2( *prev, *curr ) );
        prev = curr;
    }
    return result;
}


std::list<Segment_2> segmentsFrom( const std::vector<ci::vec2> &points )
{
    assert( points.size() % 2 == 0 );

    std::list<Segment_2> result;
    if ( points.empty() ) return result;

    for ( auto i = points.begin(); i != points.end(); ++i ) {
        result.push_back( Segment_2( Point_2( i->x, i->y ), Point_2( (++i)->x, i->y ) ) );
    }
    return result;
}

std::list<Segment_2> segmentsFrom( const std::vector<Point_2> &points )
{
    assert( points.size() % 2 == 0 );

    std::list<Segment_2> result;
    if ( points.empty() ) return result;

    for ( auto i = points.begin(); i != points.end(); ++i ) {
        result.push_back( Segment_2( *i, *++i ) );
    }
    return result;
}