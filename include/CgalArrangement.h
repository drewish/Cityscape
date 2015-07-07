//
//  CgalArrangment.h
//  Cityscape
//
//  Created by Andrew Morton on 7/7/15.
//
//

#ifndef Cityscape_CgalArrangment_h
#define Cityscape_CgalArrangment_h

#include "CgalKernel.h"

#include <CGAL/Arrangement_2.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arr_naive_point_location.h>
#include <CGAL/Sweep_line_2_algorithms.h>

typedef CGAL::Arr_segment_traits_2<ExactK>            Traits_2;
typedef CGAL::Arrangement_2<Traits_2>                 Arrangement_2;
typedef CGAL::Arr_naive_point_location<Arrangement_2> Naive_pl;
typedef Traits_2::Point_2                             Point_2;
typedef Traits_2::X_monotone_curve_2                  Segment_2;

inline ci::PolyLine2f polyLineFrom( const Arrangement_2::Ccb_halfedge_circulator &circulator )
{
    ci::PolyLine2f result;
    Arrangement_2::Ccb_halfedge_circulator cc = circulator;
    do {
        result.push_back( vecFrom( cc->target()->point() ) );
    } while ( ++cc != circulator );
    return result;
}

#endif
