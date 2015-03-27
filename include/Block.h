//
//  Block.h
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#ifndef __Cityscape__Block__
#define __Cityscape__Block__

#include "CinderCGAL.h"

class Lot;

class Block {
public:
    Block( const ci::PolyLine2f outline ) : outline(outline) { };

    void setup();
    void draw();
    void subdivide();
    void placeBuildings();
    
    ci::PolyLine2f outline;
    std::vector<Lot> lots;
    SsPtr mSkel;
};

#endif /* defined(__Cityscape__Block__) */
