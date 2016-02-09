//
//  Lot.cpp
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#include "Lot.h"
#include "cinder/Rand.h"

using namespace ci;

void Lot::drawGround( const Options &options ) const
{
    if ( options.drawLots ) {
        gl::lineWidth( 1 );
        gl::color( ColorA( mColor, 0.4 ) );
        gl::draw( mShape.mesh() );
    }
}

// * * *

void SingleBuildingLot::layout( const Options &options )
{
    int floors = 1 + randInt(2);

    mBuildingRef = Building::create(
        BuildingPlan( BuildingPlan::randomOutline(), BuildingPlan::RANDOM_ROOF ),
        floors,
        // TODO: just placing it in the center for now. would be good to take
        // the street into consideration.
        mShape.centroid(),
        M_PI * randInt(4) / 2
    );
    if (!mBuildingRef) { return; }

    std::vector<PolyLine2f> a = { mShape.outline() },
        b = { mBuildingRef->outline() },
        diff = PolyLine2f::calcDifference( b, a );
    if ( diff.size() != 0 ) {
        mBuildingRef.reset();
    }

    if ( mBuildingRef ) mBuildingRef->layout( options );
}


// * * *

void FilledLot::layout( const Options &options )
{
    // Vary the floors based on the area...
    // TODO: would be interesting to make taller buildings on smaller lots
    float area = mShape.area();
    int floors = (int) (sqrt( area ) / 20) + ci::randInt( 7 );

    if ( area > 100 ) {
        mBuildingRef = Building::create( BuildingPlan( mShape.outline(), static_cast<BuildingPlan::RoofStyle>( options.building.roofStyle ) ), floors );
    }
    else {
        mBuildingRef.reset();
    }

    if ( mBuildingRef ) mBuildingRef->layout( options );
}

// * * *

void ParkLot::layout( const Options &options ) {
    float area = mShape.area();
    float totalTreeArea = 0.0;

    geom::SourceMods trees;
    while ( totalTreeArea / area < mTreeCoverRatio ) {
        float diameter = randFloat( 4, 12 );
        // Treat it as a square for faster math and less dense coverage.
        totalTreeArea += diameter * diameter;
        // TODO would be good to avoid random points by the edges so the trees
        // didn't go out of their lots.
        trees.append( makeTree( mShape.randomPoint(), diameter ) );
    }

    gl::GlslProgRef shader = gl::getStockShader( gl::ShaderDef().color() );
    mBatch = gl::Batch::create( trees, shader );
}

void ParkLot::drawStructures( const Options &options ) const {
    gl::ScopedColor scopedColor( ColorA8u( 0x69, 0x98, 0x38, 0xC0 ) );
    mBatch->draw();
}

geom::SourceMods ParkLot::makeTree( const vec2 &at, const float diameter ) const {
    return geom::Sphere().subdivisions( 12 ).radius( diameter ).center( vec3( at, diameter + 3 ) );
}
