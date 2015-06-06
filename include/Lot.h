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
        mId(src.mId),
        mColor(src.mColor),
        mBuilding(src.mBuilding),
        buildingPosition(src.buildingPosition),
        buildingRotation(src.buildingRotation)
    { };
    Lot( const uint32_t lid, const FlatShape &fs, const ci::Color &c ) : mShape(fs), mId(lid), mColor(c) { };

    void buildInCenter();
    void buildFullLot();

    void layout();
    void draw( const Options &options );

    uint32_t mId;
    FlatShape mShape;
    ci::ColorA mColor;
    Building mBuilding;
    ci::Vec2f buildingPosition;
    float buildingRotation = 0;
};

#endif /* defined(__Cityscape__Lot__) */
