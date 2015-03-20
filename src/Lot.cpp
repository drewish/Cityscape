//
//  Lot.cpp
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#include "Lot.h"


#include "CinderCGAL.h"

void Lot::setup()
{
    building.setup();
}

void Lot::draw()
{
    gl::lineWidth( 1 );
    gl::color( ColorA( mColor, 0.4 ) );
    gl::drawSolid( outline );

    gl::pushModelView();
    gl::translate(buildingPosition);
    building.draw();
    gl::popModelView();
}

void Lot::place( const Building b ) {
    building = b;
    // TODO: just placing it in the center for now. would be good to take
    // the street into consideration.
    buildingPosition = vecFrom(getCentroid(polyFrom(outline)));
}