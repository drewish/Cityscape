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

void Lot::plantTree( float diameter, const ci::vec2 &at, TreeFamily f )
{
    vec3 position = vec3( at, 3 );
    ColorA color;
    if ( f == CIRCULAR_TREE ) {
        color = ColorA( 0.41f, 0.60f, 0.22f, 0.75f );
        position += ci::vec3( 0, 0, diameter );
    } else {
        color = ColorA( 0.41f, 0.60f, 0.22f, 1.0f );
    }
    trees.push_back( Tree::create( position, diameter, color, f ) );
}

void Lot::build( const BlueprintRef &blueprint, const ci::vec2 &position, float rotation )
{
    buildings.push_back( Building::create( blueprint, position, rotation ) );
}

// * * *


CityModel::CityModel()
{
    ZoningPlanRef majesticheights = ZoningPlan::create( "default" );
    majesticheights->addUsage( nullptr, 1 );
    majesticheights->addUsage( LotDeveloperRef( new SingleFamilyHomeDeveloper( {
            BuildingPlan::create( BuildingPlan::rectangle( 30, 10 ), 1, BuildingPlan::HIPPED_ROOF ),
            BuildingPlan::create( BuildingPlan::rectangle( 30, 10 ), 1, BuildingPlan::GABLED_ROOF ),
            BuildingPlan::create( BuildingPlan::lshape(), 1, BuildingPlan::HIPPED_ROOF ),
            BuildingPlan::create( BuildingPlan::lshape(), 1, BuildingPlan::GABLED_ROOF )
        } ) ), 30 );
    majesticheights->addUsage( LotDeveloperRef( new ParkDeveloper() ), 2 );

    ZoningPlanRef farm = ZoningPlan::create( "farm" );
    farm->district.streetDivision = ZoningPlan::StreetDivision::NO_STREET_DIVISION;
    farm->block.lotDivision = ZoningPlan::LotDivision::NO_LOT_DIVISION;
    farm->addUsage( LotDeveloperRef( new FarmOrchardDeveloper() ), 2 );

    ZoningPlanRef industry = ZoningPlan::create( "industry" );
//    industry->district.streetDivision = ZoningPlan::StreetDivision::NO_STREET_DIVISION;
    industry->block.lotDivision = ZoningPlan::LotDivision::NO_LOT_DIVISION;
    industry->addUsage( LotDeveloperRef( new SquareGridDeveloper( OilTank::create(), 50, 50, 0.0 ) ), 1 );

    zoningPlans = { majesticheights, farm, industry };
}

}