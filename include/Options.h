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
    uint8_t highwayWidth = 40;
    uint8_t sidestreetWidth = 20;
    int16_t sidestreetAngle1 = 0; // -180 - +180 degrees
    int16_t sidestreetAngle2 = 90; // -90 - +90 degrees
    uint16_t blockHeight = 200;
    uint16_t blockWidth = 100;
};

struct BlockOptions {
    enum BlockDivision {
        NO_BLOCK_DIVISION = 0,
        BLOCK_DIVIDED = 1,
    };

    BlockDivision division = BLOCK_DIVIDED;
    int16_t lotWidth = 40;
};

struct LotOptions {
    enum BuildingPlacement {
        BUILDING_IN_CENTER = 0,
        BUILDING_FILL_LOT = 1,
    };

    BuildingPlacement buildingPlacement = BUILDING_FILL_LOT;
};

struct BuildingOptions {
    int roofStyle = 1;
};

struct Options {
    bool drawRoads = true;
    bool drawBlocks = false;
    bool drawLots = false;
    bool drawBuildings = true;
    cinder::gl::GlslProgRef	buildingShader;

    RoadOptions road;
    BlockOptions block;
    LotOptions lot;
    BuildingOptions building;
};
