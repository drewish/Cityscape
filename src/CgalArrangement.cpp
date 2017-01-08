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

void setFaceRoles( Arrangement_2 &arr, FaceRole data )
{
    for( auto face = arr.faces_begin(); face != arr.faces_end(); ++face ) {
        if( face->is_unbounded() ) continue;
        face->set_data( data );
    }
}
void setEdgeRoles( Arrangement_2 &arr, EdgeRole data )
{
    for ( auto edge = arr.edges_begin(); edge != arr.edges_end(); ++edge ) { edge->set_data( data ); }
}
void setVertexData( Arrangement_2 &arr, float data )
{
    for( auto vert = arr.vertices_begin(); vert != arr.vertices_end(); ++vert ) { vert->set_data( data ); }
}

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

std::vector<ci::vec3> vec3sFrom( const Arrangement_2::Ccb_halfedge_const_circulator &circulator )
{
    std::vector<ci::vec3> result;
    auto cc = circulator;
    result.push_back( vec3From( cc->source() ) );
    do {
        result.push_back( vec3From( cc->target() ) );
    } while ( ++cc != circulator );
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
