#pragma once

#include "Mode.h"

class BuildingMode : public BaseMode
{
public:
    void setup();
    void addParams( ci::params::InterfaceGlRef params );
    void addPoint( ci::vec2 point );
    void layout();
    void draw();

    BuildingRef mBuilding;
    ci::PolyLine2f mOutline;
    int32_t mFloors = 1;
};
