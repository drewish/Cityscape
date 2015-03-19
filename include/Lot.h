//
//  Lot.h
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#ifndef __Cityscape__Lot__
#define __Cityscape__Lot__

using namespace ci;

#include "Building.h"

class Lot {
  public:
    Lot( const Lot &src ) : outline(src.outline), mColor(src.mColor), building(src.building) { };
    Lot( const PolyLine2f outline ) : outline(outline) { };
    
    void place( const Building b );
    void draw();
    
    PolyLine2f outline;
    Color mColor;
    Building building;
};

#endif /* defined(__Cityscape__Lot__) */
