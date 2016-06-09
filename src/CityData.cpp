//
//  CityData.cpp
//  Cityscape
//
//  Created by Andrew Morton on 3/5/16.
//
//

#include "CityData.h"
#include "FlatShape.h"
#include "Scenery.h"
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


CityModel::CityModel()
{
    ZoningPlanRef majesticheights = ZoningPlan::create( "default" );
    majesticheights->addUsage( nullptr, 1 );
    majesticheights->addUsage( LotDeveloperRef( new SingleFamilyHomeDeveloper( {
            BuildingPlan::create( polyLineRectangle( 30, 10 ), 1, BuildingPlan::HIPPED_ROOF ),
            BuildingPlan::create( polyLineRectangle( 30, 10 ), 1, BuildingPlan::GABLED_ROOF ),
            BuildingPlan::create( polyLineLShape(), 1, BuildingPlan::HIPPED_ROOF ),
            BuildingPlan::create( polyLineLShape(), 1, BuildingPlan::GABLED_ROOF )
        } ) ), 30 );
    majesticheights->addUsage( LotDeveloperRef( new ParkDeveloper() ), 2 );

    ZoningPlanRef farm = ZoningPlan::create( "farm" );
    farm->district.streetDivision = ZoningPlan::StreetDivision::NO_STREET_DIVISION;
//    farm->block.lotDivision = ZoningPlan::LotDivision::NO_LOT_DIVISION;
    farm->district.streetDivision = ZoningPlan::StreetDivision::GRID_STREET_DIVIDED;
    farm->district.grid.avenueSpacing = 400;
    farm->district.grid.streetSpacing = 400;
    farm->block.lotWidth = 200;
    farm->addUsage( LotDeveloperRef( new FarmFieldDeveloper( 0, 5, 2 ) ), 2 );
    farm->addUsage( LotDeveloperRef( new FarmFieldDeveloper( M_PI_2, 10, 5 ) ), 2 );
    farm->addUsage( LotDeveloperRef( new FarmFieldDeveloper( M_PI_4, 7, 3 ) ), 1 );
    farm->addUsage( LotDeveloperRef( new FarmFieldDeveloper( M_PI_2 + M_PI_4, 6, 3 ) ), 1 );
    farm->addUsage( LotDeveloperRef( new FarmOrchardDeveloper( 0, 10, 7 ) ), 2 );
    farm->addUsage( LotDeveloperRef( new FarmOrchardDeveloper( M_PI_2, 10, 7 ) ), 2 );

    ZoningPlanRef industry = ZoningPlan::create( "industry" );
    industry->district.streetDivision = ZoningPlan::StreetDivision::NO_STREET_DIVISION;
    industry->block.lotDivision = ZoningPlan::LotDivision::NO_LOT_DIVISION;
    industry->addUsage( LotDeveloperRef( new SquareGridDeveloper( OilTank::create(), 50, 50, 0.0 ) ), 1 );

    zoningPlans = {
        farm,
        majesticheights,
        industry
    };
}

}