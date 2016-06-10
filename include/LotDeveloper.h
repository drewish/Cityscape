//
//  LotDeveloper.h
//  Cityscape
//
//  Created by Andrew Morton on 3/26/16.
//
//

#pragma once

#include "CityData.h"
#include "BuildingPlan.h"

namespace Cityscape {

class LotDeveloper {
  public:
    virtual ~LotDeveloper() {};
    virtual bool isValidFor( LotRef &lot ) const { return false; }
    virtual void buildIn( LotRef &lot ) const {};
};

class ParkDeveloper : public LotDeveloper {
  public:
    virtual bool isValidFor( LotRef &lot )  const override { return true; }
    virtual void buildIn( LotRef &lot ) const override;

  private:
    float mTreeCoverRatio;
};

class SingleFamilyHomeDeveloper : public LotDeveloper {
  public:
    SingleFamilyHomeDeveloper( const std::vector<BuildingPlanRef> &plans )
        : mPlans( plans ) {};

    virtual bool isValidFor( LotRef &lot ) const override;
    virtual void buildIn( LotRef &lot ) const override;

  private:
    std::vector<BuildingPlanRef> mPlans;
};

class WarehouseDeveloper : public LotDeveloper {
  public:
    WarehouseDeveloper( const std::vector<BuildingPlanRef> &plans )
        : mPlans( plans ) {};

    virtual bool isValidFor( LotRef &lot ) const override;
    virtual void buildIn( LotRef &lot ) const override;

  private:
    std::vector<BuildingPlanRef> mPlans;
};

class FullLotDeveloper : public LotDeveloper {
  public:
    FullLotDeveloper( BuildingPlan::RoofStyle roof ): mRoof( roof ) {};

    virtual bool isValidFor( LotRef &lot ) const override;
    virtual void buildIn( LotRef &lot ) const override;

    const BuildingPlan::RoofStyle mRoof;
};

class SquareGridDeveloper : public LotDeveloper {
  public:
    typedef std::function<Scenery::InstanceRef( const ci::vec2& )> Builder;

    SquareGridDeveloper( Builder sceneryBuilder, float rowSpacing, float structureSpacing, float angle = 0.0 )
    :   mSceneryBuilder( sceneryBuilder ), mRowSpacing( rowSpacing ),
        mStructureSpacing( structureSpacing ), mAngle( angle )
    {};

    virtual bool isValidFor( LotRef &lot )  const override;
    virtual void buildIn( LotRef &lot ) const override;

    const float mAngle;
    const float mRowSpacing;
    const float mStructureSpacing;
    const Builder mSceneryBuilder;
};

class FarmOrchardDeveloper : public LotDeveloper {
  public:
    FarmOrchardDeveloper( float angle = 0.0, float spacing = 13.0, float diameter = 5.0f )
        : mAngle( angle ), mTreeSpacing( spacing ), mDiameter( diameter ) {};

    virtual bool isValidFor( LotRef &lot )  const override { return true; }
    virtual void buildIn( LotRef &lot ) const override;

    const float mAngle;
    const float mTreeSpacing;
    const float mDiameter;
};

class FarmFieldDeveloper : public LotDeveloper {
  public:
    FarmFieldDeveloper( float angle = 0.0, float rowSpacing = 10.0, float rowWidth = 5.0f )
        : mAngle( angle ), mRowSpacing( rowSpacing ), mRowWidth( rowWidth ) {};

    virtual bool isValidFor( LotRef &lot )  const override { return true; }
    virtual void buildIn( LotRef &lot ) const override;

    const float mAngle;
    const float mRowSpacing;
    const float mRowWidth;
};

}