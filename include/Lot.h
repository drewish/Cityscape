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
        mBuildingRef(src.mBuildingRef),
        buildingPosition(src.buildingPosition),
        buildingRotation(src.buildingRotation)
    { };
    Lot( const FlatShape &fs, const ci::Color &c ) : mShape(fs), mColor(c) { };

    void buildInCenter();
    void buildFullLot();

    void layout();
    void draw( const Options &options );
    void drawBuilding( const Options &options );

    FlatShape mShape;
    ci::ColorA mColor;
    BuildingRef mBuildingRef;
    ci::vec2 buildingPosition;
    float buildingRotation = 0;
};

#endif /* defined(__Cityscape__Lot__) */
