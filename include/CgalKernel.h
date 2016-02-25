//
//  CgalLKernel.h
//  Cityscape
//
//  Created by Andrew Morton on 7/7/15.
//
//

#pragma once

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel_with_sqrt.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel InexactK;
typedef CGAL::Exact_predicates_exact_constructions_kernel_with_sqrt ExactK;

inline ci::vec2 vecFrom(const InexactK::Point_2 &p)
{
    return ci::vec2( p.x(), p.y() );
}

inline ci::vec2 vecFrom(const ExactK::Point_2 &p)
{
    return ci::vec2( p.x().floatValue(), p.y().floatValue() );
}

template<class K>
inline CGAL::Point_2<K> pointFrom(const ci::vec2 &p)
{
    return CGAL::Point_2<K>(p.x, p.y);
}

