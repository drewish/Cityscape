//
//  Building.cpp
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#include "Building.h"


void Building::draw()
{
    gl::color( ColorA( 1.0f, 0.0f, 0.0f, 0.8f ) );
    gl::drawSolid( outline );
}