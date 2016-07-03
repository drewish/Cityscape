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

#include <boost/format.hpp>
#include <glm/gtx/vector_angle.hpp>
// TODO:
// - interpolate the rotation angle turning
// - only closed polylines should loop 
// - move this to its own files
// - draw a custom shape to indicate direction
// - build path from highways in city
// - use instanced drawing
// - move into the citydata structure
class Mover {
  public:

    void setup( const PolyLine2f &path )
    {
        mPath = path;
        // If it's closed and the front and back aren't the same point add it
        // to close it up.
        if ( path.isClosed() && glm::distance2( mPath.getPoints().front(), mPath.getPoints().back() ) > 0.001 ) {
            mPath.getPoints().push_back( mPath.getPoints().front() );
        }
        mAngle = 0;

        mPosition = vec2( mPath.getPoints()[0] );
        mVelocity = vec2( 0 );
        mMaxSpeed = 4;
        mMaxForce = 0.5;
        mMass = 5;

        mPrevPoint = mPath.size() > 1 ? mPath.size() - 1 : 0;
        mCurrPoint = 0;
        mNextPoint = mPath.size() > 1 ? 1 : 0;
        moveToNextSegment();
    }

    void moveToNextSegment()
    {
        mPrevPoint = mCurrPoint;
        mCurrPoint = mNextPoint;
        ++mNextPoint;

        if ( mNextPoint > mPath.size() - ( mPath.isClosed() ? 2 : 1 ) ) {
            mNextPoint = 0;
        }

        std::vector<vec2> &points = mPath.getPoints();

        vec2 p = glm::normalize( points[mCurrPoint] - points[mPrevPoint] );
        vec2 n = glm::normalize( points[mNextPoint] - points[mCurrPoint] );
        float diff = glm::angle( n, p );
        if ( diff <= M_PI_4 + 0.1 ) {
            mNextTurnSpeed = mMaxSpeed;
        } else if ( diff <= M_PI_2 + 0.1 ) {
            mNextTurnSpeed = mMaxSpeed / 2;
        } else {
            mNextTurnSpeed = mMaxSpeed / 3;
        }

        vec2 _center;
        findRadius( points[mPrevPoint], points[mCurrPoint], points[mNextPoint], mNextTurnRadius, _center );

        mSlowingDistance = calcSlowingDistance();
    }

    float calcSlowingDistance()
    {
        float max_accel = mMaxForce / mMass;
        float fudge_factor = 1.1;
        return fudge_factor * std::abs( mMaxSpeed * mMaxSpeed - mNextTurnSpeed * mNextTurnSpeed ) / ( 2.0 * max_accel );
    }

    vec2 truncate( vec2 v, float s )
    {
        if ( glm::length2( v ) >  s * s )
            return glm::normalize( v ) * s;
        else
            return v;
    }

    // All this is based of Reynold's steering behaviors
    // http://www.red3d.com/cwr/steer/gdc99/
    void update( double dt )
    {
        if ( mPath.size() < 2 ) return;

        vec2 target = mPath.getPoints()[mCurrPoint];

        // steer to arrive at next point
        vec2 target_offset = target - mPosition;
        float distance = glm::length( target_offset );
        float ramped_speed = mNextTurnSpeed * ( distance / mSlowingDistance );
        float clipped_speed = std::min( ramped_speed, mMaxSpeed );
        vec2 desired_velocity = (clipped_speed / distance) * target_offset;
        vec2 steering_direction = desired_velocity - mVelocity;
        vec2 steering_force = truncate( steering_direction, mMaxForce );

        vec2 acceleration = steering_force / mMass;
        mVelocity = truncate( mVelocity + acceleration, mMaxSpeed );
        mPosition = mPosition + mVelocity;

        if ( ramped_speed < mMaxSpeed )
            mColor = ColorA(1, 0, 0, 0.75);
//        else if ( accLen2 > 0.0 )
//            mColor = ColorA(0, 1, 0, 0.75);
        else
            mColor = ColorA(1, 1, 1, 0.75);

        // When we get close move to the next point
        if ( distance < mNextTurnRadius && glm::length2( mVelocity ) < ( mNextTurnSpeed * mNextTurnSpeed ) ) {
            moveToNextSegment();
        }
    }

    void findRadius( const vec2 &v1, const vec2 &v2, const vec2 &v3, float &r, vec2 &center )
    {
        // Put the point at the origin create two normal vectors heading
        // to the adjacent points.
        vec2 prev = glm::normalize( v1 - v2 );
        vec2 next = glm::normalize( v3 - v2 );

        // Find the radius of circle tangent to the vectors leaving this
        // point d distance away.
        /*
                         d
               prev <--+----+
                       |  θ/ \
                      r|  /   \
                       | /h    v
                       |/      next
                       +

            θ=inner angle/2
            tan(θ)=r/d => r=d*tan(θ)
            cos(θ)=d/h => h=d/cos(θ)
          
            I love having to relearn trig for stuff like this.
        */
        float diff = glm::angle( prev, next ) / 2.0;
        r = mTurnDistance * std::tan( diff );

        // If we want to display the circle we'll need to locate the
        // center which is on the hypotenuse.
        vec2 middle = ( prev + next ) / vec2( 2 );
        float middleAngle = std::atan2( middle.y, middle.x );
        float h = mTurnDistance / std::cos( diff );
        center = v2 + vec2( std::cos( middleAngle ), std::sin( middleAngle ) ) * h;
    }

    void draw()
    {
        {
            gl::ScopedColor color( mColor );
            gl::ScopedModelMatrix matrix;
            gl::translate( getPosition() );
            gl::rotate( getAngle() );
            gl::drawVector( vec3( 0, 0, 0 ), vec3( 10, 0, 0 ), 20, 10);
        }
        gl::draw( mPath );

        // Text debugging info
        {
            gl::ScopedMatrices mat;
            gl::setMatricesWindow( cinder::app::getWindowSize() );
            boost::format formatter( "%07.5f" );
            gl::drawString( "Speed: " + (formatter % glm::length(mVelocity)).str(), vec2( 10, 10 ) );
            gl::drawString( "Turn:  " + (formatter % mNextTurnSpeed).str(), vec2( 10, 25 ) );
            gl::drawString( "Max:   " + (formatter % mMaxSpeed).str(), vec2( 10, 40 ) );
            gl::drawString( "Dist:  " + (formatter % mSlowingDistance).str(), vec2( 10, 55 ) );
        }
    }

    vec2 getPosition()
    {
        return mPosition;
    }

    float getAngle()
    {
        if ( glm::length2( mVelocity ) > 0.001 ) {
            mAngle = std::atan2( mVelocity.y, mVelocity.x );
        }
        return mAngle;
    }

    const float G = 9.8;
    const float MU = 0.1;
    const float mTurnDistance = 10;

    float mMaxSpeed;
    float mMaxForce;

    PolyLine2f mPath;
    size_t mPrevPoint;
    size_t mCurrPoint;
    size_t mNextPoint;

    ColorA mColor;
    vec2 mPosition;
    vec2 mVelocity;
    float mMass;
    float mAngle;
    float mNextTurnRadius;
    float mNextTurnSpeed;
    float mSlowingDistance;
};

Mover mover;


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
