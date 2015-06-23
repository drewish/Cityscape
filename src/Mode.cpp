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
            Vec2f(133,41),
            Vec2f(143,451),
            Vec2f(143,451),
            Vec2f(495,424),
            Vec2f(491,421),
            Vec2f(370,254),
            Vec2f(377,262),
            Vec2f(529,131),
        });
    }, "key=1" );
    params->addButton( "Test 2", [&] {
        mRoads.clear();
        mRoads.addPoints({
            Vec2f(133,41),
            Vec2f(143,451),
            Vec2f(143,451),
            Vec2f(495,424),
            Vec2f(491,421),
            Vec2f(370,254),
            Vec2f(377,262),
            Vec2f(529,131),
            Vec2f(131,47),
            Vec2f(523,132),
        });
    }, "key=2" );
    params->addButton( "Test 3", [&] {
        mRoads.clear();
        mRoads.addPoints({
            Vec2f(-391.031,1191.03),
            Vec2f(1026.58,1173.85),
            Vec2f(1026.58,1173.85),
            Vec2f(538.783,-52.5473),
            Vec2f(538.783,-52.5473),
            Vec2f(103.206,-48.1886),
            Vec2f(103.206,-48.1886),
            Vec2f(-391.031,1191.03),
        });
    }, "key=3" );
    params->addButton( "Test 4", [&] {
        mRoads.addPoints({
            Vec2f(163.104,60.2898),
            Vec2f(306.353,918.302),
            Vec2f(306.353,918.302),
            Vec2f(490.026,113.687),
            Vec2f(490.026,113.687),
            Vec2f(163.104,60.2898),
        });
    }, "key=4" );
    params->addButton( "Test 5", [&] {
        mRoads.clear();
        mRoads.addPoints({
            Vec2f(0.8666,1108.26),
            Vec2f(158.236,41.0269),
            Vec2f(159.075,44.556),
            Vec2f(313.45,0.94),
            Vec2f(313.45,0.94),
            Vec2f(408.625,90.0115),
            Vec2f(408.625,90.0115),
            Vec2f(331.941,319.65),
            Vec2f(331.941,319.65),
            Vec2f(313.635,1054.66),
            Vec2f(313.635,1054.66),
            Vec2f(0.1429,1069.64),
        });
    }, "key=5" );
}

void CityMode::addPoint( ci::Vec2f point ) {
    console() << "Vec2f(" << point.x << "," << point.y << "),\n";
    mRoads.addPoint( point );
}

void CityMode::layout() {
    mRoads.layout();
}

void CityMode::draw() {
    mRoads.draw( mOptions );
}

// * * *

void BuildingMode::setup() {
    mOutline = BuildingPlan::lshape();
    layout();
    mOptions.drawBuildings = true;
}

void BuildingMode::addParams( params::InterfaceGlRef params ) {
    // TODO: figure out how to get an update callback on the drop down list.
    params->addParam( "Roof", BuildingPlan::roofStyleNames(), (int*)(&mBuildingRoof) );
    params->addParam( "Floors", &mFloors).min( 1 ).max( 10 ).updateFn( std::bind( &BuildingMode::layout, this ) );
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
    }, "key=7" );
}

void BuildingMode::addPoint( ci::Vec2f point ) {
}

void BuildingMode::layout() {
    mBuilding = Building::create( BuildingPlan( mOutline, mBuildingRoof ), mFloors );
    mBuilding->layout();
}

void BuildingMode::draw() {
    gl::translate( getWindowCenter() );
    gl::rotate( 360.0 * mMousePos.x / (float) getWindowWidth() );
    gl::scale( 8, 8, 8);

    if ( mBuilding ) mBuilding->draw( mOptions );
}
