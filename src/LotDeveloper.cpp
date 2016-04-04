//
//  LotDeveloper.cpp
//  Cityscape
//
//  Created by Andrew Morton on 3/26/16.
//
//

#include "LotDeveloper.h"
#include "FlatShape.h"

#include "cinder/Rand.h"
#include "GeometryHelpers.h" // used to contract lot size to avoid overflow

using namespace ci;

namespace Cityscape {

void ParkDeveloper::buildIn( LotRef &lot ) const
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
        lot->plantTree( diameter, lot->shape->randomPoint() );

        // Treat it as a square for faster math and less dense coverage.
        totalTreeArea += diameter * diameter;
    }
}

// * * *

bool SingleFamilyHomeDeveloper::isValidFor( LotRef &lot ) const
{
    // TODO: should have a configurable minimum lot size.
    return mPlans.size() > 0 && lot->shape->area() > 100;
}
void SingleFamilyHomeDeveloper::buildIn( LotRef &lot ) const
{
    // Pick a random plan
    auto plan = mPlans[ randInt( 0, mPlans.size() ) ];

    // TODO: just placing it in the center for now. would be good to take
    // the street into consideration for setback and orientation.
    BuildingRef building = Building::create( plan, lot->shape->centroid() );
    if ( building ) {
        // Remove the building goes outside the lot
        // TODO: try moving and/or rotating it around?
        std::vector<PolyLine2f> a = { lot->shape->outline() },
            b = { building->plan->outline( building->position, building->rotation ) },
            diff = PolyLine2f::calcDifference( b, a );
        if ( diff.size() != 0 ) {
            building.reset();
        }
    }
    if ( building ) {
        lot->buildings.push_back( building );
    }

    // TODO: like the idea of planting trees but doesn't take tree diameter
    // into account
    vec2 treeAt = lot->shape->randomPoint();
    if ( !building || !building->plan->outline().contains( treeAt ) ) {
        lot->plantTree( randFloat( 5, 12 ), treeAt );
    }
}

// * * *

bool FullLotDeveloper::isValidFor( LotRef &lot ) const
{
    float area = lot->shape->area();
    return area > 100;
}
void FullLotDeveloper::buildIn( LotRef &lot ) const
{
    // Vary the floors based on the area...
    // TODO: would be interesting to make taller buildings on smaller lots
    float area = lot->shape->area();

    lot->buildings.clear();

    if ( area > 100 ) {
        int floors = 1 + (int) ( sqrt( area ) / 20 ) + ci::randInt( 6 );
        lot->buildings.push_back( Building::create( BuildingPlan::create( lot->shape->outline(), floors, mRoof ) ) );
    }
}

// * * *

void FarmOrchardDeveloper::buildIn( LotRef &lot ) const
{
    std::vector<seg2> dividers = lot->shape->contract( 5 ).dividerSeg2s( mAngle, mTreeSpacing );

    // Walk along each segment and plant trees. Orchards often use a quincunx
    // pattern so every other row should start halfway between. Note: holes in
    // the lot will cause glitches with this simple algorithm.
    bool even = true;
    for ( const seg2 &divider : dividers ) {
        vec2 v = divider.second - divider.first;
        float length = glm::length( v );
        vec2 unitVector = v / length * mTreeSpacing;
        size_t treeCount = length / mTreeSpacing;
        for ( size_t i = 1; i < treeCount; ++i ) {
            vec2 at = divider.first + unitVector * ( i + ( even ? 0.0f : 0.5f ) );
            lot->plantTree( mDiameter + randFloat( 1.0 ), at + randVec2(), CIRCULAR_TREE );
        }
        even = !even;
    }
}


}