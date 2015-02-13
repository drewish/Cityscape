//
//  Road.h
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#ifndef __Cityscape__Road__
#define __Cityscape__Road__

using namespace ci;

class Road {
  public:
    Road( const Vec2f a, const Vec2f b, uint width = 10 )
    : pointA(a), pointB(b), width(width)
    {
        Vec2f normal = Vec2f(b.y - a.y, -(b.x - a.x)).normalized();
        Vec2f offset = normal * width / 2;
        outline.push_back(a + offset);
        outline.push_back(a - offset);
        outline.push_back(b - offset);
        outline.push_back(b + offset);
        outline.setClosed();
    };
    const Vec2f pointA, pointB;
    PolyLine2f outline;
    const unsigned int width;
};

#endif /* defined(__Cityscape__Road__) */
