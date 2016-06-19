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

#include "CityView.h"

class BaseMode;
typedef std::shared_ptr<BaseMode> ModeRef;

class BaseMode
{
  public:
    BaseMode() {
    }

    virtual void addParams( ci::params::InterfaceGlRef params) {}
    virtual void setup() {}
    virtual void layout() {}
    virtual void draw() { if ( mCityView ) mCityView->draw( mViewOptions ); }
    virtual void update()
    {
        // TODO: Don't redo layout on every change, set a timer to update every
        // half second or so.
        if ( mLayoutNeeded ) {
            layout();
            mLayoutNeeded = false;
        }
    }
    void requestLayout() { mLayoutNeeded = true; }

    virtual std::vector<ci::vec2> getPoints() { return {}; }
    virtual void addPoint( ci::vec2 point ) {}
    virtual bool isOverMovablePoint( ci::vec2 &point, float margin ) { return false; }
    virtual void movePoint( ci::vec2 from, ci::vec2 to ) {}

    virtual bool isOverOutline( const ci::vec2 &point, ci::PolyLine2f &outline ) { return false; }

    bool                mLayoutNeeded = true;
    Cityscape::CityModel  mModel;
    CityView::Options   mViewOptions;
    CityViewRef         mCityView;
};
