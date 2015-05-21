//
//  Lot.h
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#ifndef __Cityscape__Lot__
#define __Cityscape__Lot__

#include "Building.h"

class Lot {
  public:
    Lot( const Lot &src ) : mId(src.mId), outline(src.outline), mColor(src.mColor), building(src.building), buildingPosition(src.buildingPosition) { };
    Lot( const unsigned int lid, const ci::PolyLine2f outline ) : mId(lid), outline(outline) { };
    
    void place( const Building b );
    void setup();
    void draw();

    unsigned int mId;
    ci::PolyLine2f outline;
    ci::Color mColor;
    Building building;
    ci::Vec2f buildingPosition;
};

#endif /* defined(__Cityscape__Lot__) */
