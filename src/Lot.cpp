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


void Lot::buildInCenter()
{
    // TODO: just placing it in the center for now. would be good to take
    // the street into consideration.
    buildingPosition = mShape.centroid();
    buildingRotation = M_PI * randInt(4) / 2;

    int floors = 1 + randInt(2);

    mBuildingRef = Building::create( BuildingPlan( BuildingPlan::randomOutline(), BuildingPlan::randomRoof() ), floors );
    if (!mBuildingRef) { return; }

// TODO: figure out why this stopped working after the cinder upgrade
    std::vector<PolyLine2f> a = { mShape.outline() },
        b = { mBuildingRef->plan().outline(buildingPosition) },
        diff = PolyLine2f::calcDifference( b,a );
    if ( diff.size() != 0 ) {
        mBuildingRef = NULL;
    }
}

void Lot::buildFullLot()
{
    buildingPosition = glm::zero<ci::vec2>();

    // Vary the floors based on the area...
    // TODO: would be interesting to make taller buildings on smaller lots
    float area = mShape.polygon<InexactK>().area();
    int floors = (int) (sqrt(area) / 20) + ci::randInt(7);

    if ( area > 100 ) {
        mBuildingRef = Building::create( BuildingPlan( mShape.outline(), BuildingPlan::randomRoof() ), floors );
    }
    else {
        mBuildingRef = NULL;
    }
}

void Lot::layout( const Options &options )
{
    switch ( options.buildingPlacement ) {
    case Options::BUILDING_IN_CENTER:
        buildInCenter();
        break;
    case Options::BUILDING_FILL_LOT:
        buildFullLot();
        break;
    }

    if ( mBuildingRef ) mBuildingRef->layout( options );
}

void Lot::draw( const Options &options )
{
    if ( options.drawLots ) {
        gl::lineWidth( 1 );
        gl::color( ColorA( mColor, 0.4 ) );
        gl::draw( mShape.mesh() );
    }
}

void Lot::drawBuilding( const Options &options )
{
    if ( !mBuildingRef ) return;

    gl::pushModelView();
    gl::translate(buildingPosition);
    gl::rotate(buildingRotation);
    mBuildingRef->draw( options );
    gl::popModelView();
}