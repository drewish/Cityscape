//
//  Vehicle.cpp
//  Cityscape
//
//  Created by andrew morton on 6/19/16.
//
//

#include "VehicleMode.h"
#include "GeometryHelpers.h"
#include "Vehicle.h"

using namespace ci;

Vehicle mover;


void VehicleMode::setup()
{
    mViewOptions.drawBlocks = false;
    mViewOptions.drawLots = false;
    mViewOptions.drawBuildings = true;

    mPath = polyLineCircle( 500, 10 );

    layout();
}

void VehicleMode::addParams( params::InterfaceGlRef params ) {
    params->addButton( "Clear Points", [&] {
        mPath = PolyLine2f();
        requestLayout();
    }, "key=0");
    params->addButton( "Circle", [&] {
        mPath = polyLineCircle( 500, 10 );
        requestLayout();
    }, "key=1" );
    params->addButton( "Tee", [&] {
        mPath = polyLineTee().scaled( vec2( 20 ) );
        requestLayout();
    }, "key=2" );
    params->addButton( "Triangle", [&] {
        mPath = polyLineTriangle().scaled( vec2( 20 ) );
        requestLayout();
    }, "key=3" );
}

void VehicleMode::layout() {
    mover.setup( mPath );
}

void VehicleMode::update( double elapsed )
{
    BaseMode::update( elapsed );
    mover.update( elapsed );
}

void VehicleMode::draw()
{
    BaseMode::draw();
    mover.draw();
}


std::vector<ci::vec2> VehicleMode::getPoints()
{
    return mPath.getPoints();
}

void VehicleMode::addPoint( ci::vec2 point ) {
    // Don't add on to a closed outline
    if ( mPath.isClosed() ) return;

    mPath.push_back( point );
    requestLayout();
}

bool VehicleMode::isOverMovablePoint( ci::vec2 &point, float margin )
{
    for ( const auto &other : mPath ) {
        if ( length2( point - other ) < margin * margin ) {
            // Snap their point to ours
            point = other;
            return true;
        }
    }
    return false;
}

void VehicleMode::movePoint( ci::vec2 from, ci::vec2 to )
{
    PolyLine2f newPath;
    for ( const auto &p : mPath ) {
        newPath.push_back( from == p ? to : p );
    }
    newPath.setClosed( mPath.isClosed() );

    mPath = newPath;

    layout();
}
