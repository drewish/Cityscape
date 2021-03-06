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
    virtual bool isValidFor( LotRef &lot ) const { return true; }
    virtual void buildIn( LotRef &lot ) const {};
};

class ParkDeveloper : public LotDeveloper {
  public:
    virtual void buildIn( LotRef &lot ) const override;

  private:
    float mTreeCoverRatio;
};

class SingleFamilyHomeDeveloper : public LotDeveloper {
  public:
    SingleFamilyHomeDeveloper( const std::vector<SceneryRef> &plans )
        : mPlans( plans ) {};

    virtual bool isValidFor( LotRef &lot ) const override;
    virtual void buildIn( LotRef &lot ) const override;

  private:
    std::vector<SceneryRef> mPlans;
};

class PickFromListDeveloper : public LotDeveloper {
  public:
    PickFromListDeveloper( const std::vector<SceneryRef> &plans )
        : mPlans( plans ) {};

    virtual bool isValidFor( LotRef &lot ) const override;
    virtual void buildIn( LotRef &lot ) const override;

  private:
    std::vector<SceneryRef> mPlans;
};

class FullLotDeveloper : public LotDeveloper {
  public:
    FullLotDeveloper( RoofStyle roof ): mRoof( roof ) {};

    virtual bool isValidFor( LotRef &lot ) const override;
    virtual void buildIn( LotRef &lot ) const override;

    const RoofStyle mRoof;
};

class SquareGridDeveloper : public LotDeveloper {
  public:
    SquareGridDeveloper( SceneryRef scenery, float rowSpacing, float structureSpacing, float angle = 0.0 )
    :   mScenery( scenery ), mRowSpacing( rowSpacing ), mStructureSpacing( structureSpacing ), mAngle( angle )
    {};

    virtual bool isValidFor( LotRef &lot )  const override;
    virtual void buildIn( LotRef &lot ) const override;

    const float mAngle;
    const float mRowSpacing;
    const float mStructureSpacing;
    const SceneryRef mScenery;
};

class FarmOrchardDeveloper : public LotDeveloper {
  public:
    FarmOrchardDeveloper( float angle = 0.0, float spacing = 13.0, float diameter = 5.0f )
        : mAngle( angle ), mTreeSpacing( spacing ), mDiameter( diameter ) {};

    virtual void buildIn( LotRef &lot ) const override;

    const float mAngle;
    const float mTreeSpacing;
    const float mDiameter;
};

class FarmFieldDeveloper : public LotDeveloper {
  public:
    FarmFieldDeveloper( float rowSpacing = 10.0, float rowWidth = 5.0f, const SceneryRef building = nullptr )
        : mRowSpacing( rowSpacing ), mRowWidth( rowWidth ), mBuilding( building ) {};

    virtual void buildIn( LotRef &lot ) const override;

    const float mRowSpacing;
    const float mRowWidth;
    const SceneryRef mBuilding;
};

}
