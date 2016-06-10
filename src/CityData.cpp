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
    ZoningPlanRef majesticheights = ZoningPlan::create( "suburbsx" );
    majesticheights->block.lotDivision = ZoningPlan::LotDivision::SKELETON_LOT_DIVISION;
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
    farm->district.streetDivision = ZoningPlan::StreetDivision::GRID_STREET_DIVIDED;
    farm->district.grid.avenueSpacing = 400;
    farm->district.grid.streetSpacing = 400;
    farm->block.lotDivision = ZoningPlan::LotDivision::OOB_LOT_DIVISION;
    farm->block.lotWidth = 200;
    farm->addUsage( LotDeveloperRef( new FarmFieldDeveloper( 0, 5, 2 ) ), 2 );
    farm->addUsage( LotDeveloperRef( new FarmFieldDeveloper( M_PI_2, 10, 5 ) ), 2 );
    farm->addUsage( LotDeveloperRef( new FarmFieldDeveloper( M_PI_4, 7, 3 ) ), 1 );
    farm->addUsage( LotDeveloperRef( new FarmFieldDeveloper( M_PI_2 + M_PI_4, 6, 3 ) ), 1 );
    farm->addUsage( LotDeveloperRef( new FarmOrchardDeveloper( 0, 10, 7 ) ), 2 );
    farm->addUsage( LotDeveloperRef( new FarmOrchardDeveloper( M_PI_2, 10, 7 ) ), 2 );

    ZoningPlanRef industry = ZoningPlan::create( "industry" );
    industry->district.streetDivision = ZoningPlan::StreetDivision::NO_STREET_DIVISION;
    industry->block.lotDivision = ZoningPlan::LotDivision::OOB_LOT_DIVISION;
    industry->block.lotAreaMax = 160000;
    auto oiltank = OilTank::create();
    industry->addUsage( LotDeveloperRef( new SquareGridDeveloper( [oiltank](const vec2 &at){ return oiltank->createInstace( at, 40, 15 ); }, 50, 50, 0.0 ) ), 1 );
    industry->addUsage( LotDeveloperRef( new SquareGridDeveloper( [oiltank](const vec2 &at){ return oiltank->createInstace( at, 60, 20 ); }, 70, 80, 0.0 ) ), 1 );
    industry->addUsage( LotDeveloperRef( new WarehouseDeveloper( {
        BuildingPlan::create( polyLineRectangle( 60, 40 ), 1, BuildingPlan::SAWTOOTH_ROOF ),
        BuildingPlan::create( polyLineRectangle( 40, 60 ), 1, BuildingPlan::FLAT_ROOF ),
    } ) ), 30 );

    ZoningPlanRef downtown = ZoningPlan::create( "downtown" );
    downtown->district.streetDivision = ZoningPlan::StreetDivision::GRID_STREET_DIVIDED;
    downtown->district.grid.avenueSpacing = 200;
    downtown->district.grid.streetSpacing = 300;
    downtown->block.lotDivision = ZoningPlan::LotDivision::SKELETON_LOT_DIVISION;
    downtown->block.lotWidth = 40;
    downtown->addUsage( LotDeveloperRef( new FullLotDeveloper( BuildingPlan::FLAT_ROOF ) ), 10 );
    downtown->addUsage( LotDeveloperRef( new ParkDeveloper() ), 1 );


    zoningPlans = {
        farm,
        majesticheights,
        industry
    };
}

}