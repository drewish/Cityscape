#include "BuildingMode.h"

using namespace ci;
using namespace ci::app;


void BuildingMode::setup() {
    mOptions.drawBuildings = true;
    mOutline = BuildingPlan::lshape();
}

void BuildingMode::addParams( params::InterfaceGlRef params ) {
    params->addParam( "Roof", BuildingPlan::roofStyleNames(), (int*)&mOptions.building.roofStyle )
        .keyDecr( "[" ).keyIncr( "]" )
        .updateFn( std::bind( &BuildingMode::layout, this ) );
    params->addParam( "Floors", &mFloors)
        .min( 1 ).max( 5 )
        .keyDecr( "-" ).keyIncr( "=" )
        .updateFn( std::bind( &BuildingMode::layout, this ) );
    params->addSeparator();
    params->addButton( "Clear Points", [&] {
        mOutline = PolyLine2f();
        mOutline.offset( getWindowCenter() );
        layout();
    }, "key=0");
    params->addButton( "Square", [&] {
        mOutline = BuildingPlan::square();
        mOutline.offset( getWindowCenter() );
        layout();
    }, "key=1" );
    params->addButton( "Rect", [&] {
        mOutline = BuildingPlan::rectangle(60, 40);
        mOutline.offset( getWindowCenter() );
        layout();
    }, "key=2" );
    params->addButton( "L", [&] {
        mOutline = BuildingPlan::lshape();
        mOutline.offset( getWindowCenter() );
        layout();
    }, "key=3" );
    params->addButton( "T", [&] {
        mOutline = BuildingPlan::tee();
        mOutline.offset( getWindowCenter() );
        layout();
    }, "key=4" );
    params->addButton( "+", [&] {
        mOutline = BuildingPlan::plus();
        mOutline.offset( getWindowCenter() );
        layout();
    }, "key=5" );
    params->addButton( "<", [&] {
        mOutline = BuildingPlan::triangle();
        mOutline.offset( getWindowCenter() );
        layout();
    }, "key=6" );

}

void BuildingMode::addPoint( ci::vec2 point ) {
    // Don't add on to a closed outline
    if ( mOutline.isClosed() ) return;

    console() << "vec2(" << point.x << "," << point.y << "),\n";
    mOutline.push_back( point );
    layout();
}

void BuildingMode::layout() {
    if ( mOutline.size() == 0 ) {
        mBuilding.reset();
        return;
    }

    // Make sure the outline is closed
    ci::PolyLine2f outline = mOutline.getPoints();
    if ( ! outline.isClosed() ) {
        outline.push_back( outline.getPoints().front() );
    }

    mBuilding = Building::create( BuildingPlan( outline, static_cast<BuildingPlan::RoofStyle>( mOptions.building.roofStyle ) ), mFloors );
    mBuilding->layout( mOptions );
}

void BuildingMode::draw() {
    if ( mBuilding ) mBuilding->draw( mOptions );
}
