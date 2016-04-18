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
    // It's much faster to do this math outside of CGAL's exact kernel.
    PolyLine2f line = rectangleFrom( a, b, width );
    const std::vector<ci::vec2> &points = line.getPoints();
    assert( points.size() == 4 );

    CGAL::Polygon_2<ExactK> results;
    results.push_back( pointFrom<ExactK>( points[0] ) );
    results.push_back( pointFrom<ExactK>( points[1] ) );
    results.push_back( pointFrom<ExactK>( points[2] ) );
    results.push_back( pointFrom<ExactK>( points[3] ) );
    return results;
};

// in Highways
// out Districts and paved FlatShape
void buildHighwaysAndDistricts( CityModel &city )
{
    city.pavement.clear();
    city.districts.clear();

    // Collect all the road shapes so we can insert them at once.
    vector<CGAL::Polygon_2<ExactK>> roads;
    for ( auto &h : city.highways ) {
        // TODO: not properly handling the polylines, assuming only two points
        vector<vec2> points = h->centerline.getPoints();
        roads.push_back( roadOutline( points[0], points[1], city.highwayWidth ) );
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

        district->blocks.clear();

        ZoningPlanRef plan = district->zoningPlan;

        if ( plan->district.streetDivision != ZoningPlan::StreetDivision::GRID_STREET_DIVIDED ) {
            // No Block division
            district->blocks.push_back( Block::create( district->shape ) );
            continue;
        }

        const std::vector<vec2> outlinePoints = district->shape->outline().getPoints();
        vector<CGAL::Polygon_2<ExactK>> roads;

        // Create narrow roads to cover the bounding box
        uint16_t angle = plan->district.grid.avenueAngle;
        vector<seg2> dividers = computeDividers( outlinePoints, angle * M_PI / 180.0, plan->district.grid.avenueSpacing );
        for ( auto &d : dividers ) {
            roads.push_back( roadOutline( d.first, d.second, plan->district.grid.roadWidth ) );
        }

        // TODO move duplicated logic to a function
        angle += plan->district.grid.streetAngle;
        dividers = computeDividers( outlinePoints, angle * M_PI / 180.0, plan->district.grid.streetSpacing );
        for ( auto &d : dividers ) {
            roads.push_back( roadOutline( d.first, d.second, plan->district.grid.roadWidth ) );
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
// TODO: might be better for something else to collect up the streets from the
// districts. then the districts have a clear way to remove their roads.
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
