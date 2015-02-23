//
//  Block.h
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#ifndef __Cityscape__Block__
#define __Cityscape__Block__

using namespace ci;

class Lot;

class Block {
public:
    // Outline's coords should be centered around the origin so we can transform
    // it to fit on the lot.
    Block( const PolyLine2f outline ) : outline(outline) { };

    void draw();
    void subdivide();
    
    PolyLine2f outline;
    std::vector<Lot> lots;
};

#endif /* defined(__Cityscape__Block__) */
