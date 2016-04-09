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

    virtual const std::string name() const { return "Un-developer"; }
    virtual bool isValidFor( LotRef &lot ) const { return false; }
    virtual void buildIn( LotRef &lot ) const {};
};

class ParkDeveloper : public LotDeveloper {
  public:
    virtual const std::string name() const override { return "Park Builder"; }
    virtual bool isValidFor( LotRef &lot )  const override { return true; }
    virtual void buildIn( LotRef &lot ) const override;

  private:
    float mTreeCoverRatio;
};

class SingleFamilyHomeDeveloper : public LotDeveloper {
  public:
    SingleFamilyHomeDeveloper( const std::vector<BlueprintRef> &plans )
        : mPlans( plans ) {};

    virtual const std::string name() const override { return "Home Builder"; }
    virtual bool isValidFor( LotRef &lot ) const override;
    virtual void buildIn( LotRef &lot ) const override;

  private:
    std::vector<BlueprintRef> mPlans;
};

class FullLotDeveloper : public LotDeveloper {
  public:
    FullLotDeveloper( BuildingPlan::RoofStyle roof ): mRoof( roof ) {};

    virtual const std::string name() const override { return "City Builder"; }
    virtual bool isValidFor( LotRef &lot ) const override;
    virtual void buildIn( LotRef &lot ) const override;

  private:
    BuildingPlan::RoofStyle mRoof;
};

class FarmOrchardDeveloper : public LotDeveloper {
  public:
    FarmOrchardDeveloper( float angle = 0.0, float spacing = 13.0, float diameter = 5.0f )
        : mAngle( angle ), mTreeSpacing( spacing ), mDiameter( diameter ) {};

    virtual const std::string name() const override { return "Orchard Builder"; }
    virtual bool isValidFor( LotRef &lot )  const override { return true; }
    virtual void buildIn( LotRef &lot ) const override;

  private:
    float mAngle;
    float mTreeSpacing;
    float mDiameter;
};

class FarmFieldDeveloper : public LotDeveloper {

};

}