//
//  Building.h
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#ifndef __Cityscape__Building__
#define __Cityscape__Building__

using namespace ci;

class Building {
  public:
    // Default to a 10x10 square
    Building() : outline(PolyLine2f( { Vec2f(-5, -5), Vec2f(-5, 5), Vec2f(5, 5), Vec2f(5, -5) } )) {};
    Building( const PolyLine2f outline ) : outline(outline) { };
    Building( const Building &src ) : outline(src.outline) { };
    PolyLine2f outline;
};

#endif /* defined(__Cityscape__Building__) */
