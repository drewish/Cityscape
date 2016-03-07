//
//  Lot.cpp
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#include "Lot.h"
#include "cinder/Rand.h"

using namespace ci;


void SingleBuildingLot::layout( const Options &options )
{
    int floors = 1 + randInt(2);

    mBuildingRef = Building::create(
        BuildingPlan( BuildingPlan::randomOutline(), BuildingPlan::RANDOM_ROOF ),
        floors,
        // TODO: just placing it in the center for now. would be good to take
        // the street into consideration.
        mShape.centroid(),
        M_PI * randInt(4) / 2
    );
    if (!mBuildingRef) { return; }

    std::vector<PolyLine2f> a = { mShape.outline() },
        b = { mBuildingRef->outline() },
        diff = PolyLine2f::calcDifference( b, a );
    if ( diff.size() != 0 ) {
        mBuildingRef.reset();
    }
}


// * * *

void FilledLot::layout( const Options &options )
{
    // Vary the floors based on the area...
    // TODO: would be interesting to make taller buildings on smaller lots
    float area = mShape.area();
    int floors = 1 + (int) ( sqrt( area ) / 20 ) + ci::randInt( 6 );

    if ( area > 100 ) {
        mBuildingRef = Building::create( BuildingPlan( mShape.outline(), static_cast<BuildingPlan::RoofStyle>( options.building.roofStyle ) ), floors );
    }
    else {
        mBuildingRef.reset();
    }
}

// * * *

void ParkLot::layout( const Options &options ) {
    float area = mShape.area();
    float totalTreeArea = 0.0;

    while ( totalTreeArea / area < mTreeCoverRatio ) {
        // Bigger areas should get bigger trees (speeds up the generation).
        // TODO: come up with a better formula for this
        float diameter = area < 10000 ? randFloat( 4, 12 ) : randFloat( 10, 20 );

        // TODO would be good to avoid random points by the edges so the trees
        // didn't go out of their lots.
        Tree t( vec3( mShape.randomPoint(), diameter + 3 ), diameter );
        mTrees.push_back( t );

        // Treat it as a square for faster math and less dense coverage.
        totalTreeArea += diameter * diameter;
    }

}
