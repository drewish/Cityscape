//
//  LotDeveloper.cpp
//  Cityscape
//
//  Created by Andrew Morton on 3/26/16.
//
//

#include "LotDeveloper.h"
#include "FlatShape.h"

#include <glm/gtx/closest_point.hpp>
#include "cinder/Rand.h"
#include "GeometryHelpers.h" // used to contract lot size to avoid overflow
#include "Scenery.h"

using namespace ci;

namespace Cityscape {

float angleToLongestStreet( const LotRef &lot, const vec2 &from )
{
    float angle = 0;
    if ( lot->streetFacingSides.size() > 1 ) {
        const seg2 longestSide = *std::max_element(
            lot->streetFacingSides.begin(),
            lot->streetFacingSides.end(),
            []( const seg2 &a, const seg2 &b) {
                return glm::length2( a.first - a.second ) < glm::length2( b.first - b.second );
            }
        );
        vec2 closest = glm::closestPointOnLine( from, longestSide.first, longestSide.second );
        vec2 diff = closest - from;
        angle = atan2( diff.y, diff.x ) + M_PI_2;
    }
    return angle;
}

bool buildingOverlaps( const Scenery::Instance &building, const PolyLine2f lotOutline )
{
    PolyLine2fs l = { lotOutline };
    PolyLine2fs b = { building.footprint() };
    PolyLine2fs diff = PolyLine2f::calcDifference( b, l );
    return ( diff.size() != 0 );
}

ConeTreeRef coneTree = ConeTree::create();
SphereTreeRef sphereTree = SphereTree::create();
RowCropRef crop = RowCrop::create();

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

            lot->plants.push_back( sphereTree->instance( shape.randomPoint(), diameter ) );

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
    SceneryRef plan = mPlans[ randInt( 0, mPlans.size() ) ];

    // TODO: just placing it in the center for now. would be good to take
    // the street and a setback into consideration for the position.
    vec2 centroid = lot->shape->centroid();
    float angle = angleToLongestStreet( lot, centroid );
    Scenery::Instance building = plan->instance( centroid, angle );

    // Remove the building if it goes outside the lot.
    // TODO: try moving and/or rotating it around?
    bool buildingGoesOutOfLot = buildingOverlaps( building, lot->shape->outline() );
    if ( ! buildingGoesOutOfLot ) {
        lot->buildings.push_back( building );
    }

    // TODO: the intersection check should take tree diameter into account
    vec2 treeAt = lot->shape->randomPoint();
    if ( buildingGoesOutOfLot || ! building.footprint().contains( treeAt ) ) {
        float ratio = randFloat( 1, 3 );
        float diameter = randFloat( 5, 10 );
        lot->plants.push_back( coneTree->instance( treeAt, diameter, diameter * ratio ) );
    }
}

// * * *

bool WarehouseDeveloper::isValidFor( LotRef &lot ) const
{
    // TODO: should have a configurable minimum lot size.
    return mPlans.size() > 0 && lot->shape->area() > 300;
}
void WarehouseDeveloper::buildIn( LotRef &lot ) const
{
    for ( const FlatShape &shape : lot->shape->contract( 5 ) ) {

        // TODO: just placing it in the center for now. would be good to take
        // the street and a setback into consideration for the position.
        vec2 centroid = shape.centroid();
        float angle = angleToLongestStreet( lot, centroid );

        // Pick a random plan
        SceneryRef plan = mPlans[ randInt( 0, mPlans.size() ) ];
        Scenery::Instance building = plan->instance( centroid, angle );

        // Remove the building if it goes outside the lot.
        // TODO: try moving and/or rotating it around?
        if ( ! buildingOverlaps( building, lot->shape->outline() ) ) {
            lot->buildings.push_back( building );
        }
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
        SceneryRef plan = BuildingPlan::create( lot->shape->outline(), floors, mRoof );
        lot->buildings.push_back( plan->instance( vec2( 0 ) ) );
    }
}

// * * *


void GroupDeveloper::buildIn( LotRef &lot ) const
{
//    for ( const FlatShape &shape : lot->shape->contract( 5 ) ) {
        // TODO: just placing it in the center for now. would be good to take
        // the street and a setback into consideration for the position.
        vec2 centroid = lot->shape->centroid();
        float groupRotation = angleToLongestStreet( lot, centroid );
        mat4 transform = glm::rotate( glm::translate( vec3( centroid, 0 ) ), groupRotation, vec3( 0, 0, 1 ) );

        auto group = mGroups.at( randInt( 0, mGroups.size() ) );
        lot->buildings.push_back( group->instance( transform ) );
//    }
}

// * * *

bool SquareGridDeveloper::isValidFor( LotRef &lot ) const
{
    float area = lot->shape->area();
    return area > mRowSpacing * mStructureSpacing;
}
void SquareGridDeveloper::buildIn( LotRef &lot ) const
{
    float setback = math<float>::min( mRowSpacing, mStructureSpacing ) / 4;

    for ( const FlatShape &shape : lot->shape->contract( setback ) ) {
        std::vector<seg2> dividers = shape.dividerSeg2s( mAngle, mRowSpacing );

        for ( const seg2 &divider : dividers ) {
            vec2 v = divider.second - divider.first;
            float length = glm::length( v );
            vec2 unitVector = v / length * mStructureSpacing;
            size_t count = length / mStructureSpacing;
            for ( size_t i = 0; i < count; ++i ) {
                vec2 at = divider.first + unitVector * ( i + 0.5f );
                lot->buildings.push_back( mScenery->instance( at ) );
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
                lot->plants.push_back( sphereTree->instance( at + randVec2(), mDiameter + randFloat( 1.0 ) ) );
            }
            even = !even;
        }
    }
}

// * * *

void FarmFieldDeveloper::buildIn( LotRef &lot ) const
{
    for ( const FlatShape &shape : lot->shape->contract( mRowSpacing ) ) {
        for ( const seg2 &divider : shape.dividerSeg2s( mAngle, mRowSpacing ) ) {
            lot->plants.push_back( crop->instance( divider.first, divider.second, mRowWidth ) );
        }
    }
}

}
