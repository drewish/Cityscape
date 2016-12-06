//
//  CgalArrangment.h
//  Cityscape
//
//  Created by Andrew Morton on 7/7/15.
//
//

#pragma once

#include "CgalKernel.h"

#include <CGAL/Arrangement_2.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arr_extended_dcel.h>
#include <CGAL/Arr_naive_point_location.h>

// Need to benchmark this kernel to see if it's faster.
#include <CGAL/Cartesian.h>
typedef CGAL::Quotient<CGAL::MP_Float> Number_type;
typedef CGAL::Cartesian<Number_type> Kernel;

enum class FaceRole { Lot, Hole };
enum class EdgeRole { Street, Interior };
typedef CGAL::Arr_segment_traits_2<ExactK>              Traits_2;
// vert, halfedge, face but I'm only using edge and face values
typedef CGAL::Arr_extended_dcel<Traits_2, bool, EdgeRole, FaceRole> Dcel;
typedef CGAL::Arrangement_2<Traits_2, Dcel>             Arrangement_2;
typedef Traits_2::Point_2                               Point_2;
typedef Traits_2::X_monotone_curve_2                    Segment_2;
typedef CGAL::Arr_naive_point_location<Arrangement_2>   Naive_pl;

inline ci::vec2 vecFrom(const Kernel::Point_2 &p)
{
    return ci::vec2( CGAL::to_double( p.x() ), CGAL::to_double( p.y() ) );
}

ci::PolyLine2f polyLineFrom( const Arrangement_2::Ccb_halfedge_const_circulator &circulator );

inline Point_2 pointFrom( const ci::vec2 &p )
{
    return Point_2( p.x, p.y );
}

typedef std::pair<ci::vec2, ci::vec2> seg2;
inline Segment_2 segmentFrom( const seg2 &s )
{
    return Segment_2( pointFrom( s.first ), pointFrom( s.second ) );
}

void findIntersections( const std::list<Segment_2> &input, std::list<Segment_2> &newEdges );

// Segments will be created from a->b, b->c, c->d
std::list<Segment_2> contiguousSegmentsFrom( const std::vector<ci::vec2> &points );
std::list<Segment_2> contiguousSegmentsFrom( const std::vector<Point_2> &points );
template<class OI>
void contiguousSegmentsFrom( const std::vector<ci::vec2> &points, OI out )
{
    if ( points.empty() ) return;

    std::transform( begin( points ), end( points ) - 1, begin( points ) + 1, out,
        []( const ci::vec2 &a, const ci::vec2 &b ) {
            return Segment_2( pointFrom( a ), pointFrom( b ) );
        } );
}
template<class OI>
void contiguousSegmentsFrom( const std::vector<Point_2> &points, OI out )
{
    if ( points.empty() ) return;

    std::transform( begin( points ), end( points ) - 1, begin( points ) + 1, out,
        []( const Point_2 &a, const Point_2 &b ) {
            return Segment_2( a, b );
        } );
}

// Segments will be created from a->b, c->d
std::list<Segment_2> segmentsFrom( const std::vector<ci::vec2> &points );
std::list<Segment_2> segmentsFrom( const std::vector<Point_2> &points );
