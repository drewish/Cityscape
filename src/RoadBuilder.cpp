//
//  RoadBuilder.cpp
//  Cityscape
//
//  Created by Andrew Morton on 3/5/16.
//
//

#include "RoadBuilder.h"
#include "GeometryHelpers.h"
#include "FlatShape.h"
#include "CgalPolygon.h"
#include <CGAL/Polygon_set_2.h>

using namespace std;
using namespace ci;

namespace Cityscape {

CGAL::Polygon_2<ExactK> roadOutline( const ci::vec2 &a, const ci::vec2 &b, uint8_t width = 10 )
{
    // It's faster to do this math outside of CGAL's exact kernel.
    ci::vec2 perpendicular = glm::normalize( ci::vec2( b.y - a.y, -( b.x - a.x ) ) );
    ci::vec2 offset = perpendicular * ci::vec2( width / 2.0 );

    CGAL::Polygon_2<ExactK> results;
    results.push_back( pointFrom<ExactK>( b + offset ) );
    results.push_back( pointFrom<ExactK>( b - offset ) );
    results.push_back( pointFrom<ExactK>( a - offset ) );
    results.push_back( pointFrom<ExactK>( a + offset ) );
    return results;
};

// in Highways
// out Districts and paved FlatShape
void buildHighwaysAndDistricts( CityModel &city )
{
    // Collect all the road shapes so we can insert them at once.
    vector<CGAL::Polygon_2<ExactK>> roads;
    for ( auto &h : city.highways ) {
        // TODO: not properly handling the polylines, assuming only two points
        vector<vec2> points = h->centerline.getPoints();
        roads.push_back( roadOutline( points[0], points[1], city.options.road.highwayWidth ) );
    }

    CGAL::Polygon_set_2<ExactK> paved, unpaved;
    list<CGAL::Polygon_with_holes_2<ExactK>> pavedShapes, unpavedShapes;

    paved.join( roads.begin(), roads.end() );

    paved.polygons_with_holes( back_inserter( pavedShapes ) );
    for ( auto &s : pavedShapes ) {
        city.pavement.push_back( FlatShape::create( s ) );
    }

    // Find the unpaved chunks to break up with streets
    unpaved.complement( paved );
    unpaved.polygons_with_holes( back_inserter( unpavedShapes ) );
    for ( auto &s : unpavedShapes ) {
        FlatShapeRef fs;

        if ( s.is_unbounded() ) {
            CGAL::Polygon_2<ExactK> board;
            board.push_back( ExactK::Point_2( -600, -600 ) );
            board.push_back( ExactK::Point_2(  600, -600 ) );
            board.push_back( ExactK::Point_2(  600,  600 ) );
            board.push_back( ExactK::Point_2( -600,  600 ) );
            CGAL::Polygon_with_holes_2<ExactK> outer( board );
            for ( auto hole = s.holes_begin(); hole != s.holes_end(); ++hole ) {
                outer.add_hole( *hole );
            }
            fs = FlatShape::create( outer );
        } else {
            fs = FlatShape::create( s );
        }

        city.districts.push_back( District::create( fs, city.zoningPlans.front() ) );
    }
}

// in Districts
// out Streets, Blocks and paved FlatShape
void buildStreetsAndBlocks( CityModel &city )
{
    for ( auto &district : city.districts ) {
        if ( district->zoningPlan->district.streetDivision != ZoningPlan::StreetDivision::GRID_STREET_DIVIDED ) {
            // No Block division
            continue;
        }

        const std::vector<vec2> outlinePoints = district->shape->outline().getPoints();
        vector<CGAL::Polygon_2<ExactK>> roads;

        // Create narrow roads to cover the bounding box
        uint16_t angle = city.options.road.sidestreetAngle1;
        vector<vec2> dividerPoints = computeDividers( outlinePoints, angle * M_PI / 180.0, city.options.road.blockHeight );
        assert( dividerPoints.size() % 2 == 0 );
        for ( auto a = dividerPoints.cbegin(); a != dividerPoints.cend() ; ++a ) {
            roads.push_back( roadOutline( *a, *( ++a ), city.options.road.sidestreetWidth) );
        }

        // TODO move duplicated logic to a function
        angle += city.options.road.sidestreetAngle2;
        dividerPoints = computeDividers( outlinePoints, angle * M_PI / 180.0, city.options.road.blockWidth );
        assert( dividerPoints.size() % 2 == 0 );
        for ( auto a = dividerPoints.cbegin(); a != dividerPoints.cend() ; ++a ) {
            roads.push_back( roadOutline( *a, *( ++a ), city.options.road.sidestreetWidth) );
        }

        auto districtWithHoles = district->shape->polygonWithHoles<ExactK>();

        CGAL::Polygon_set_2<ExactK> paved;
        paved.join( roads.begin(), roads.end() );
        // Find the intersection of the streets and block
        paved.intersection( districtWithHoles );

        // Add those as new streets.
        list<CGAL::Polygon_with_holes_2<ExactK>> pavedShapes, unpavedShapes;
        paved.polygons_with_holes( back_inserter( pavedShapes ) );
        for ( auto &s : pavedShapes ) {
            city.pavement.push_back( FlatShape::create( s ) );
        }

        // Find the unpaved chunks to break up with streets
        CGAL::Polygon_set_2<ExactK> unpaved( districtWithHoles );
        unpaved.difference( paved );
        unpaved.polygons_with_holes( back_inserter( unpavedShapes ) );
        for ( auto &s : unpavedShapes ) {
            district->blocks.push_back( Block::create( FlatShape::create( s ) ) );
        }
    }
}

} // Cityscape namespace
