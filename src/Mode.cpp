//
//  Mode.cpp
//  Cityscape
//
//  Created by andrew morton on 6/16/15.
//
//

#include "Mode.h"

using namespace ci;
using namespace ci::app;

void CityMode::setup() {
    mOptions.drawBlocks = false;
    mOptions.drawLots = true;
    mOptions.drawBuildings = true;
}

void CityMode::addParams( ci::params::InterfaceGlRef params) {
    params->addParam( "Roads", &mOptions.drawRoads, "key=a" );
    params->addParam( "Block", &mOptions.drawBlocks, "key=s" );
    params->addParam( "Lot", &mOptions.drawLots, "key=d" );
    params->addParam( "Building", &mOptions.drawBuildings, "key=f" );
    params->addParam( "Clip City", &mOptions.clipCityLimit, "key=c" );

    params->addButton( "Clear Points", [&] { mRoads.clear(); }, "key=0" );
    params->addButton( "Test 1", [&] {
        mRoads.clear();
        mRoads.addPoints({
            vec2(133,41),
            vec2(143,451),
            vec2(143,451),
            vec2(495,424),
            vec2(491,421),
            vec2(370,254),
            vec2(377,262),
            vec2(529,131),
        });
    }, "key=1" );
    params->addButton( "Test 2", [&] {
        mRoads.clear();
        mRoads.addPoints({
            vec2(133,41),
            vec2(143,451),
            vec2(143,451),
            vec2(495,424),
            vec2(491,421),
            vec2(370,254),
            vec2(377,262),
            vec2(529,131),
            vec2(131,47),
            vec2(523,132),
        });
    }, "key=2" );
    params->addButton( "Test 3", [&] {
        mRoads.clear();
        mRoads.addPoints({
            vec2(-391.031,1191.03),
            vec2(1026.58,1173.85),
            vec2(1026.58,1173.85),
            vec2(538.783,-52.5473),
            vec2(538.783,-52.5473),
            vec2(103.206,-48.1886),
            vec2(103.206,-48.1886),
            vec2(-391.031,1191.03),
        });
    }, "key=3" );
    params->addButton( "Test 4", [&] {
        mRoads.addPoints({
            vec2(163.104,60.2898),
            vec2(306.353,918.302),
            vec2(306.353,918.302),
            vec2(490.026,113.687),
            vec2(490.026,113.687),
            vec2(163.104,60.2898),
        });
    }, "key=4" );
    params->addButton( "Test 5", [&] {
        mRoads.clear();
        mRoads.addPoints({
            vec2(0.8666,1108.26),
            vec2(158.236,41.0269),
            vec2(159.075,44.556),
            vec2(313.45,0.94),
            vec2(313.45,0.94),
            vec2(408.625,90.0115),
            vec2(408.625,90.0115),
            vec2(331.941,319.65),
            vec2(331.941,319.65),
            vec2(313.635,1054.66),
            vec2(313.635,1054.66),
            vec2(0.1429,1069.64),
        });
    }, "key=5" );
}

void CityMode::addPoint( ci::vec2 point ) {
    console() << "vec2(" << point.x << "," << point.y << "),\n";
    mRoads.addPoint( point );
}

void CityMode::layout() {
    mRoads.layout();
}

void CityMode::draw() {
    mRoads.draw( mOptions );
}

// * * *

#include <CGAL/Sweep_line_2_algorithms.h>
#include <CGAL/Arr_naive_point_location.h>

typedef Traits_2::Point_2                             Point_2;
typedef Traits_2::X_monotone_curve_2                  Segment_2;
typedef CGAL::Arr_naive_point_location<Arrangement_2> Naive_pl;


void BlockMode::setup() {
    mOptions.drawBlocks = false;
    mOptions.drawLots = true;
    mOptions.drawBuildings = true;
}

void BlockMode::addParams( ci::params::InterfaceGlRef params) {
    params->addParam( "Roads", &mOptions.drawRoads, "key=a" );
    params->addParam( "Block", &mOptions.drawBlocks, "key=s" );
    params->addParam( "Lot", &mOptions.drawLots, "key=d" );
    params->addParam( "Building", &mOptions.drawBuildings, "key=f" );
    params->addParam( "Clip City", &mOptions.clipCityLimit, "key=c" );

    params->addButton( "Clear Points", [&] {
        mOutline = PolyLine2f();
        layout();
    }, "key=0");
    params->addButton( "Test 1", [&] {
        mOutline = PolyLine2f({
            vec2(133,41),
            vec2(143,451),
            vec2(495,424),
            vec2(370,254),
            vec2(529,131),
        });
        layout();
    }, "key=1" );
    params->addButton( "Test 2", [&] {
        mOutline = PolyLine2f({
            vec2(133,41),
            vec2(143,451),
            vec2(495,424),
            vec2(370,254),
            vec2(529,131),
            vec2(133,41),
        });
        layout();
    }, "key=2" );
}

void BlockMode::addPoint( ci::vec2 point ) {
    console() << "vec2(" << point.x << "," << point.y << "),\n";
    mOutline.push_back( point );
    layout();
}

void BlockMode::layout() {
    mArr.clear();
    if (mOutline.size() == 0) return;

    // Put the outline onto the arrangment.
    std::list<Segment_2> outlineSegments;
    for ( auto prev = mOutline.begin(), i = prev + 1; i != mOutline.end(); ++i ) {
        outlineSegments.push_back( Segment_2 (Point_2( prev->x, prev->y ), Point_2( i->x, i->y ) ) );
        prev = i;
    }
    insert_empty( mArr, outlineSegments.begin(), outlineSegments.end() );

    // Create a bounding box for the outline...
    Rectf bounds( mOutline.getPoints() );
    float x = bounds.x1;
    float width = 100;
    // ...and a list of segements to intersect with.
    std::list<Segment_2> intersect;
    intersect.insert( intersect.begin(), outlineSegments.begin(), outlineSegments.end() );

    std::list<Segment_2> newEdges;
    // Then start walking across the outline looking for the intersections...
    while ( x < bounds.x2 ) {
        //
        intersect.push_back( Segment_2( Point_2( x, bounds.y2 ), Point_2( x, bounds.y1 ) ) );

        std::vector<Point_2> pts;
        CGAL::compute_intersection_points( intersect.begin(), intersect.end(), std::back_inserter(pts) );

        // Handle an odd number of intersections (open path).
        Naive_pl pl(mArr);
        if ( pts.size() % 2 == 1 ) insert_point( mArr, pts[0], pl );

        // Handle an even number of interesections.
        for ( int i = pts.size() - 1; i > 0; i -= 2 ) {
            newEdges.push_back( Segment_2 ( pts[i - 1], pts[i] ) );
        }

        intersect.pop_back();
        x += width;
    };
    // Add the new edges all at once for better performance.
    if (newEdges.size()) insert( mArr, newEdges.begin(), newEdges.end() );

}

void BlockMode::draw() {
    for ( auto i = mArr.vertices_begin(); i != mArr.vertices_end(); ++i ) {
        vec3 v = vec3( vecFrom( i->point() ), 0 );
        gl::drawColorCube( v, vec3( 10 ) );
    }

    gl::color(1, 0, 0 );
    for ( auto i = mArr.edges_begin(); i != mArr.edges_end(); ++i ) {
        PolyLine2f p = PolyLine2f({ vecFrom( i->source()->point() ), vecFrom( i->target()->point() ) } );
        gl::draw( p );
    }

    gl::color(1, 1, 1 );
    for ( auto i = mArr.faces_begin(); i != mArr.faces_end(); ++i ) {
        for ( auto j = i->holes_begin(); j != i->holes_end(); ++j ) {
            PolyLine2f faceOutline;
            Arrangement_2::Ccb_halfedge_circulator cc = *j;
            do {
                Arrangement_2::Halfedge_handle he = cc;
                faceOutline.push_back( vecFrom( he->target()->point() ) );
            } while ( ++cc != *j );
            gl::drawSolid( faceOutline );
        }
    }
}

// * * *

void BuildingMode::setup() {
    mOptions.drawBuildings = true;
    mOutline = BuildingPlan::lshape();
}

void BuildingMode::addParams( params::InterfaceGlRef params ) {
    params->addParam( "Roof", BuildingPlan::roofStyleNames(), (int*)(&mBuildingRoof) )
        .keyDecr( "[" ).keyIncr( "]" )
        .updateFn( std::bind( &BuildingMode::layout, this ) );
    params->addParam( "Floors", &mFloors)
        .min( 1 ).max( 5 )
        .keyDecr( "-" ).keyIncr( "=" )
        .updateFn( std::bind( &BuildingMode::layout, this ) );
    params->addSeparator();
    params->addButton( "Square", [&] {
        mOutline = BuildingPlan::square();
        layout();
    }, "key=1" );
    params->addButton( "Rect", [&] {
        mOutline = BuildingPlan::rectangle(60, 40);
        layout();
    }, "key=2" );
    params->addButton( "L", [&] {
        mOutline = BuildingPlan::lshape();
        layout();
    }, "key=3" );
    params->addButton( "T", [&] {
        mOutline = BuildingPlan::tee();
        layout();
    }, "key=4" );
    params->addButton( "+", [&] {
        mOutline = BuildingPlan::plus();
        layout();
    }, "key=5" );
    params->addButton( "<", [&] {
        mOutline = BuildingPlan::triangle();
        layout();
    }, "key=6" );
}

void BuildingMode::addPoint( ci::vec2 point ) {
}

void BuildingMode::layout() {
    mBuilding = Building::create( BuildingPlan( mOutline, mBuildingRoof ), mFloors );
    mBuilding->layout();
}

void BuildingMode::draw() {
    gl::translate( getWindowCenter() );
    gl::rotate( 2 * M_PI * mMousePos.x / (float) getWindowWidth() );
    gl::scale( 8, 8, 8);

    if ( mBuilding ) mBuilding->draw( mOptions );
}
