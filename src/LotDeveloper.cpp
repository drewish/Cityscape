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

bool compareSeg2Length( const seg2 &a, const seg2 &b )
{
    return glm::length2( a.first - a.second ) < glm::length2( b.first - b.second );
}

seg2 longestStreetSide( const LotRef &lot )
{
    if ( lot->streetFacingSides.size() == 0 ) {
        return seg2( vec2( 0 ), vec2( 0 ) );
    }

    return *std::max_element( lot->streetFacingSides.begin(), lot->streetFacingSides.end(), compareSeg2Length );
}

seg2 longestEdgeIn( const FlatShape &shape ) {
    std::vector<seg2> edges = shape.edges();

    if ( edges.size() == 0 ) {
        return seg2( vec2( 0 ), vec2( 0 ) );
    }

    return *std::max_element( edges.begin(), edges.end(), compareSeg2Length );
}

float angleToLongestStreet( const LotRef &lot, const vec2 &from )
{
    if ( lot->streetFacingSides.size() == 0 ) {
        return 0;
    }

    const seg2 longestSide = longestStreetSide( lot );
    vec2 closest = glm::closestPointOnLine( from, longestSide.first, longestSide.second );
    vec2 diff = closest - from;
    return atan2( diff.y, diff.x ) + M_PI_2;
}

float angleOfLongestEdge( const FlatShape &shape )
{
    if ( shape.outline().size() == 0 ) {
        return 0;
    }

    const seg2 longestSide = longestEdgeIn( shape );
    vec2 diff = longestSide.first - longestSide.second;
    float ang = atan2( -diff.y, diff.x ) + M_PI_2;
    return ang;
}

bool buildingOverlaps( const Scenery::Instance &building, const PolyLine2f lotOutline )
{
    PolyLine2fs l = { lotOutline };
    PolyLine2fs b = { building.footprint() };
    PolyLine2fs diff = PolyLine2f::calcDifference( b, l );
    return ( diff.size() != 0 );
}

// Evaluate several positions
boost::optional<Scenery::Instance> findPosition( const LotRef &lot, u_int8_t tries, std::function<Scenery::Instance()> generateInstance )
{
    bool validSpot = false;
    while( !validSpot && tries > 0 ) {
        Scenery::Instance instance = generateInstance();
        validSpot = !buildingOverlaps( instance, lot->shape->outline() );
        if ( validSpot ) {
            return instance;
        }
        tries--;
    };
    return {};
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
    Rand r;
    for ( const FlatShape &shape : lot->shape->contract( 5 ) ) {
        auto maybeInstance = findPosition( lot, 3, [&]
            {
                // TODO: just placing it in the center for now. would be good to take
                // the street and a setback into consideration for the position.
                vec2 centroid = shape.centroid();
                float angle = angleToLongestStreet( lot, centroid );

                // Pick a random plan
                SceneryRef plan = mPlans[ r.nextUint( mPlans.size() ) ];
                return plan->instance( centroid, angle );
            }
        );
        if( maybeInstance ) {
            lot->buildings.push_back( maybeInstance.value() );

            // TODO: the intersection check should take tree diameter into account
            vec2 treeAt = lot->shape->randomPoint();
            if (  ! maybeInstance->footprint().contains( treeAt ) ) {
                float ratio = randFloat( 1, 3 );
                float diameter = randFloat( 5, 10 );
                lot->plants.push_back( coneTree->instance( treeAt, diameter, diameter * ratio ) );
            }
        }
    }
}

// * * *

bool PickFromListDeveloper::isValidFor( LotRef &lot ) const
{
    // TODO: should have a configurable minimum lot size.
    return mPlans.size() > 0 && lot->shape->area() > 300;
}
void PickFromListDeveloper::buildIn( LotRef &lot ) const
{
    Rand r;
    for ( const FlatShape &shape : lot->shape->contract( 5 ) ) {
        auto maybeInstance = findPosition( lot, 3, [&]
            {
                // TODO: just placing it in the center for now. would be good to take
                // the street and a setback into consideration for the position.
                vec2 centroid = shape.centroid();
                float angle = angleToLongestStreet( lot, centroid );

                // Pick a random plan
                SceneryRef plan = mPlans[ r.nextUint( mPlans.size() ) ];
                return plan->instance( centroid, angle );
            }
        );
        if( maybeInstance ) {
            lot->buildings.push_back( maybeInstance.value() );
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
    // TODO: would be nice if there was an easier way to add holes:
    PolyLine2 outline = lot->shape->outline();
    PolyLine2fs holes = lot->shape->holes();

    if( mBuilding ) {
        boost::optional<Scenery::Instance> maybeInstance = findPosition( lot, 3, [&]
            {
                vec2 houseAt = lot->shape->randomPoint();
                return mBuilding->instance( houseAt, angleToLongestStreet( lot, houseAt ) );
            }
        );
        if( maybeInstance ) {
            lot->buildings.push_back( maybeInstance.value() );
            holes.push_back( maybeInstance->footprint().reversed() );
        }
    }

    FlatShape shapeWithBuilding( outline, holes );
    for( const FlatShape &shape : shapeWithBuilding.contract( mRowSpacing ) ) {
        float angle = angleOfLongestEdge( shape );
        for( const seg2 &divider : shape.dividerSeg2s( angle, mRowSpacing ) ) {
            lot->plants.push_back( crop->instance( divider.first, divider.second, mRowWidth ) );
        }
    }
}

} // namespace Cityscape
