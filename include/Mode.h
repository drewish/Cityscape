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
#include "RoadNetwork.h"

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
    virtual void setup() {}
    virtual void addParams( ci::params::InterfaceGlRef params) {}
    virtual void addPoint( ci::vec2 point ) {}
    virtual void layout() {}
    virtual void draw() {}

    Options   mOptions;
    ci::ivec2 mMousePos;
};
