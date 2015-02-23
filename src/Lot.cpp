//
//  Lot.cpp
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#include "Lot.h"

void Lot::draw()
{
    gl::lineWidth( 1 );
    gl::color( ColorA( 0.2f, 1.0f, 1.0f, 1.0f ) );
    gl::draw( outline );

    building.draw();
}