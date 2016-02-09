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
    void subdivideNotReally( const Options &options );
    void subdivideSkeleton( const Options &options );

    LotRef buildLot( const ci::PolyLine2f &lotOutline, const ci::ColorA &color, const LotOptions::BuildingPlacement placement );

    Arrangement_2 arrangementSubdividing( const FlatShape &shape, const int16_t lotWidth );

    void placeBuildings();

    FlatShape mShape;
    // Use shared pointers to lots so we can call virtual methods.
    std::vector<LotRef> mLots;
    Arrangement_2 mArr;
};
