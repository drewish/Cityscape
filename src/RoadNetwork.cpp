//
//  RoadNetwork.cpp
//  Cityscape
//
//  Created by Andrew Morton on 5/31/15.
//
//

#include "RoadNetwork.h"
#include "Lot.h"
#include "CgalPolygon.h"
#include "GeometryHelpers.h"

using namespace ci;
using namespace std;

void RoadNetwork::buildHighways( const Options &options, CGAL::Polygon_set_2<ExactK> &paved )
{
    vector<CGAL::Polygon_2<ExactK>> roads;
    for( uint i = 1, size = mPoints.size(); i < size; i += 2 ) {
        roads.push_back( roadOutline( mPoints[i-1], mPoints[i], options.road.highwayWidth ) );
    }
    // It's more efficient for CGAL to insert all at once:
    paved.join( roads.begin(), roads.end() );
}

// Add some secondary streets
void RoadNetwork::buildSideStreets( const Options &options, CGAL::Polygon_set_2<ExactK> &paved )
{
    CGAL::Polygon_set_2<ExactK> unpaved;
    list<CGAL::Polygon_with_holes_2<ExactK>> unpavedPwh;

    // Find the unpaved chunks to break up with streets
    unpaved.complement(paved);
    unpaved.polygons_with_holes( back_inserter( unpavedPwh ) );
    for ( auto chunk : unpavedPwh ) {
        if ( chunk.is_unbounded() ) continue;

        const std::vector<vec2> outlinePoints = polyLineFrom( chunk.outer_boundary() ).getPoints();
        vector<CGAL::Polygon_2<ExactK>> roads;

        // Create narrow roads to cover the bounding box
        uint16_t angle = options.road.sidestreetAngle1;
        vector<vec2> dividerPoints = computeDividers( outlinePoints, angle * M_PI / 180.0, options.road.blockHeight );
        assert( dividerPoints.size() % 2 == 0 );
        for ( auto a = dividerPoints.cbegin(); a != dividerPoints.cend() ; ++a ) {
            roads.push_back( roadOutline( *a, *( ++a ), options.road.sidestreetWidth) );
        }

        angle += options.road.sidestreetAngle2;
        // TODO move duplicated logic to a function
        dividerPoints =  computeDividers( outlinePoints, angle * M_PI / 180.0, options.road.blockWidth );
        assert( dividerPoints.size() % 2 == 0 );
        for ( auto a = dividerPoints.cbegin(); a != dividerPoints.cend() ; ++a ) {
            roads.push_back( roadOutline( *a, *( ++a ), options.road.sidestreetWidth) );
        }

        CGAL::Polygon_set_2<ExactK> newStreets;
        newStreets.join( roads.begin(), roads.end() );
        // Find the intersection of the streets and block
        newStreets.intersection( chunk );
        // Add those as new streets.
        paved.join( newStreets );
    }
}

// Should probably be called build the rest... a bit of a code smell here.
void RoadNetwork::buildBlocks( const Options &options )
{
    CGAL::Polygon_set_2<ExactK> paved, unpaved;
    list<CGAL::Polygon_with_holes_2<ExactK>> pavedShapes, unpavedShapes;

    buildHighways( options, paved );

    unpaved.complement( paved );

//    // TODO: Make this step optional
//    if ( false ) {
//        buildSideStreets( options, paved );
//        unpaved.complement( paved );
//    }

    // Extract the final blocks
    unpaved.polygons_with_holes( back_inserter( unpavedShapes ) );
    float hue = 0;
    std::cout << "\n\n" << unpavedShapes.size() << " unpavedshapes\n";
    for ( auto &s : unpavedShapes ) {

        printPolygon( s );

        ColorA color = ColorA( CM_HSV, hue, 1.0, 1.0, 0.5 );
        if ( s.is_unbounded() ) {
// TODO: This could go up before we do side streets to get sub divided.
            CGAL::Polygon_2<ExactK> board;
            board.push_back( ExactK::Point_2( -600, -600 ) );
            board.push_back( ExactK::Point_2(  600, -600 ) );
            board.push_back( ExactK::Point_2(  600,  600 ) );
            board.push_back( ExactK::Point_2( -600,  600 ) );
            CGAL::Polygon_with_holes_2<ExactK> outer( board );
            for ( auto hole = s.holes_begin(); hole != s.holes_end(); ++hole ) {
                outer.add_hole( *hole );
            }
            mBlocks.push_back( Block( FlatShape( outer ), color) );
        } else {
            mBlocks.push_back( Block( FlatShape( s ), color) );
        }
        hue += 0.17;
        if (hue > 1) hue -= 1.0;
    }

    // Keep the final street network
    paved.polygons_with_holes( back_inserter( pavedShapes ) );
    for ( auto &s : pavedShapes ) {
        mRoadShapes.push_back( FlatShape( s ) );
    }
}

CGAL::Polygon_2<ExactK> RoadNetwork::roadOutline( const ci::vec2 &a, const ci::vec2 &b, uint8_t width )
{
    // It's much faster to do this math outside of CGAL.
    ci::vec2 perpendicular = glm::normalize( ci::vec2( b.y - a.y, -( b.x - a.x ) ) );
    ci::vec2 offset = perpendicular * ci::vec2( width / 2.0 );

    CGAL::Polygon_2<ExactK> results;
    results.push_back( pointFrom<ExactK>( b + offset ) );
    results.push_back( pointFrom<ExactK>( b - offset ) );
    results.push_back( pointFrom<ExactK>( a - offset ) );
    results.push_back( pointFrom<ExactK>( a + offset ) );
    return results;
};

void RoadNetwork::layout( const Options &options )
{
    // Don't bother redoing the layout if we have an odd number of points.
    if ( mPoints.size() % 2 == 1 ) return;

    mBlocks.clear();
    mRoadShapes.clear();

    buildBlocks( options );

    for( auto it = mBlocks.begin(); it != mBlocks.end(); ++it ) {
        it->layout( options );
    }
}

