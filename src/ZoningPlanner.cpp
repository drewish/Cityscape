//
//  ZoningPlanner.cpp
//  Cityscape
//
//  Created by Andrew Morton on 1/9/17.
//
//

#include "ZoningPlanner.h"
#include "CityData.h"
#include "LotDeveloper.h"
#include "Scenery.h"

namespace Cityscape {

using namespace ci;

ZoningPlanRef zoneMajesticHeights() {
    ZoningPlanRef plan = ZoningPlan::create( "suburbsx" );
    plan->block.lotDivision = ZoningPlan::LotDivision::SKELETON_LOT_DIVISION;
    plan->addUsage( nullptr, 1 );
    plan->addUsage( LotDeveloperRef( new SingleFamilyHomeDeveloper( {
            BuildingPlan::create( polyLineRectangle( 30, 10 ), 1, RoofStyle::HIPPED ),
            BuildingPlan::create( polyLineRectangle( 30, 10 ), 1, RoofStyle::GABLED ),
            BuildingPlan::create( polyLineLShape(), 1, RoofStyle::HIPPED ),
            BuildingPlan::create( polyLineLShape(), 1, RoofStyle::GABLED )
        } ) ), 30 );
    plan->addUsage( LotDeveloperRef( new ParkDeveloper() ), 2 );

    return plan;
}

ZoningPlanRef zoneFarming() {
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

    return farm;
}

ZoningPlanRef zoneIndustrial() {
    ZoningPlanRef industry = ZoningPlan::create( "industry" );
    industry->district.streetDivision = ZoningPlan::StreetDivision::NO_STREET_DIVISION;
    industry->block.lotDivision = ZoningPlan::LotDivision::OOB_LOT_DIVISION;
    industry->block.lotAreaMax = 160000;
    auto oiltank = OilTank::create();
    industry->addUsage( LotDeveloperRef( new SquareGridDeveloper( [oiltank](const vec2 &at){ return oiltank->createInstace( at, 40, 15 ); }, 50, 50, 0.0 ) ), 1 );
    industry->addUsage( LotDeveloperRef( new SquareGridDeveloper( [oiltank](const vec2 &at){ return oiltank->createInstace( at, 60, 20 ); }, 70, 80, 0.0 ) ), 1 );
    industry->addUsage( LotDeveloperRef( new WarehouseDeveloper( {
        BuildingPlan::create( polyLineRectangle( 60, 40 ), 1, RoofStyle::SAWTOOTH ),
        BuildingPlan::create( polyLineRectangle( 40, 60 ), 1, RoofStyle::FLAT ),
    } ) ), 30 );

    return industry;
}

ZoningPlanRef zoneDowntown() {
    ZoningPlanRef downtown = ZoningPlan::create( "downtown" );
    downtown->district.streetDivision = ZoningPlan::StreetDivision::GRID_STREET_DIVIDED;
    downtown->district.grid.avenueSpacing = 200;
    downtown->district.grid.streetSpacing = 300;
    downtown->block.lotDivision = ZoningPlan::LotDivision::SKELETON_LOT_DIVISION;
    downtown->block.lotWidth = 40;
    downtown->addUsage( LotDeveloperRef( new FullLotDeveloper( RoofStyle::SAWTOOTH ) ), 10 );
    downtown->addUsage( LotDeveloperRef( new ParkDeveloper() ), 1 );

    return downtown;
}


} // namespace Cityscape
