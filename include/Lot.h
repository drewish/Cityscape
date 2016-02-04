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

class Lot;
typedef std::shared_ptr<Lot> LotRef;

class Lot {
  public:
    Lot( const Lot &src ) : mShape( src.mShape ), mColor( src.mColor ) {};
    Lot( const FlatShape &fs, const ci::Color &c ) : mShape( fs ), mColor( c ) {};
    virtual ~Lot() {};

    virtual void layout( const Options &options ) {};
    // Drawing is separated into two steps so we can get all the surfaces in
    // place before drawing the structures atop them.
    virtual void drawGround( const Options &options ) const;
    virtual void drawStructures( const Options &options ) const {
        //
    };

  protected:
    FlatShape mShape;
    ci::ColorA mColor;
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
    using Lot::Lot;

    virtual void layout( const Options &options ) override;
    virtual void drawStructures( const Options &options ) const override;

  protected:
    cinder::gl::BatchRef mBatch;
};