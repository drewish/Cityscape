//
//  CgalArrangement.cpp
//  Cityscape
//
//  Created by Andrew Morton on 7/18/15.
//
//

#include "CgalArrangement.h"
#include "CgalPolygon.h"

#include <CGAL/Sweep_line_2_algorithms.h>

ci::PolyLine2f polyLineFrom( const Arrangement_2::Ccb_halfedge_const_circulator &circulator )
{
    ci::PolyLine2f result;
    auto cc = circulator;
    result.push_back( vecFrom( cc->source()->point() ) );
    do {
        result.push_back( vecFrom( cc->target()->point() ) );
    } while ( ++cc != circulator );
    return result;
}

std::list<Segment_2> contiguousSegmentsFrom( const std::vector<ci::vec2> &points )
{
    std::list<Segment_2> result;
    contiguousSegmentsFrom( points, std::back_inserter( result ) );
    return result;
}

std::list<Segment_2> contiguousSegmentsFrom( const std::vector<Point_2> &points )
{
    std::list<Segment_2> result;
    contiguousSegmentsFrom( points, std::back_inserter( result ) );
    return result;
}


std::list<Segment_2> segmentsFrom( const std::vector<ci::vec2> &points )
{
    assert( points.size() % 2 == 0 );

    std::list<Segment_2> result;
    if ( points.empty() ) return result;

    for ( auto i = points.begin(); i != points.end(); ++i ) {
        result.push_back( Segment_2( pointFrom( *i ), pointFrom( *(++i) ) ) );
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
