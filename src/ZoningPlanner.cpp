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
    PolyLine2f rect = polyLineRectangle( 21, 8 );
    PolyLine2f el = polyLineLShape().scaled( vec2( 0.7 ) );
    plan->addUsage(
        LotDeveloperRef(
            new SingleFamilyHomeDeveloper( {
                BuildingPlan::create( rect, 1, RoofStyle::HIPPED, 0.5, 1 ),
                BuildingPlan::create( rect, 1, RoofStyle::GABLED, 0.5, 1 ),
                BuildingPlan::create( el, 1, RoofStyle::HIPPED, 0.5, 1 ),
                BuildingPlan::create( el, 1, RoofStyle::GABLED, 0.5, 1 )
            } )
        ),
        30
    );
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

    BuildingSettings barnSettings;
    barnSettings.floors = 1;
    barnSettings.roofStyle = RoofStyle::GABLED;
    barnSettings.overhang = 0.5;
    barnSettings.slope = 0.6666;
    SceneryRef barn = BuildingPlan::create( polyLineRectangle( 12, 10 ), barnSettings );
    SceneryRef silo = GrainSiloConeTop::create( 2.5, 10 );
    SceneryRef barnAndSilo = SceneryGroup::create(
        {
            SceneryGroup::Item( barn, vec3( 0 ), M_PI_2 ),
            SceneryGroup::Item( silo, vec3( 8, 0, 0 ) ),
        },
        polyLineRectangle( 17, 15 )
    );

    farm->addUsage( LotDeveloperRef( new FarmFieldDeveloper(  5, 2, barnAndSilo ) ), 2 );
    farm->addUsage( LotDeveloperRef( new FarmFieldDeveloper( 10, 5 ) ), 2 );
    farm->addUsage( LotDeveloperRef( new FarmFieldDeveloper( 7, 3, barnAndSilo ) ), 1 );
    farm->addUsage( LotDeveloperRef( new FarmFieldDeveloper( 6, 3 ) ), 1 );
    farm->addUsage( LotDeveloperRef( new FarmOrchardDeveloper( 0, 10, 7 ) ), 2 );
    farm->addUsage( LotDeveloperRef( new FarmOrchardDeveloper( M_PI_2, 10, 7 ) ), 2 );

    return farm;
}

ZoningPlanRef zoneIndustrial() {
    ZoningPlanRef industry = ZoningPlan::create( "industry" );
    industry->district.streetDivision = ZoningPlan::StreetDivision::NO_STREET_DIVISION;
    industry->block.lotDivision = ZoningPlan::LotDivision::OOB_LOT_DIVISION;
    industry->block.lotAreaMax = 160000;

    industry->district.streetDivision = ZoningPlan::StreetDivision::GRID_STREET_DIVIDED;
    industry->district.grid.avenueSpacing = 400;
    industry->district.grid.streetSpacing = 400;
    industry->block.lotAreaMax = 16000;

    industry->addUsage( LotDeveloperRef( new SquareGridDeveloper( OilTank::create( 40, 15 ), 50, 50, 0.0 ) ), 1 );
    industry->addUsage( LotDeveloperRef( new SquareGridDeveloper( OilTank::create( 60, 20 ), 70, 80, 0.0 ) ), 1 );

    SceneryRef smoke = SmokeStack::create();
    SceneryRef flat = BuildingPlan::create( polyLineRectangle( 60, 40 ), 1, RoofStyle::FLAT );
    SceneryRef saw = BuildingPlan::create( polyLineRectangle( 60, 40 ), 1, RoofStyle::SAWTOOTH );
    industry->addUsage( LotDeveloperRef( new PickFromListDeveloper( {
        flat,
        saw,
        SceneryGroup::create( {
            SceneryGroup::Item( saw ),
            SceneryGroup::Item( smoke, vec3( 10, -2, 5 ) ),
            SceneryGroup::Item( smoke, vec3( 10, +2, 5 ) ),
        } ),
        SceneryGroup::create( {
            SceneryGroup::Item( flat ),
            SceneryGroup::Item( smoke, vec3( 10, -3, 5 ) ),
            SceneryGroup::Item( smoke, vec3( 10, 0, 5 ) ),
            SceneryGroup::Item( smoke, vec3( 10, 3, 5 ) ),
        } ),
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
    downtown->addUsage( LotDeveloperRef( new FullLotDeveloper( RoofStyle::FLAT ) ), 10 );
    downtown->addUsage( LotDeveloperRef( new ParkDeveloper() ), 1 );

    return downtown;
}


} // namespace Cityscape
