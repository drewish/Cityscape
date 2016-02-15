//
//  Mode.h
//  Cityscape
//
//  Created by andrew morton on 6/16/15.
//
//

#pragma once

#include "cinder/gl/GlslProg.h"
#include "cinder/params/Params.h"

#include "Options.h"
#include "Resources.h"

class BaseMode;
typedef std::shared_ptr<BaseMode> ModeRef;

class BaseMode
{
  public:
    BaseMode() {
        mOptions.buildingShader = ci::gl::GlslProg::create(
           ci::app::loadResource( RES_VERT ),
           ci::app::loadResource( RES_FRAG )
       );
    }

    virtual void addParams( ci::params::InterfaceGlRef params) {}
    virtual void setup() {}
    virtual void layout() {}
    virtual void draw() {}

    virtual std::vector<ci::vec2> getPoints() { return {}; }
    virtual void addPoint( ci::vec2 point ) {}
    virtual bool isOverMovablePoint( ci::vec2 &point, float margin ) { return false; }
    virtual void movePoint( ci::vec2 from, ci::vec2 to ) {}

    Options   mOptions;
};
