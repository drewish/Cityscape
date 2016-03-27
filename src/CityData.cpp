//
//  CityData.cpp
//  Cityscape
//
//  Created by Andrew Morton on 3/5/16.
//
//

#include "CityData.h"
#include "FlatShape.h"
#include "BuildingPlan.h"
#include "LotDeveloper.h"

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

// * * *

void Lot::plantTree( float diameter )
{
    plantTree( diameter, shape->randomPoint() );
}

void Lot::plantTree( float diameter, ci::vec2 at )
{
    trees.push_back( Tree::create( ci::vec3( at, diameter + 3 ), diameter ) );
}

// * * *


CityModel::CityModel()
{
    ZoningPlanRef majesticheights = ZoningPlan::create( "default" );
    majesticheights->lotUsages.push_back( ZoningPlan::LotUsage( nullptr, 1 ) );
    majesticheights->lotUsages.push_back( ZoningPlan::LotUsage( LotDeveloperRef( new SingleFamilyHomeDeveloper( {
            BuildingPlan::create( BuildingPlan::rectangle( 30, 10 ), 1, BuildingPlan::HIPPED_ROOF ),
            BuildingPlan::create( BuildingPlan::rectangle( 30, 10 ), 1, BuildingPlan::GABLED_ROOF ),
            BuildingPlan::create( BuildingPlan::lshape(), 1, BuildingPlan::HIPPED_ROOF ),
            BuildingPlan::create( BuildingPlan::lshape(), 1, BuildingPlan::GABLED_ROOF )

        } ) ), 30 ) );
    majesticheights->lotUsages.push_back( ZoningPlan::LotUsage( LotDeveloperRef( new ParkDeveloper() ), 2 ) );

    ZoningPlanRef farm = ZoningPlan::create( "farm" );
    farm->district.streetDivision = ZoningPlan::StreetDivision::NO_STREET_DIVISION;
    farm->block.lotDivision = ZoningPlan::LotDivision::NO_LOT_DIVISION;
    farm->lotUsages.push_back( ZoningPlan::LotUsage( LotDeveloperRef( new FarmOrchardDeveloper() ), 2 ) );


    zoningPlans = { majesticheights, farm };
}

}