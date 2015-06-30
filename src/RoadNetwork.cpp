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

void RoadNetwork::buildHighways( CGAL::Polygon_set_2<ExactK> &paved )
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
void RoadNetwork::buildSideStreets( CGAL::Polygon_set_2<ExactK> &paved, const float blockWidth, const float blockHeight )
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
        float roadWidth = 10.0;

        CGAL::Bbox_2 bounds = chunk->bbox();
        std::vector<CGAL::Polygon_2<ExactK>> roads;
        for ( float y = bounds.ymin() + blockHeight; y < bounds.ymax(); y += blockHeight ) {
            roads.push_back( polygonFrom<ExactK>( Road( vec2( bounds.xmin(), y ), vec2( bounds.xmax(), y ), roadWidth).outline ) );
        }
        for ( float x = bounds.xmin() + blockWidth; x < bounds.xmax(); x += blockWidth ) {
            roads.push_back( polygonFrom<ExactK>( Road( vec2( x, bounds.ymin() ), vec2( x, bounds.ymax() ), roadWidth).outline ) );
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
void RoadNetwork::buildBlocks()
{
    CGAL::Polygon_set_2<ExactK> paved, unpaved;
    std::list<CGAL::Polygon_with_holes_2<ExactK>> pavedShapes, unpavedShapes;

    buildHighways( paved );
    // TODO: Expose these configuration items
    // TODO: Make this step optional
    buildSideStreets( paved, 100, 150 );


//    if (mOptions.clipCityLimit) {
//        Vec2i windowSize = getWindowSize();
//        CGAL::Polygon_2<ExactK> window;
//        window.push_back( ExactK::Point_2( 0, 0 ) );
//        window.push_back( ExactK::Point_2( windowSize.x, 0 ) );
//        window.push_back( ExactK::Point_2( windowSize.x, windowSize.y ) );
//        window.push_back( ExactK::Point_2( 0, windowSize.y ) );
//
//        paved.intersection(window); // Intersect with the clipping rectangle.
//    }

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

void RoadNetwork::layout()
{
    mBlocks.clear();
    mShapes.clear();

    buildBlocks();

    for( auto it = mBlocks.begin(); it != mBlocks.end(); ++it ) {
        it->layout();
    }
}

void RoadNetwork::draw( const Options &options )
{
    if ( options.drawRoads ) {
        gl::color( ColorA( 0.3f, 0.3f, 0.3f, 0.4f ) );
        for ( auto it = mShapes.begin(); it != mShapes.end(); ++it ) {
            gl::draw( it->mesh() );
        }
    }

    for( auto block = mBlocks.begin(); block != mBlocks.end(); ++block ) {
        block->draw( options );
        // Sort of tacky to go in like this but the hope is that all the lots
        // draw atop the blocks.
        for( auto lot = block->mLots.begin(); lot != block->mLots.end(); ++lot ) {
            lot->draw( options );
        }
    }

    // Draw buildings on top of lots and blocks
    for( auto block = mBlocks.begin(); block != mBlocks.end(); ++block ) {
        // Sort of tacky to go in like this but the hope is that all the lots
        // draw atop the blocks.
        for( auto lot = block->mLots.begin(); lot != block->mLots.end(); ++lot ) {
            lot->drawBuilding( options );
        }
    }

}
