//
//  Building.h
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#pragma once

#include "BuildingPlan.h"
#include "Options.h"

class Building;
typedef std::shared_ptr<Building> BuildingRef;


class Building {
  public:

    static BuildingRef create( const BuildingPlan &plan, uint32_t floors = 1,
        ci::vec2 position = ci::vec2( 0 ), float rotation = 0 )
    {
        return BuildingRef( new Building( plan, floors, position, rotation ) );
    }

    Building( const BuildingPlan &plan, uint32_t floors = 1,
        ci::vec2 position = ci::vec2(0, 0), float rotation = 0 )
        : mPlan(plan), mFloors(floors), mPosition(position), mRotation(rotation)
    {};
    Building( const Building &s )
        : mPlan(s.mPlan), mFloors(s.mFloors), mPosition(s.mPosition), mRotation(s.mRotation)
    {};

    void layout( const Options &options );

    const BuildingPlan plan() { return mPlan; };
    const ci::PolyLine2f outline() const
    {
        return mPlan.outline(mPosition, mRotation);
    }

// TODO: move to CityModel
//private:
    BuildingPlan mPlan;
    uint32_t mFloors;
    ci::vec2 mPosition;
    float mRotation; // radians
};
