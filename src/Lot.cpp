//
//  Lot.cpp
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#include "Lot.h"

using namespace ci;

void Lot::setup()
{
    building.setup();
}

void Lot::draw()
{
    gl::lineWidth( 1 );
    gl::color( ColorA( mColor, 0.4 ) );
    gl::draw( mShape.mesh() );

    gl::pushModelView();
    gl::translate(buildingPosition);
    building.draw();
    gl::popModelView();
}

void Lot::place( const Building b ) {
    building = b;
    // TODO: just placing it in the center for now. would be good to take
    // the street into consideration.
    buildingPosition = mShape.centroid();
}