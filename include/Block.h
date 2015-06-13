//
//  Block.h
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#ifndef __Cityscape__Block__
#define __Cityscape__Block__


#include "FlatShape.h"
#include "Lot.h"
#include "Options.h"


class Block {
public:

    Block( const Block &src )
        : mShape(src.mShape), mLots(src.mLots)
    {}
    Block( const FlatShape &fs, const ci::Color &c )
    : mShape(fs)
    {}
    void layout();
    void draw( const Options &options );
    void subdivideNotReally();
    void subdivideSkeleton();
    void placeBuildings();

    FlatShape mShape;
    std::vector<Lot> mLots;
};

#endif /* defined(__Cityscape__Block__) */
