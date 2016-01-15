#include "CityMode.h"

using namespace ci;
using namespace ci::app;

void CityMode::setup() {
    mOptions.drawBlocks = false;
    mOptions.drawLots = true;
    mOptions.drawBuildings = true;
}

void CityMode::addParams( ci::params::InterfaceGlRef params) {

    // TODO: Don't redo layout on every change, set a timer to update every half
    // second or so.
    //    params->addParam( "highwayWidth", &mOptions.road.highwayWidth )
    //        .min( 10 ).max( 50 ).step( 1 ).updateFn( std::bind( &CityMode::layout, this ) );
    //    params->addParam( "sidestreetWidth", &mOptions.road.sidestreetWidth )
    //        .min( 10 ).max( 50 ).step( 1 ).updateFn( std::bind( &CityMode::layout, this ) );
    params->addParam( "blockWidth", &mOptions.road.blockWidth )
    .min( 15 ).max( 400 ).updateFn( std::bind( &CityMode::layout, this ) );
    params->addParam( "blockHeight", &mOptions.road.blockHeight )
    .min( 15 ).max( 400 ).updateFn( std::bind( &CityMode::layout, this ) );

    params->addSeparator();

    params->addParam( "Division", {"None", "Divided"}, (int*)&mOptions.block.division )
    .updateFn( std::bind( &CityMode::layout, this ) );
    params->addParam( "Placement", {"Center", "Fill"}, (int*)&mOptions.lot.buildingPlacement )
    .updateFn( std::bind( &CityMode::layout, this ) );

    params->addSeparator();

    params->addParam( "Roads", &mOptions.drawRoads, "key=a" );
    params->addParam( "Block", &mOptions.drawBlocks, "key=s" );
    params->addParam( "Lot", &mOptions.drawLots, "key=d" );
    params->addParam( "Building", &mOptions.drawBuildings, "key=f" );

    params->addSeparator();

    params->addButton( "Clear Points", [&] {
        mRoads.clear();
        mRoads.layout( mOptions );
    }, "key=0" );
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
        mRoads.layout( mOptions );
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
        mRoads.layout( mOptions );
    }, "key=2" );
    params->addButton( "Test 3", [&] {
        mRoads.addPoints({
            vec2(163.104,60.2898),
            vec2(306.353,918.302),
            vec2(306.353,918.302),
            vec2(490.026,113.687),
            vec2(490.026,113.687),
            vec2(163.104,60.2898),
        });
        mRoads.layout( mOptions );
    }, "key=3" );
    params->addButton( "Test 4", [&] {
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
        mRoads.layout( mOptions );
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
        mRoads.layout( mOptions );
    }, "key=5" );
}

void CityMode::addPoint( ci::vec2 point ) {
    console() << "vec2(" << point.x << "," << point.y << "),\n";
    mRoads.addPoint( point );
    mRoads.layout( mOptions );
}

void CityMode::layout() {
    mRoads.layout( mOptions );
}

void CityMode::draw() {
    mRoads.draw( mOptions );
}
