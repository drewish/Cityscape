//
//  CityData.cpp
//  Cityscape
//
//  Created by Andrew Morton on 3/5/16.
//
//

#include "CityData.h"
#include "FlatShape.h"

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
    FlatShapeRef fs = FlatShape::create( PolyLine2f( {
        vec2( -600, -600 ), vec2(  600, -600 ),
        vec2(  600,  600 ), vec2( -600,  600 )
    } ) );
    Cityscape::DistrictRef district = Cityscape::District::create( fs, zoningPlans.front() );

    districts.push_back( district );
    district->blocks.push_back( block );
}

CityModel::CityModel( const BuildingRef &building )
{
    FlatShapeRef fs = FlatShape::create( PolyLine2f( {
        vec2( -600, -600 ), vec2(  600, -600 ),
        vec2(  600,  600 ), vec2( -600,  600 )
    } ) );
    Cityscape::DistrictRef district = Cityscape::District::create( fs, zoningPlans.front() );
    Cityscape::BlockRef    block    = Cityscape::Block::create( fs );
    Cityscape::LotRef      lot      = Cityscape::Lot::create( fs );

    districts.push_back( district );
    district->blocks.push_back( block );
    block->lots.push_back( lot );
    lot->building = building;
}


}