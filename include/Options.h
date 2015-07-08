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
    enum BlockDivision {
        NO_BLOCK_DIVISION = 0,
        BLOCK_DIVIDED = 1,
    };
    enum BuildingPlacement {
        BUILDING_IN_CENTER = 0,
        BUILDING_FILL_LOT = 1,
    };

    bool drawRoads = true;
    bool drawBlocks = false;
    bool drawLots = true;
    bool drawBuildings = true;
    BlockDivision blockDivision = BLOCK_DIVIDED;
    BuildingPlacement buildingPlacement = BUILDING_FILL_LOT;
    cinder::gl::GlslProgRef	buildingShader;
};

#endif
