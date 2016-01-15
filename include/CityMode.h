#pragma once

#include "Mode.h"

class CityMode : public BaseMode
{
public:
    void setup();
    void addParams( ci::params::InterfaceGlRef params );
    void addPoint( ci::vec2 point );
    void layout();
    void draw();

    RoadNetwork mRoads;
};
