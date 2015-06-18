//
//  Mode.h
//  Cityscape
//
//  Created by andrew morton on 6/16/15.
//
//

#ifndef __Cityscape__Mode__
#define __Cityscape__Mode__

#include "cinder/gl/GlslProg.h"
#include "cinder/params/Params.h"

#include "Options.h"
#include "Resources.h"
#include "RoadNetwork.h"

//using namespace ci;
using namespace ci::app;

class BaseMode;
typedef std::shared_ptr<BaseMode> ModeRef;

class BaseMode
{
public:
    BaseMode() {
        //    try {
        mOptions.buildingShader = ci::gl::GlslProg::create( loadResource( RES_VERT ), loadResource( RES_FRAG ) );
        //    }
        //    catch( gl::GlslProgCompileExc &exc ) {
        //        console() << "Shader compile error: " << std::endl;
        //        console() << exc.what();
        //    }
        //    catch( ... ) {
        //        console() << "Unable to load shader" << std::endl;
        //    }
    }
    virtual void setup() {}
    virtual void addParams( ci::params::InterfaceGlRef params) {}
    virtual void addPoint( ci::Vec2f point ) {}
    virtual void layout() {}
    virtual void draw() {}

    Options   mOptions;
    ci::Vec2i mMousePos;
};

class CityMode : public BaseMode
{
public:
    void setup();
    void addParams( ci::params::InterfaceGlRef params );
    void addPoint( ci::Vec2f point );
    void layout();
    void draw();

    RoadNetwork mRoads;
};

class BuildingMode : public BaseMode
{
public:
    void setup();
    void addParams( ci::params::InterfaceGlRef params );
    void addPoint( ci::Vec2f point );
    void layout();
    void draw();

    BuildingRef     mBuilding;
    BuildingPlan::RoofStyle mBuildingRoof = BuildingPlan::HIPPED_ROOF;
    uint32_t        mFloors = 1;
    
};

#endif /* defined(__Cityscape__Mode__) */
