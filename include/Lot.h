//
//  Lot.h
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#pragma once

#include "FlatShape.h"
#include "Building.h"
#include "Options.h"

struct Tree {
    Tree( const ci::vec3 &p, float d ) : position( p ), diameter( d ) { };

    ci::vec3 position;
    float diameter;
};

class Lot;
typedef std::shared_ptr<Lot> LotRef;

class Lot {
  public:
    Lot( const FlatShape &fs, const ci::ColorA &c, const std::vector<Tree> &trees = {} )
        : mShape( fs ), mColor( c ), mTrees( trees) {};
    virtual ~Lot() {};

    virtual void layout( const Options &options ) {};
    // Drawing is separated into two steps so we can get all the surfaces in
    // place before drawing the structures atop them.
    virtual void drawGround( const Options &options ) const;
    virtual void drawStructures( const Options &options ) const {};

// TODO: move to CityModel
//  protected:
    FlatShape mShape;
    ci::ColorA mColor;
    std::vector<Tree> mTrees;
};

class EmptyLot : public Lot {
    using Lot::Lot;
};

class FilledLot : public Lot  {
public:
    using Lot::Lot;
    FilledLot( const FilledLot &src )
        : Lot( src ), mBuildingRef( src.mBuildingRef ) {};

    virtual void layout( const Options &options ) override;
    virtual void drawStructures( const Options &options ) const override
    {
        if ( mBuildingRef ) mBuildingRef->draw( options );
    }

protected:
    BuildingRef mBuildingRef;
};

class SingleBuildingLot : public Lot  {
  public:
    using Lot::Lot;
    SingleBuildingLot( const SingleBuildingLot &src )
        : Lot( src ), mBuildingRef( src.mBuildingRef ) {};

    virtual void layout( const Options &options ) override;
    virtual void drawStructures( const Options &options ) const override
    {
        if ( mBuildingRef ) mBuildingRef->draw( options );
    }

  protected:
    BuildingRef mBuildingRef;
};

class ParkLot : public Lot {
  public:
    ParkLot( const FlatShape &fs, const float treeCover = 0.25 )
        : Lot( fs, ci::ColorA8u( 0xA1, 0xC9, 0x76 ) ), mTreeCoverRatio( treeCover ) {};
    ParkLot( const ParkLot &src )
        : Lot( src.mShape, src.mColor ), mTreeCoverRatio( src.mTreeCoverRatio )
    {};

    virtual void layout( const Options &options ) override;

  protected:

    cinder::gl::BatchRef mBatch;
    float mTreeCoverRatio;
};