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
#include "CgalStraightSkeleton.h"

class Block;
typedef std::shared_ptr<Block> BlockRef;

class Block {
  public:

    static BlockRef create( const FlatShape &fs, const ci::ColorA &c )
    {
        return BlockRef( new Block( fs, c ) );
    }

    Block( const FlatShape &fs, const ci::ColorA &c )
        : mShape( fs ), mColor( c )
    {}
    Block( const Block &src )
        : mShape( src.mShape ), mLots( src.mLots ), mColor( src.mColor )
    {}
    void layout( const Options &options );
    void subdivideNotReally( const Options &options );
    void subdivideSkeleton( const Options &options );

    LotRef buildLot( const ci::PolyLine2f &lotOutline, const ci::ColorA &color, const LotOptions::BuildingPlacement placement );

    Arrangement_2 arrangementSubdividing( const FlatShape &shape, const int16_t lotWidth );

    void placeBuildings();

    friend class BlockMode;

    // TODO: move to CityModel
    // Use shared pointers to lots so we can call virtual methods.
    std::vector<LotRef> mLots;
    FlatShape mShape;
    ci::ColorA mColor;

  private:
    Arrangement_2 mArr;
    SsPtr mSkel;
    float mDividerAngle;
};
