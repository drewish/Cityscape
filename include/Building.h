//
//  Building.h
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#ifndef __Cityscape__Building__
#define __Cityscape__Building__

#include "cinder/gl/Vbo.h"
#include "Options.h"

class Building {
  public:
    // Outline's coords should be centered around the origin so we can transform
    // it to fit on the lot.
    // Default to a 10x10 square
    Building() : mOutline(ci::PolyLine2f( { ci::Vec2f(10, -10), ci::Vec2f(10, 10), ci::Vec2f(-10, 10), ci::Vec2f(-10, -10) } )) {};
    Building( const ci::PolyLine2f outline ) : mOutline(outline) { };
    Building( const Building &src ) : mColor(src.mColor), mOutline(src.mOutline), mFloors(src.mFloors) { };

    void layout();
    void draw( const Options &options );

    ci::ColorA mColor;
    ci::PolyLine2f mOutline;
    uint32_t mFloors = 1;
    ci::gl::VboMesh mMesh;
    float mArea = 0;

};

#endif /* defined(__Cityscape__Building__) */
