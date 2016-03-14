//
//  CityData.cpp
//  Cityscape
//
//  Created by Andrew Morton on 3/5/16.
//
//

#include "CityData.h"

using namespace ci;

namespace Cityscape {

// Give relatively unique colors
ColorA colorWheel()
{
    static float hue = 0.0f;
    ColorA color = ci::ColorA( ci::CM_HSV, hue, 1.0, 1.0, 0.5 );
    hue += 0.17f;
    if (hue > 1) hue -= 1.0f;
    return color;
}

CityModel::CityModel( const std::vector<ci::vec2> &highwayPoints )
{
    // Translate from RoadNetwork into Highways
    for ( size_t i = 1, size = highwayPoints.size(); i < size; i += 2 ) {
        highways.push_back( HighwayRef( new Highway( highwayPoints[i - 1], highwayPoints[i] ) ) );
    }
}

// For debugging build a city from a small portion
CityModel::CityModel( const BlockRef &block )
{
assert( false );
}

CityModel::CityModel( const BuildingRef &building )
{
assert( false );
}


}