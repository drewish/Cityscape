//
//  Block.h
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#pragma once

// TODO: see if we can use forward declarations to avoid including all this
#include "FlatShape.h"
#include "Lot.h"
#include "Options.h"
#include "CgalArrangement.h"

class Block;
typedef std::shared_ptr<Block> BlockRef;

class Block {
public:

    static BlockRef create( const FlatShape &fs )
    {
        return BlockRef( new Block( fs ) );
    }

    Block( const FlatShape &fs, const ci::Color &c = ci::Color::white() )
        : mShape(fs)
    {}
    Block( const Block &src )
        : mShape(src.mShape), mLots(src.mLots)
    {}
    void layout( const Options &options );
    void draw( const Options &options ) const;
    void subdivideNotReally();
    void subdivideSkeleton( int16_t lotWidth );

    Arrangement_2 arrangementSubdividing( const FlatShape &shape, const int16_t lotWidth );

    void placeBuildings();

    FlatShape mShape;
    std::vector<Lot> mLots;
};
