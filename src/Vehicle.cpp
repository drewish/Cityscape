//
//  Vehicle.cpp
//  Cityscape
//
//  Created by andrew morton on 7/3/16.
//
//

#include "Vehicle.h"

using namespace ci;

#include <boost/format.hpp>
#include <glm/gtx/vector_angle.hpp>
// TODO:
// - interpolate the rotation angle turning
// - only closed polylines should loop
// - build path from highways in city
// - use instanced drawing
// - move into the citydata structure
// - create a car model

#include "Resources.h"
#include "cinder/ObjLoader.h"

void Vehicle::setup( const PolyLine2f &path )
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


    // Load the plane model in so we have something to draw
    ObjLoader loader( app::loadResource( RES_PLANE_OBJ ) );
    TriMeshRef mesh = TriMesh::create( loader );
    if( ! loader.getAvailableAttribs().count( geom::NORMAL ) )
        mesh->recalculateNormals();
    mBatch = gl::Batch::create( *mesh, gl::getStockShader( gl::ShaderDef().color() ) );
}

void findRadius( float turnDistance, const vec2 &v1, const vec2 &v2, const vec2 &v3, float &r, vec2 &center )
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
    r = turnDistance * std::tan( diff );

    // If we want to display the circle we'll need to locate the
    // center which is on the hypotenuse.
    vec2 middle = ( prev + next ) / vec2( 2 );
    float middleAngle = std::atan2( middle.y, middle.x );
    float h = turnDistance / std::cos( diff );
    center = v2 + vec2( std::cos( middleAngle ), std::sin( middleAngle ) ) * h;
}

void Vehicle::moveToNextSegment()
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
    // TODO: Come up with a more gradual way of handling this
    if ( diff <= M_PI_4 + 0.1 ) {
        mNextTurnSpeed = mMaxSpeed;
    } else if ( diff <= M_PI_2 + 0.1 ) {
        mNextTurnSpeed = mMaxSpeed / 2;
    } else {
        mNextTurnSpeed = mMaxSpeed / 3;
    }

    vec2 _center;
    findRadius( mTurnDistance, points[mPrevPoint], points[mCurrPoint], points[mNextPoint], mNextTurnRadius, _center );

    mSlowingDistance = calcSlowingDistance();
}

float Vehicle::calcSlowingDistance()
{
    float max_accel = mMaxForce / mMass;
    float fudge_factor = 1.1;
    return fudge_factor * std::abs( mMaxSpeed * mMaxSpeed - mNextTurnSpeed * mNextTurnSpeed ) / ( 2.0 * max_accel );
}

vec2 Vehicle::truncate( vec2 v, float s )
{
    if ( glm::length2( v ) >  s * s )
        return glm::normalize( v ) * s;
    return v;
}

// All this is based of Reynold's steering behaviors
// http://www.red3d.com/cwr/steer/gdc99/
void Vehicle::update( double dt )
{
    if ( mPath.size() < 2 ) return;

    vec2 target = mPath.getPoints()[mCurrPoint];

    // steer to arrive at next point
    vec2 target_offset = target - mPosition;
    float distance = glm::length( target_offset );
    float ramped_speed = mNextTurnSpeed * ( distance / mSlowingDistance );
    float clipped_speed = std::min( ramped_speed, mMaxSpeed );
    vec2 desired_velocity = ( clipped_speed / distance ) * target_offset;
    vec2 steering_direction = desired_velocity - mVelocity;
    vec2 steering_force = truncate( steering_direction, mMaxForce );

    vec2 acceleration = steering_force / mMass;
    mVelocity = truncate( mVelocity + acceleration, mMaxSpeed );
    mPosition = mPosition + mVelocity;

    if ( ramped_speed < mMaxSpeed )
        mColor = ColorA(1, 0, 0, 0.75);
//    else if ( accelerating? )
//        mColor = ColorA(0, 1, 0, 0.75);
    else
        mColor = ColorA(1, 1, 1, 0.75);

    // When we get close move to the next point
    if ( distance < mNextTurnRadius && glm::length2( mVelocity ) < ( mNextTurnSpeed * mNextTurnSpeed ) ) {
        moveToNextSegment();
    }
}

void Vehicle::draw() const
{
    {
        gl::ScopedColor color( mColor );
        gl::ScopedModelMatrix matrix;
        gl::translate( getPosition() );
        gl::rotate( getAngle(), vec3( 0, 0, 1 ) );
        //  gl::drawVector( vec3( 0, 0, 0 ), vec3( 10, 0, 0 ), 20, 10);
        // For what ever reason we have to do some rotating to get the model
        // facing the right direction.
        gl::rotate( 1.571, vec3( 1, 0, 0 ) );
        gl::rotate( 1.571, vec3( 0, 1, 0 ) );
        gl::scale( vec3( 10 ) );
        mBatch->draw();
    }

    if ( false ) return;

    gl::draw( mPath );

    // Slowing boundary
    {
        gl::ScopedColor color( 1, 0, 0, 0.125 );
        const std::vector<vec2> &points = mPath.getPoints();
        gl::drawStrokedCircle( points[mCurrPoint], 2 * mSlowingDistance );
    }

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
