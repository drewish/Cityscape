#pragma once

#include "Mode.h"
#include "BuildingPlan.h"

class BuildingMode : public BaseMode
{
  public:
    virtual void setup() override;
    virtual void addParams( ci::params::InterfaceGlRef params ) override;
    virtual void layout() override;

    virtual std::vector<ci::vec2> getPoints() override;
    virtual void addPoint( ci::vec2 point ) override;
    virtual bool isOverMovablePoint( ci::vec2 &point, float margin ) override;
    virtual void movePoint( ci::vec2 from, ci::vec2 to ) override;

    void setOutline( const ci::PolyLine2f &outline );

  private:
    ci::PolyLine2f mOutline;
    BuildingSettings mBuildingSettings;
};
