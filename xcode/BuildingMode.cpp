#include "BuildingMode.h"

using namespace ci;
using namespace ci::app;


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
    mBuilding->layout( mOptions );
}

void BuildingMode::draw() {
    gl::translate( getWindowCenter() );
    gl::rotate( 2 * M_PI * mMousePos.x / (float) getWindowWidth() );
    gl::scale( 8, 8, 8);

    if ( mBuilding ) mBuilding->draw( mOptions );
}
