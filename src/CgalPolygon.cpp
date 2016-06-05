//
//  CgalPolygon.cpp
//  Cityscape
//
//  Created by andrew morton on 5/14/16.
//
//

#include "CgalPolygon.h"
#include <CGAL/Polyline_simplification_2/simplify.h>

using namespace ci;
using namespace CGAL;

PolyLine2f simplify( const PolyLine2f &polyline ) {
    Polyline_simplification_2::Squared_distance_cost cost;
    Polyline_simplification_2::Stop_above_cost_threshold stop( 1000 );
//    PS::Stop_below_count_ratio_threshold stop( 0.9 );
    return polyLineFrom<InexactK>( Polyline_simplification_2::simplify( polygonFrom<InexactK>( polyline ), cost, stop ) );
}
