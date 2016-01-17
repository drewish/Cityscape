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


void Lot::buildInCenter( const Options &options )
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
        mBuildingRef = NULL;
    }
}

void Lot::buildFullLot( const Options &options )
{
    // Vary the floors based on the area...
    // TODO: would be interesting to make taller buildings on smaller lots
    float area = mShape.polygon<InexactK>().area();
    int floors = (int) (sqrt(area) / 20) + ci::randInt(7);

    if ( area > 100 ) {
//std::cout << mShape.outline() << std::endl;
        mBuildingRef = Building::create( BuildingPlan( mShape.outline(), static_cast<BuildingPlan::RoofStyle>( options.building.roofStyle ) ), floors );
    }
    else {
        mBuildingRef = NULL;
    }
}

void Lot::layout( const Options &options )
{
    switch ( options.lot.buildingPlacement ) {
    case LotOptions::BUILDING_IN_CENTER:
        buildInCenter( options );
        break;
    case LotOptions::BUILDING_FILL_LOT:
        buildFullLot( options );
        break;
    }

    if ( mBuildingRef ) mBuildingRef->layout( options );
}

void Lot::draw( const Options &options ) const
{
    if ( options.drawLots ) {
        gl::lineWidth( 1 );
        gl::color( ColorA( mColor, 0.4 ) );
        gl::draw( mShape.mesh() );
    }
}
