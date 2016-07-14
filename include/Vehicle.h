//
//  Vehicle.hpp
//  Cityscape
//
//  Created by andrew morton on 7/3/16.
//
//

#pragma once

// TODO:
// - interpolate the rotation angle turning
// - only closed polylines should loop 
// - move this to its own files
// - draw a custom shape to indicate direction
// - build path from highways in city
// - use instanced drawing
// - move into the citydata structure
class Vehicle {
  public:

    void setup( const ci::PolyLine2f &path );
    void update( double dt );
    void draw() const;

    ci::vec2 getPosition() const
    {
        return mPosition;
    }

    float getAngle() const
    {
        if ( glm::length2( mVelocity ) > 0.001 ) {
            mAngle = std::atan2( mVelocity.y, mVelocity.x );
        }
        return mAngle;
    }

    void moveToNextSegment();
    float calcSlowingDistance();
    ci::vec2 truncate( ci::vec2 v, float s );

  protected:
    const float mTurnDistance = 10;

    float mMaxSpeed;
    float mMaxForce;

    ci::PolyLine2f mPath;
    size_t mPrevPoint;
    size_t mCurrPoint;
    size_t mNextPoint;

    ci::ColorA mColor;
    ci::vec2 mPosition;
    ci::vec2 mVelocity;
    float mMass;
    mutable float mAngle;
    float mNextTurnRadius;
    float mNextTurnSpeed;
    float mSlowingDistance;
};
