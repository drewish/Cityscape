//
//  Options.h
//  Cityscape
//
//  Created by Andrew Morton on 5/21/15.
//
//

#pragma once

#include "cinder/gl/GlslProg.h"

struct RoadOptions {
    float highwayWidth = 20;
    float sidestreetWidth = 10;
    float blockHeight = 200;
    float blockWidth = 100;
};

struct BlockOptions {
    enum BlockDivision {
        NO_BLOCK_DIVISION = 0,
        BLOCK_DIVIDED = 1,
    };

    BlockDivision division = BLOCK_DIVIDED;
};

struct LotOptions {
    enum BuildingPlacement {
        BUILDING_IN_CENTER = 0,
        BUILDING_FILL_LOT = 1,
    };

   BuildingPlacement buildingPlacement = BUILDING_IN_CENTER;
};

struct Options {
    bool drawRoads = true;
    bool drawBlocks = false;
    bool drawLots = true;
    bool drawBuildings = true;
    cinder::gl::GlslProgRef	buildingShader;

    RoadOptions road;
    BlockOptions block;
    LotOptions lot;
};
