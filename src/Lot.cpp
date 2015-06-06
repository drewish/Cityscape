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

    mBuilding = Building( Building::lshape(), mColor );

    std::vector<PolyLine2f> a = { mShape.outline() },
        b = { mBuilding.outline(buildingPosition) },
        diff = PolyLine2f::calcDifference( b,a );
    if ( diff.size() != 0 ) {
        mBuilding.mFloors = 0;
    }
    else {
        // Vary the floors based on the area...
        // TODO: would be interesting to make taller buildings on smaller lots
        float area = mShape.polygon<InexactK>().area();
        mBuilding.mFloors = (int) sqrt(area) / 20;
    }
}

void Lot::buildFullLot()
{
    buildingPosition = Vec2f::zero();

    mBuilding = Building( mShape.outline(), mColor );

    // Vary the floors based on the area...
    // TODO: would be interesting to make taller buildings on smaller lots
    float area = mShape.polygon<InexactK>().area();
    mBuilding.mFloors = (int) (sqrt(area) / 20)  + ci::randInt(5);
}

void Lot::layout()
{
    buildFullLot();
    
    mBuilding.layout();
}

void Lot::draw( const Options &options )
{
    if ( options.drawLots ) {
        gl::lineWidth( 1 );
        gl::color( ColorA( mColor, 0.4 ) );
        gl::draw( mShape.mesh() );
    }

    gl::pushModelView();
    gl::translate(buildingPosition);
    gl::rotate(buildingRotation);
    mBuilding.draw( options );
    gl::popModelView();
}
