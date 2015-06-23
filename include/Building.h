//
//  Building.h
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#ifndef __Cityscape__Building__
#define __Cityscape__Building__

//#include <boost/flyweight.hpp>
//#include <boost/flyweight/key_value.hpp>
//using namespace ::boost::flyweights;

#include "BuildingPlan.h"
#include "Options.h"

class Building;
typedef std::shared_ptr<Building> BuildingRef;


class Building {
  public:

    static BuildingRef create( const BuildingPlan plan, const uint32_t floors ) {
        return BuildingRef( new Building( plan, floors ) );
    }

    static BuildingRef createRandom( const uint32_t floors, const BuildingPlan::RoofStyle roof ) {
        return BuildingRef( new Building( BuildingPlan::random( roof ), floors ) );
    }

    Building( const BuildingPlan plan, const uint32_t floors = 1.0 ) : mPlan(plan), mFloors(floors) { };
    Building( const Building &s ) : mPlan(s.mPlan), mFloors(s.mFloors) { };

    void layout();
    void draw( const Options &options ) const;

    const BuildingPlan plan() { return mPlan; };

private:
    BuildingPlan mPlan;
    uint32_t mFloors;

};

#endif /* defined(__Cityscape__Building__) */
