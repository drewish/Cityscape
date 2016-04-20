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
#include "Scenery.h"

using namespace ci;

namespace Cityscape {

ConeTreeRef coneTree = ConeTree::create();
SphereTreeRef sphereTree = SphereTree::create();

void ParkDeveloper::buildIn( LotRef &lot ) const
{
    // Shrink the area so trees don't hang out of the sides
    for ( const FlatShape &shape : lot->shape->contract( 5 ) ) {
        float area = shape.area();
        float treeCoverage = randFloat( 0.25, 0.75 );
        float totalTreeArea = 0.0;

        while ( totalTreeArea / area < treeCoverage ) {
            // Bigger areas should get bigger trees (speeds up the generation).
            // TODO: come up with a better formula for this
            float diameter = area < 10000 ? randFloat( 4, 12 ) : randFloat( 10, 20 );

            lot->plants.push_back( sphereTree->createInstace( shape.randomPoint(), diameter ) );

            // Treat it as a square for faster math and less dense coverage.
            totalTreeArea += diameter * diameter;
        }
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
    Scenery::InstanceRef building = plan->createInstace( lot->shape->centroid(), 0 );
    if ( building ) {
        // Remove the building goes outside the lot
        // TODO: try moving and/or rotating it around?
        std::vector<PolyLine2f> a = { lot->shape->outline() },
            b = { building->footprint() },
            diff = PolyLine2f::calcDifference( b, a );
        if ( diff.size() != 0 ) {
            building.reset();
        }
    }
    if ( building ) {
        lot->buildings.push_back( building );
    }

    // TODO: like the idea of planting trees but the intersection check doesn't take tree diameter
    // into account
    vec2 treeAt = lot->shape->randomPoint();
    if ( !building || !building->footprint().contains( treeAt ) ) {
        float ratio = randFloat( 1, 3 );
        float diameter = randFloat( 5, 10 );
        lot->plants.push_back( coneTree->createInstace( treeAt, diameter, diameter * ratio ) );
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

        // It's kind of odd that we're passing the coordinates in via the
        // outline and having no instance offset. I guess it doesn't matter
        // since we're not reusing the plan or rotating it.
        BuildingPlanRef plan = BuildingPlan::create( lot->shape->outline(), floors, mRoof );
        lot->buildings.push_back( plan->createInstace( vec3( 0 ) ) );
    }
}

// * * *

bool SquareGridDeveloper::isValidFor( LotRef &lot ) const
{
    float area = lot->shape->area();
    return area > mRowSpacing * mStructureSpacing;
}
void SquareGridDeveloper::buildIn( LotRef &lot ) const
{
    float setback = math<float>::max( mRowSpacing, mStructureSpacing ) / 2;

    for ( const FlatShape &shape : lot->shape->contract( setback ) ) {
        std::vector<seg2> dividers = shape.dividerSeg2s( mAngle, mRowSpacing );

        for ( const seg2 &divider : dividers ) {
            vec2 v = divider.second - divider.first;
            float length = glm::length( v );
            vec2 unitVector = v / length * mStructureSpacing;
            size_t count = length / mStructureSpacing;
            for ( size_t i = 0; i < count; ++i ) {
                vec2 at = divider.first + unitVector * ( i + 0.5f );
                lot->buildings.push_back( mStructure->createInstace( at ) );
            }
        }
    }
}

// * * *

void FarmOrchardDeveloper::buildIn( LotRef &lot ) const
{
    for ( const FlatShape &shape : lot->shape->contract( mDiameter / 2.0 ) ) {
        std::vector<seg2> dividers = shape.dividerSeg2s( mAngle, mTreeSpacing );

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
                lot->plants.push_back( sphereTree->createInstace( at + randVec2(), mDiameter + randFloat( 1.0 ) ) );
            }
            even = !even;
        }
    }
}


}