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

    // TODO: like the idea of planting trees but doesn't take tree diameter
    // into account
    vec2 treeAt = lot->shape->randomPoint();
    if ( !lot->building || !lot->building->plan->outline().contains( treeAt ) ) {
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

    if ( area > 100 ) {
        int floors = 1 + (int) ( sqrt( area ) / 20 ) + ci::randInt( 6 );
        lot->building = Building::create( BuildingPlan::create( lot->shape->outline(), floors, mRoof ) );
    }
    else {
        lot->building.reset();
    }
}

// * * *

void FarmOrchardDeveloper::buildIn( LotRef &lot ) const
{
    float angle = 0.0;
    float rowSpacing = 20.0;
    float treeSpacing = 10.0;

    std::vector<seg2> dividers = lot->shape->dividerSeg2s( angle, rowSpacing );

    // Walk along each segment and plant trees
    for ( const seg2 &divider : dividers ) {
        vec2 v = divider.second - divider.first;
        float length = glm::length( v );
        vec2 unitVector = v / length * treeSpacing;
        size_t treeCount = length / treeSpacing;
        // Skip the ends to avoid tree's hanging over the edges
        for ( size_t i = 2; i < treeCount; ++i ) {
            vec2 jitter = vec2( randFloat( -1, +1 ), randFloat( -1, +1 ) );
            lot->plantTree( 5 + randFloat( 1.0 ), divider.first + ( unitVector * (float)( i - 0.5 ) ) + jitter );
        }
    }
}


}