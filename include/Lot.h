//
//  Lot.h
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#ifndef __Cityscape__Lot__
#define __Cityscape__Lot__

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

    void buildInCenter();
    void buildFullLot();

    void layout( const Options &options );
    void draw( const Options &options );

    FlatShape mShape;
    ci::ColorA mColor;
    BuildingRef mBuildingRef;
};

#endif /* defined(__Cityscape__Lot__) */
