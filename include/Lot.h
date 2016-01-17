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

class Lot {
  public:
    Lot( const Lot &src ) :
        mShape(src.mShape),
        mColor(src.mColor),
        mBuildingRef(src.mBuildingRef)
    { };
    Lot( const FlatShape &fs, const ci::Color &c ) : mShape(fs), mColor(c) { };

    void buildInCenter( const Options &options );
    void buildFullLot( const Options &options );

    void layout( const Options &options );
    void draw( const Options &options ) const;

    FlatShape mShape;
    ci::ColorA mColor;
    BuildingRef mBuildingRef;
};
