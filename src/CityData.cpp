//
//  CityData.cpp
//  Cityscape
//
//  Created by Andrew Morton on 3/5/16.
//
//

#include "CityData.h"
#include "FlatShape.h"
#include "BuildingPlan.h"

#include "cinder/Rand.h" // Not needed once the LotDeveloper stuff moves out

using namespace ci;

namespace Cityscape {

// Give relatively unique colors
ColorA colorWheel()
{
    static float hue = 0.0f;
    ColorA color = ci::ColorA( ci::CM_HSV, hue, 1.0, 1.0, 0.5 );
    hue += 0.17f;
    if (hue > 1) hue -= 1.0f;
    return color;
}


// * * *
// TODO: Needs to move elsewhere

class ParkDeveloper : public LotDeveloper {
  public:
    virtual const std::string name() const override { return "Park Builder"; }
    virtual bool isValidFor( LotRef &lot )  const override { return true; }
    virtual void buildIn( LotRef &lot ) const override
    {
        float area = lot->shape->area();
        float treeCoverage = randFloat( 0.25, 0.75 );
        float totalTreeArea = 0.0;

        while ( totalTreeArea / area < treeCoverage ) {
            // Bigger areas should get bigger trees (speeds up the generation).
            // TODO: come up with a better formula for this
            float diameter = area < 10000 ? randFloat( 4, 12 ) : randFloat( 10, 20 );

            // TODO would be good to avoid random points by the edges so the trees
            // didn't go out of their lots.
            lot->trees.push_back( Tree::create( ci::vec3( lot->shape->randomPoint(), diameter + 3 ), diameter ) );

            // Treat it as a square for faster math and less dense coverage.
            totalTreeArea += diameter * diameter;
        }
    }

  private:
    float mTreeCoverRatio;
};

class SingleFamilyHomeDeveloper : public LotDeveloper {
  public:
    SingleFamilyHomeDeveloper( const std::vector<BuildingPlanRef> &plans ): mPlans( plans ) {};

    virtual const std::string name() const override { return "Home Builder"; }
    virtual bool isValidFor( LotRef &lot ) const override
    {
        // TODO: should have a configurable minimum lot size.
        return mPlans.size() > 0 && lot->shape->area() > 100;
    }
    virtual void buildIn( LotRef &lot ) const override
    {
        // Pick a random plan
        auto plan = mPlans[ randInt( 0, mPlans.size() ) ];

        // TODO: just placing it in the center for now. would be good to take
        // the street into consideration for setback and orientation.
        lot->building = Building::create( plan, lot->shape->centroid() );
        if ( !lot->building ) { return; }

        // Remove the building goes outside the lot
        // TODO: try moving it around?
        std::vector<PolyLine2f> a = { lot->shape->outline() },
            b = { lot->building->plan->outline( lot->building->position, lot->building->rotation ) },
            diff = PolyLine2f::calcDifference( b, a );
        if ( diff.size() != 0 ) {
            lot->building.reset();
        }
    }

  private:
    std::vector<BuildingPlanRef> mPlans;
};

class FullLotDeveloper : public LotDeveloper {
  public:
    FullLotDeveloper( BuildingPlan::RoofStyle roof ): mRoof( roof ) {};

    virtual const std::string name() const override { return "Home Builder"; }
    virtual bool isValidFor( LotRef &lot ) const override
    {
        float area = lot->shape->area();
        return area > 100;
    }
    virtual void buildIn( LotRef &lot ) const override
    {
        // Vary the floors based on the area...
        // TODO: would be interesting to make taller buildings on smaller lots
        float area = lot->shape->area();

        if ( area > 100 ) {
            int floors = 1 + (int) ( sqrt( area ) / 20 ) + ci::randInt( 6 );
            lot->building = Building::create( BuildingPlan::create( lot->shape->outline(), mRoof, floors ) );
        }
        else {
            lot->building.reset();
        }
    }
  private:
    BuildingPlan::RoofStyle mRoof;
};
// END Move elsewhere
// * * *


CityModel::CityModel()
{
    ZoningPlanRef plan = ZoningPlan::create( "default" );

    plan->lotUsages.push_back( ZoningPlan::LotUsage( nullptr, 1 ) );
    plan->lotUsages.push_back( ZoningPlan::LotUsage( LotDeveloperRef( new SingleFamilyHomeDeveloper( {
            BuildingPlan::create( BuildingPlan::rectangle( 30, 10 ), BuildingPlan::HIPPED_ROOF ),
            BuildingPlan::create( BuildingPlan::rectangle( 30, 10 ), BuildingPlan::GABLED_ROOF ),
            BuildingPlan::create( BuildingPlan::lshape(), BuildingPlan::HIPPED_ROOF ),
            BuildingPlan::create( BuildingPlan::lshape(), BuildingPlan::GABLED_ROOF )

        } ) ), 30 ) );
    plan->lotUsages.push_back( ZoningPlan::LotUsage( LotDeveloperRef( new ParkDeveloper() ), 2 ) );

    zoningPlans = { plan };
}

}