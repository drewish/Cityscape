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
#include "Options.h"

class Lot;

class Block {
public:

    Block( const Block &src )
        : mShape(src.mShape), mId(src.mId), mLots(src.mLots)
    {}
    Block( const unsigned int bid, const FlatShape &fs )
    : mShape(fs), mId(bid)
    {}
    void layout();
    void draw( const Options &options );
    void subdivide();
    void placeBuildings();

    FlatShape mShape;
    unsigned int mId;
    std::vector<Lot> mLots;
};

#endif /* defined(__Cityscape__Block__) */
