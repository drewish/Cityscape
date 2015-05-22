//
//  Road.cpp
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#include "Road.h"

using namespace ci;

void Road::layout()
{
}

void Road::draw( const Options &options )
{
    if ( options.drawRoads ) {
        gl::color( ColorA( 0.3f, 0.3f, 0.3f, 0.4f ) );
        gl::drawSolid( outline );
    }
}