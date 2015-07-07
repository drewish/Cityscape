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
    u_int16_t blockDivision;
    u_int16_t buildingPlacement;
    cinder::gl::GlslProgRef	buildingShader;
};

#endif
