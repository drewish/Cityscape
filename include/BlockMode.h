#pragma once

#include "Mode.h"
#include "Block.h"

class BlockMode : public BaseMode
{
public:
    void setup();
    void addParams( ci::params::InterfaceGlRef params );
    void addPoint( ci::vec2 point );
    void layout();
    void draw();

    BlockRef        mBlock;
    ci::PolyLine2f  mOutline;
};

