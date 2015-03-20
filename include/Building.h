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

class Building {
  public:
    // Outline's coords should be centered around the origin so we can transform
    // it to fit on the lot.
    // Default to a 10x10 square
    Building() : mOutline(ci::PolyLine2f( { ci::Vec2f(-10, -10), ci::Vec2f(-10, 10), ci::Vec2f(10, 10), ci::Vec2f(10, -10) } )) {};
    Building( const ci::PolyLine2f outline ) : mOutline(outline) { };
    Building( const Building &src ) : mOutline(src.mOutline), mFloors(src.mFloors) { };

    void setup();
    void draw();
    
    ci::PolyLine2f mOutline;
    ci::gl::VboMesh mMesh;
    unsigned int mFloors = 1;
};

#endif /* defined(__Cityscape__Building__) */
