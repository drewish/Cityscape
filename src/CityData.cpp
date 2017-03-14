//
//  CityData.cpp
//  Cityscape
//
//  Created by Andrew Morton on 3/5/16.
//
//

#include "cinder/ConvexHull.h"

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

} // namespace Cityscape

SceneryGroup::SceneryGroup( const std::vector<Item> &items, const ci::PolyLine2f &footprint )
    : Scenery( footprint, nullptr ), items( items )
{}

SceneryGroup::SceneryGroup( const std::vector<Item> &items )
    : Scenery( ci::PolyLine2f(), nullptr ), items( items )
{
    std::vector<ci::vec2> points;
    for ( auto item : items ) {
        ci::mat4 transfomation = item.transformation();
        for ( const auto &p : Scenery::Instance::transform( item.scenery->footprint(), transfomation ) ) {
            points.push_back( p );
        }
    }
    mFootprint = calcConvexHull( points );
}

Scenery::Instance SceneryGroup::instance( const ci::mat4 &matrix, const ci::ColorA &color ) const
{
    Scenery::Instance instance = Scenery::Instance( shared_from_this(), matrix, color );
    for ( const Item &item : items ) {
        instance.children.push_back( item.scenery->instance( item.transformation( matrix ) ) );
    }
    return instance;
}
