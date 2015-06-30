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
    Road( const ci::vec2 a, const ci::vec2 b, uint width = 10 )
    : pointA(a), pointB(b), width(width)
    {
        ci::vec2 normal = glm::normalize( ci::vec2( b.y - a.y, - ( b.x - a.x )) );
        ci::vec2 offset = ci::vec2(width / 2.0) * normal;
        outline.push_back(b + offset);
        outline.push_back(b - offset);
        outline.push_back(a - offset);
        outline.push_back(a + offset);
        outline.setClosed();
    };

    void layout();
    void draw( const Options &options );
    const ci::Rectf bounds();

    const ci::vec2 pointA, pointB;
    ci::PolyLine2f outline;
    const unsigned int width;
};

#endif /* defined(__Cityscape__Road__) */
