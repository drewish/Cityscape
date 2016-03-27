#pragma once

#include "Mode.h"

class BlockMode : public BaseMode
{
  public:
    virtual void setup() override;
    virtual void addParams( ci::params::InterfaceGlRef params ) override;
    virtual void layout() override;
    virtual void draw() override;

    virtual std::vector<ci::vec2> getPoints() override;
    virtual void addPoint( ci::vec2 point ) override;
    virtual bool isOverMovablePoint( ci::vec2 &point, float margin ) override;
    virtual void movePoint( ci::vec2 from, ci::vec2 to ) override;

  private:
    Cityscape::BlockRef         mBlock;
    Cityscape::ZoningPlanRef    mPlan;
    ci::PolyLine2f              mOutline;
    std::vector<ci::PolyLine2f> mHoles;
};

