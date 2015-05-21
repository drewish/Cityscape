//
//  CinderCGAL.h
//  Cityscape
//
//  Created by Andrew Morton on 2/24/15.
//
//

#ifndef __Cityscape__CinderCGAL__
#define __Cityscape__CinderCGAL__

#include <CGAL/Exact_predicates_exact_constructions_kernel_with_sqrt.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Polygon_set_2.h>

typedef CGAL::Exact_predicates_exact_constructions_kernel_with_sqrt K;
typedef CGAL::Polygon_2<K> Polygon_2;
typedef CGAL::Polygon_with_holes_2<K> Polygon_with_holes_2;
typedef CGAL::Polygon_set_2<K> Polygon_set_2;

inline ci::Vec2f vecFrom(const K::Point_2 &p)
{
    return ci::Vec2f( p.x().floatValue(), p.y().floatValue() );
}

inline K::Point_2 pointFrom(const ci::Vec2f &p)
{
    return K::Point_2(p.x, p.y);
}

Polygon_2 polygonFrom(const ci::PolyLine2f &p, bool forceClockwise = true);
Polygon_with_holes_2 polygonFrom(const ci::PolyLine2f &outline, const std::vector<ci::PolyLine2f> &holes);

ci::PolyLine2f polyLineFrom(const Polygon_2 &p);


K::Point_2 getCentroid( Polygon_2 p );





#endif /* defined(__Cityscape__CinderCGAL__) */
