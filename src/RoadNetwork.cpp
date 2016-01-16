//
//  RoadNetwork.cpp
//  Cityscape
//
//  Created by Andrew Morton on 5/31/15.
//
//

#include "RoadNetwork.h"
#include "Road.h"
#include "Lot.h"

#include "CinderCGAL.h"

using namespace ci;

void RoadNetwork::buildHighways( const Options &options, CGAL::Polygon_set_2<ExactK> &paved )
{
    float roadWidth = 20.0;

    std::vector<CGAL::Polygon_2<ExactK>> roads;
    for( uint i = 1, size = mPoints.size(); i < size; i += 2 ) {
        roads.push_back( polygonFrom<ExactK>( Road( mPoints[i-1], mPoints[i], roadWidth ).outline ) );
    }
    // It's more efficient for CGAL to insert all at once:
    paved.join( roads.begin(), roads.end() );
}

// Add some secondary streets
void RoadNetwork::buildSideStreets( const Options &options, CGAL::Polygon_set_2<ExactK> &paved )
{
// TODO:
//  - make roadway orientation configurable

    CGAL::Polygon_set_2<ExactK> unpaved;
    std::list<CGAL::Polygon_with_holes_2<ExactK>> unpavedPwh;

    // Find the unpaved chunks to break up with streets
    unpaved.complement(paved);
    unpaved.polygons_with_holes( std::back_inserter( unpavedPwh ) );
    for ( auto chunk = unpavedPwh.begin(); chunk != unpavedPwh.end(); ++chunk ) {
        if ( chunk->is_unbounded() ) continue;

// TODO: this is begging to be moved into a function:
        // Create narrow roads to cover the bounding box

        CGAL::Bbox_2 bounds = chunk->bbox();
        std::vector<CGAL::Polygon_2<ExactK>> roads;
        for ( float y = bounds.ymin() + options.road.blockHeight; y < bounds.ymax(); y += options.road.blockHeight ) {
            roads.push_back( polygonFrom<ExactK>( Road( vec2( bounds.xmin(), y ), vec2( bounds.xmax(), y ), options.road.sidestreetWidth).outline ) );
        }
        for ( float x = bounds.xmin() + options.road.blockWidth; x < bounds.xmax(); x += options.road.blockWidth ) {
            roads.push_back( polygonFrom<ExactK>( Road( vec2( x, bounds.ymin() ), vec2( x, bounds.ymax() ), options.road.sidestreetWidth).outline ) );
        }

        CGAL::Polygon_set_2<ExactK> newStreets;
        // It's more efficient for CGAL to insert all at once:
        newStreets.join( roads.begin(), roads.end() );

        // Find the intersection of the streets and block
        newStreets.intersection( *chunk );

        // Add those as new streets.
        paved.join( newStreets );
    }

}

// Should probably be called build the rest... a bit of a code smell here.
void RoadNetwork::buildBlocks( const Options &options )
{
    CGAL::Polygon_set_2<ExactK> paved, unpaved;
    std::list<CGAL::Polygon_with_holes_2<ExactK>> pavedShapes, unpavedShapes;

    buildHighways( options, paved );
    // TODO: Make this step optional
    buildSideStreets( options, paved );

    // Get the expanded street network
    paved.polygons_with_holes( std::back_inserter( pavedShapes ) );
    for ( auto it = pavedShapes.begin(); it != pavedShapes.end(); ++it ) {
        mShapes.push_back( FlatShape( *it ) );
    }

    // Extract the final blocks
    unpavedShapes.clear();
    unpaved.complement(paved);
    unpaved.polygons_with_holes( std::back_inserter( unpavedShapes ) );
    for ( auto it = unpavedShapes.begin(); it != unpavedShapes.end(); ++it ) {
        if ( it->is_unbounded() ) continue;

        mBlocks.push_back( Block( FlatShape( *it ), ColorA( 1.0, 1.0, 0.0, 0.3 ) ) );
    }
}

void RoadNetwork::layout( const Options &options )
{
    // Don't bother redoing the layout if we have an odd number of points.
    if ( mPoints.size() % 2 == 1 ) return;

    mBlocks.clear();
    mShapes.clear();

    buildBlocks( options );

    for( auto it = mBlocks.begin(); it != mBlocks.end(); ++it ) {
        it->layout( options );
    }
}

void RoadNetwork::draw( const Options &options )
{
    if ( options.drawRoads ) {
        gl::color( ColorA( 0.3f, 0.3f, 0.3f, 0.4f ) );
        for ( auto &shape : mShapes ) {
            gl::draw( shape.mesh() );
        }
    }

    for( const Block &block : mBlocks ) {
        block.draw( options );
        // Sort of tacky to go in like this but the hope is that all the lots
        // draw atop the blocks.
        for( const Lot &lot : block.mLots ) {
            lot.draw( options );
        }
    }

    // Draw buildings on top of lots and blocks
    for( const Block &block : mBlocks ) {
        // Sort of tacky to go in like this but the hope is that all the lots
        // draw atop the blocks.
        for( const Lot &lot : block.mLots ) {
            if (lot.mBuildingRef) lot.mBuildingRef->draw( options );
        }
    }

}
