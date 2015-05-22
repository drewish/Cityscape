//
//  Road.h
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#ifndef __Cityscape__Road__
#define __Cityscape__Road__

#include "Options.h"

class Road {
  public:
    Road( const ci::Vec2f a, const ci::Vec2f b, uint width = 10 )
    : pointA(a), pointB(b), width(width)
    {
        ci::Vec2f normal = ci::Vec2f(b.y - a.y, -(b.x - a.x)).normalized();
        ci::Vec2f offset = normal * width / 2;
        outline.push_back(a + offset);
        outline.push_back(a - offset);
        outline.push_back(b - offset);
        outline.push_back(b + offset);
        outline.setClosed();
    };

    void setup();
    void draw( const Options &options );

    const ci::Vec2f pointA, pointB;
    ci::PolyLine2f outline;
    const unsigned int width;
};

#endif /* defined(__Cityscape__Road__) */
