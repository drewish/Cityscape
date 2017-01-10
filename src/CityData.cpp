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
    ZoningPlanRef nimby = ZoningPlan::create( "Not in my backyard" );
    nimby->block.lotDivision = ZoningPlan::LotDivision::NO_LOT_DIVISION;
    nimby->addUsage( nullptr, 1 );

    zoningPlans = { nimby };
}

}
