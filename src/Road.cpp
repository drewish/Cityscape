//
//  Road.cpp
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#include "Road.h"

void Road::setup()
{
}

void Road::draw()
{
    gl::color( ColorA( 0.3f, 0.3f, 0.3f, 0.4f ) );
    gl::drawSolid( outline );
}