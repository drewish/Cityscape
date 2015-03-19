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
#include <CGAL/Polygon_2.h>
#include <CGAL/create_straight_skeleton_2.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Polygon_2<K> Polygon_2;
typedef CGAL::Straight_skeleton_2<K> Ss;
typedef boost::shared_ptr<Ss> SsPtr;

inline ci::Vec2f vecFrom(K::Point_2 p)
{
    return ci::Vec2f(p.x(), p.y());
}

inline K::Point_2 pointFrom(ci::Vec2f p)
{
    return K::Point_2(p.x, p.y);
}

Polygon_2 polyFrom(ci::PolyLine2f p);

void drawSkeleton(const SsPtr &ss);





#endif /* defined(__Cityscape__CinderCGAL__) */
