//
//  VehicleMode.h
//  Cityscape
//
//  Created by andrew morton on 6/19/16.
//
//

#pragma once

#include "Mode.h"

class VehicleMode : public BaseMode
{
  public:
    virtual void setup() override;
    virtual void addParams( ci::params::InterfaceGlRef params ) override;
    virtual void layout() override;
    virtual void update( double elapsed ) override;
    virtual void draw() override;

    virtual std::vector<ci::vec2> getPoints() override;
    virtual void addPoint( ci::vec2 point ) override;
    virtual bool isOverMovablePoint( ci::vec2 &point, float margin ) override;
    virtual void movePoint( ci::vec2 from, ci::vec2 to ) override;
//    virtual bool isOverOutline( const ci::vec2 &point, ci::PolyLine2f &outline ) override;

  private:
    ci::PolyLine2f mPath;
};
