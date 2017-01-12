//
//  LotFiller.h
//  Cityscape
//
//  Created by Andrew Morton on 3/7/16.
//
//

#pragma once

#include "CityData.h"

namespace Cityscape {
    // Uses LotDevelopers to fill lots
    //
    // in Lots
    // out Scenery (Buildings & Plants)
    void fillLots( CityModel &city );
}
