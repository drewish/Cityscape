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
        gl::color( ColorA( 0.3f, 0.3f, 0.3f, 1.0f ) );
        gl::drawSolid( outline );
    }
}

const Rectf Road::bounds()
{
    auto p = outline.begin();
    Rectf bounds( *p, *p );
    for ( ++p; p != outline.end(); ++p ) {
        bounds.include( *p );
    }
    return bounds;
}