//
//  Options.h
//  Cityscape
//
//  Created by Andrew Morton on 5/21/15.
//
//

#ifndef Cityscape_Options_h
#define Cityscape_Options_h

#include "cinder/gl/GlslProg.h"

struct Options {
    bool drawRoads = true;
    bool drawBlocks = false;
    bool drawLots = true;
    bool drawBuildings = true;
    cinder::gl::GlslProgRef	buildingShader;
};

#endif
