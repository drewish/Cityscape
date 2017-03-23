#pragma once

#include "Mode.h"

class CityMode : public BaseMode
{
  public:
    virtual void setup() override;
    virtual void addParams( ci::params::InterfaceGlRef params ) override;
    virtual void layout() override;

    virtual std::vector<ci::vec2> getPoints() override;
    virtual void addPoint( ci::vec2 point ) override;
    virtual bool isOverMovablePoint( ci::vec2 &point, float margin ) override;
    virtual void movePoint( ci::vec2 from, ci::vec2 to ) override;

    virtual bool isOverOutline( const ci::vec2 &point, ci::PolyLine2f &outline ) override;

  private:
    std::vector<ci::PolyLine2> mHighways;
    bool isAddingRoad = false;
};
