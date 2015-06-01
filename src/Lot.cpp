//
//  Lot.cpp
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#include "Lot.h"

using namespace ci;

void Lot::layout()
{
    // TODO: just placing it in the center for now. would be good to take
    // the street into consideration.
    buildingPosition = mShape.centroid();

    mBuilding = Building( Building::lshape() );
    mBuilding.mColor = mColor;

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
