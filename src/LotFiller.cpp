//
//  LotFiller.cpp
//  Cityscape
//
//  Created by Andrew Morton on 3/7/16.
//
//

#include "LotFiller.h"
#include "FlatShape.h"
#include "cinder/Rand.h"

using namespace ci;

namespace Cityscape {

void placeSingleBuilding( LotRef lot, const CityModel &city )
{
    int floors = 1 + randInt(2);

    lot->building = Building::create(
        BuildingPlan::create( BuildingPlan::randomOutline(), BuildingPlan::RANDOM_ROOF, floors),
        // TODO: just placing it in the center for now. would be good to take
        // the street into consideration.
        lot->shape->centroid(),
        M_PI * randInt(4) / 2
    );
    if ( !lot->building ) { return; }

    // Remove the building goes outside the lot
// TODO: try moving it around?
    std::vector<PolyLine2f> a = { lot->shape->outline() },
        b = { lot->building->plan->outline() },
        diff = PolyLine2f::calcDifference( b, a );
    if ( diff.size() != 0 ) {
        lot->building.reset();
    }
}

void fillLotWithBuilding( LotRef lot, const CityModel &city )
{
    // Vary the floors based on the area...
    // TODO: would be interesting to make taller buildings on smaller lots
    float area = lot->shape->area();
    int floors = 1 + (int) ( sqrt( area ) / 20 ) + ci::randInt( 6 );

    if ( area > 100 ) {
        BuildingPlan::RoofStyle roof = static_cast<BuildingPlan::RoofStyle>( city.options.building.roofStyle );
        lot->building = Building::create( BuildingPlan::create( lot->shape->outline(), roof, floors ) );
    }
    else {
        lot->building.reset();
    }
}

void buildPark( LotRef lot, float treeCoverRatio, const CityModel &city )
{
    float area = lot->shape->area();
    float totalTreeArea = 0.0;

    while ( totalTreeArea / area < treeCoverRatio ) {
        // Bigger areas should get bigger trees (speeds up the generation).
        // TODO: come up with a better formula for this
        float diameter = area < 10000 ? randFloat( 4, 12 ) : randFloat( 10, 20 );

        // TODO would be good to avoid random points by the edges so the trees
        // didn't go out of their lots.
        lot->trees.push_back( Tree::create( ci::vec3( lot->shape->randomPoint(), diameter + 3 ), diameter ) );

        // Treat it as a square for faster math and less dense coverage.
        totalTreeArea += diameter * diameter;
    }

}

void fillLots( CityModel &city )
{
    for ( const auto &district : city.districts ) {
        for ( const auto &block : district->blocks ) {
            for ( const auto &lot : block->lots ) {
                // Skip small lots
                if ( lot->shape->area() < 10 ) continue;

                // TODO these lot proportions should become an options
                if ( randInt( 8 ) == 0 ) {
                    // Randomize the tree coverage
                    buildPark( lot, randFloat( 0.25, 0.75 ), city );
                } else if ( randInt( 16 ) == 0 ) {
                    // Do nothing
                // TODO these options need to be moved from lot to block and renamed
                } else if ( city.options.lot.buildingPlacement == LotOptions::BUILDING_FILL_LOT ) {
                    fillLotWithBuilding( lot, city );
                } else {
                    placeSingleBuilding( lot, city );
                }
            }
        }
    }
}

} // Cityscape namespace
