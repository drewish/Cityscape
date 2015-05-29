//
//  CinderCGAL.h
//  Cityscape
//
//  Created by Andrew Morton on 2/24/15.
//
//

#ifndef __Cityscape__CinderCGAL__
#define __Cityscape__CinderCGAL__

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel_with_sqrt.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel InexactK;
typedef CGAL::Exact_predicates_exact_constructions_kernel_with_sqrt ExactK;

inline ci::Vec2f vecFrom(const InexactK::Point_2 &p)
{
    return ci::Vec2f( p.x(), p.y() );
}

inline ci::Vec2f vecFrom(const ExactK::Point_2 &p)
{
    return ci::Vec2f( p.x().floatValue(), p.y().floatValue() );
}

template<class K>
inline CGAL::Point_2<K> pointFrom(const ci::Vec2f &p)
{
    return CGAL::Point_2<K>(p.x, p.y);
}

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



#endif /* defined(__Cityscape__CinderCGAL__) */
