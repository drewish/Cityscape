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

class Lot {
  public:
    Lot( const Lot &src ) : mShape(src.mShape), mId(src.mId), mColor(src.mColor), building(src.building), buildingPosition(src.buildingPosition) { };
    Lot( const unsigned int lid, const ci::PolyLine2f outline ) : mShape(outline), mId(lid) { };
    
    void place( const Building b );
    void setup();
    void draw();

    unsigned int mId;
    FlatShape mShape;
    ci::Color mColor;
    Building building;
    ci::Vec2f buildingPosition;
};

#endif /* defined(__Cityscape__Lot__) */
