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
    buildingRotation = 90 * randInt(4);

    mBuildingRef = Building::createRandom();


    std::vector<PolyLine2f> a = { mShape.outline() },
        b = { mBuildingRef->outline(buildingPosition) },
        diff = PolyLine2f::calcDifference( b,a );
    if ( diff.size() != 0 ) {
        mBuildingRef = NULL;
    }
    else {
        // Vary the floors based on the area...
        // TODO: would be interesting to make taller buildings on smaller lots
        float area = mShape.polygon<InexactK>().area();
        mBuildingRef->mFloors = (int) sqrt(area) / 20;
    }
}

void Lot::buildFullLot()
{
    buildingPosition = Vec2f::zero();

    mBuildingRef = Building::create( mShape.outline() );

    // Vary the floors based on the area...
    // TODO: would be interesting to make taller buildings on smaller lots
    float area = mShape.polygon<InexactK>().area();
    mBuildingRef->mFloors = (int) (sqrt(area) / 20)  + ci::randInt(5);
}

void Lot::layout()
{
    buildInCenter();

    if ( mBuildingRef ) mBuildingRef->layout();
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