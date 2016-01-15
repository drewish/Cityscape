//
//  Block.h
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#pragma once

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
    void layout( const Options &options );
    void draw( const Options &options );
    void subdivideNotReally();
    void subdivideSkeleton();
    void subdivideForReal();
    void placeBuildings();

    FlatShape mShape;
    std::vector<Lot> mLots;
};
