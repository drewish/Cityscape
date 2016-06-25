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


#include <glm/gtx/vector_angle.hpp>
// TODO:
// - slow down at the end of a segment
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
        mLengths = distanceBetweenPointsIn( mPath );
        mAngles = anglesBetweenPointsIn( mPath );
        mSegment = 0;
        mTime = 0;
        mSpeed = 0;
        mDistance = 0;

        mTurnSpeeds.clear();
        mTurnSpeeds.insert( mTurnSpeeds.begin(), mPath.size(), 0 );
        size_t count = mPath.size();
        if ( count > 2 ) {
            std::vector<vec2> &points = mPath.getPoints();
/* 
path (count == 5):
0 1 2 3 4

open (start = 1, last = count-2)
0: 0
1: 0-1-2
2: 1-2-3
3: 2-3-4
4: 0

closed (start = 0, last = count-1)
0: 4-0-1
1: 0-1-2
2: 1-2-3
3: 2-3-4
4: 3-4-0
*/
            size_t first;
            size_t last;
            if ( mPath.isClosed() ) {
                first = 0;
                last = count - 1;
            }
            else {
                first = 1;
                last = count - 2;
            }

            for ( int64_t i = first; i <= last; ++i ) {
                size_t prev = ( i == 0 )         ? last - 1 : ( i - 1 );
                size_t next = ( i == count - 1 ) ? 1        : ( i + 1 );
                float r;
                vec2 center;
                if ( glm::distance2( points[prev], points[i] ) < 0.001 ) {
                    r = 1000000;
                } else if ( glm::distance2( points[i], points[next] ) < 0.001 ) {
                    r = 1000000;
                } else {
                    findRadius( points[prev], points[i], points[next], r, center );
                }
                // Determine maximum speed in an unbanked turn for radius.
                // http://www.batesville.k12.in.us/Physics/PhyNet/Mechanics/Circular%20Motion/an_unbanked_turn.htm
                float mu = 0.1;
                float g = 9.8;
                float v = std::sqrt( mu * r * g );
                mTurnSpeeds[i] = std::min( v, mSpeedLimit );
            }
        }

    }

    void update( double elapsed )
    {
        mTime += elapsed;

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

        if ( distanceToTurn < 100 ) {
            // slowing
            float Vf = mTurnSpeeds[mSegment];
            float Vi = mSpeed;
            float d = distanceToTurn + 1;
            // http://stackoverflow.com/a/1088194/203673
            mAcceleration = (Vf*Vf - Vi*Vi)/(2 * d);
        } else if ( mSpeed < mSpeedLimit ) {
            // speeding up
            mAcceleration = 0.01;
        } else {
            // coasting
            mAcceleration = 0;
        }
        mSpeed += mAcceleration;

        mDistance += mSpeed;
        if ( mDistance >= mLengths[mSegment] ) {
            mDistance = mDistance - mLengths[mSegment];
            ++mSegment;
            if ( mSegment > mLengths.size() - 1 ) {
                mSegment = 0;
            }
        }
    }

    void findRadius( const vec2 &v1, const vec2 &v2, const vec2 &v3, float &r, vec2 &center ) {
        // Put the point at the origin create two normal vectors heading
        // to the adjacent points.
        vec2 prev = glm::normalize( v1 - v2 );
        vec2 next = glm::normalize( v3 - v2 );

        // Find the radius of circle tangent to the vectors leaving this
        // point d distance away.
        float d = 5;
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
        r = d * std::tan( diff );

        // If we want to display the circle we'll need to locate the
        // center which is on the hypotenuse.
        vec2 middle = ( prev + next ) / vec2( 2 );
        float middleAngle = std::atan2( middle.y, middle.x );
        float h = d / std::cos( diff );
        center = v2 + vec2( std::cos( middleAngle ), std::sin( middleAngle ) ) * h;
    }

    void draw()
    {
//        size_t count = mPath.size();
//        if ( count > 2 ) {
//            std::vector<vec2> &points = mPath.getPoints();
//
//            size_t first;
//            size_t last;
//            if ( mPath.isClosed() ) {
//                first = 0;
//                last = count - 1;
//            }
//            else {
//                first = 1;
//                last = count - 2;
//            }
//
//            for ( int64_t i = first; i < last; ++i ) {
//                size_t prev = ( i == 0 )         ? last : ( i - 1 );
//                size_t next = ( i == count - 1 ) ? 0    : ( i + 1 );
//                float r;
//                vec2 center;
//                findRadius( points[prev], points[i], points[next], r, center );
//                gl::drawStrokedCircle( center, r, 32 );
//            }
//        }

        gl::draw( mPath );
        {
            gl::ScopedModelMatrix matrix;
            gl::translate( getPosition() );
            gl::rotate( getAngle() );
            gl::drawVector( vec3( 0, 0, 0 ), vec3( -10, 0, 0 ), 20, 10);
//            gl::drawColorCube( vec3( 0 ), vec3( 20 ) ) ;
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
    std::vector<float> mTurnSpeeds;
    size_t mSegment;
    float mTime;
    float mDistance;
    float mSpeed;
    float mAcceleration;
    float mAngle;
    float mSpeedLimit = 10;
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
