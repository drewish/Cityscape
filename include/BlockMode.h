#pragma once

#include "Mode.h"

class BlockMode : public BaseMode
{
public:
    void setup();
    void addParams( ci::params::InterfaceGlRef params );
    void addPoint( ci::vec2 point );
    void layout();
    void draw();

    BuildingRef mBlock;
    ci::PolyLine2f mOutline;
};

