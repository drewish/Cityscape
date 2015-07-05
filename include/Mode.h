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
    virtual void addPoint( ci::vec2 point ) {}
    virtual void layout() {}
    virtual void draw() {}

    Options   mOptions;
    ci::ivec2 mMousePos;
};

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

#include "CinderCGAL.h"
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>
typedef CGAL::Arr_segment_traits_2<ExactK>            Traits_2;
typedef CGAL::Arrangement_2<Traits_2>                 Arrangement_2;

class BlockMode : public BaseMode
{
public:
    void setup();
    void addParams( ci::params::InterfaceGlRef params );
    void addPoint( ci::vec2 point );
    void layout();
    void draw();

    Arrangement_2 mArr;
    BuildingRef mBlock;
    ci::PolyLine2f mOutline;
};

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
    BuildingPlan::RoofStyle mBuildingRoof = BuildingPlan::SAWTOOTH_ROOF;
    int32_t mFloors = 1;
    
};

#endif /* defined(__Cityscape__Mode__) */
