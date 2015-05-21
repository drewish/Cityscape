//
//  Block.h
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#ifndef __Cityscape__Block__
#define __Cityscape__Block__

#include "cinder/TriMesh.h"
#include "cinder/Triangulate.h"

#include "FlatShape.h"

class Lot;

class Block {
public:

    Block( const Block &src )
        : mShape(src.mShape), mId(src.mId), mLots(src.mLots)
    {}
    Block( const unsigned int bid, const ci::PolyLine2f outline, const FlatShape::PolyLine2fs holes = {} )
        : mShape(outline, holes), mId(bid)
    {}
    void setup();
    void draw();
    void subdivide();
    void placeBuildings();

    FlatShape mShape;
    unsigned int mId;
    std::vector<Lot> mLots;
};

#endif /* defined(__Cityscape__Block__) */
