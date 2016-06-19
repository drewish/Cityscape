//
//  Vehicle.cpp
//  Cityscape
//
//  Created by andrew morton on 6/19/16.
//
//

#include "VehicleMode.h"
#include "GeometryHelpers.h"

using namespace ci;

// TODO:
// - move this to its own files
// - pass elapsed time into update
// - slow down at the end of a segment
// - interpolate the rotation angle turning
// - draw a custom shape to indicate direction
// - build path from highways in city
// - use instanced drawing
// - move into the citydata structure
class Mover {
  public:

    void setup( PolyLine2f path )
    {
        mPath = path;
        mSpeed = 0;
        mSegment = 0;
        mDistance = 0;
        mLengths = distanceBetweenPointsIn( mPath );
        mAngles = anglesBetweenPointsIn( mPath );
    }

    void update()
    {
        if ( mPath.size() < 2 ) return;

        float distanceToTurn = mLengths[mSegment] - mDistance;

//        float angleDelta = ( mSegment)
//        if ( distanceToTurn < 5 ) {
//            // start turning to the new angle
//        }
//        else if ( mDistance < 5 ) {
//            // keep turning to match the new angle
//            mAngle = mAngles[mSegment];
//        }

        if ( distanceToTurn < 20 ) {
            // slowing
            float Vf = 0;
            float Vi = mSpeed;
            float d = distanceToTurn + 1;
            // http://stackoverflow.com/a/1088194/203673
            mAcceleration = (Vf*Vf - Vi*Vi)/(2 * d);
        } else if ( mSpeed < mSpeedLimit ) {
            // speeding up
            mAcceleration = 0.25;
        } else {
            // coasting
            mAcceleration = 0;
        }
        mSpeed += mAcceleration;

        mDistance += mSpeed;
        if ( mDistance > mLengths[mSegment] ) {
            mDistance = mDistance - mLengths[mSegment];
            ++mSegment;
            if ( mSegment > mLengths.size() - 1 ) {
                mSegment = 0;
            }
        }
    }

    void draw()
    {
        gl::draw( mPath );
        {
            gl::ScopedModelMatrix matrix;
            gl::translate( getPosition() );
            gl::rotate( getAngle() );
            gl::drawColorCube( vec3( 0 ), vec3( 20 ) ) ;
        }
    }

    vec2 getPosition()
    {
        if ( mPath.size() == 0 ) return vec2( 0 );
        if ( mPath.size() == 1 ) return mPath.getPoints().front();

        std::vector<vec2> &points = mPath.getPoints();
        float lerpT = mDistance / mLengths[mSegment];
        return points[mSegment] * ( 1 - lerpT ) + points[mSegment + 1] * lerpT;
    }

    float getAngle()
    {
        if ( mPath.size() < 2 ) return 0;

        return mAngles[mSegment];
    }

    PolyLine2f mPath;
    std::vector<float> mAngles;
    std::vector<float> mLengths;
    size_t mSegment;
    float mDistance;
    float mSpeed;
    float mAcceleration;
    float mAngle;
    float mSpeedLimit = 2;
};

Mover mover;


void VehicleMode::setup()
{
    mViewOptions.drawBlocks = false;
    mViewOptions.drawLots = false;
    mViewOptions.drawBuildings = true;

    mPath = polyLineTriangle().scaled( vec2( 20 ) );

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
    }, "key=2" );
}

void VehicleMode::layout() {
    mover.setup( mPath );
}

void VehicleMode::update()
{
    BaseMode::update();
    mover.update();
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
