//
//  RoadBuilder.hpp
//  Cityscape
//
//  Created by Andrew Morton on 3/5/16.
//
//

#pragma once

#include "CityData.h"

namespace Cityscape {
    // in Highways
    // out Districts and paved FlatShape
    void buildHighwaysAndDistricts( CityModel &city );

    // in Districts
    // out Streets, Blocks and paved FlatShape
    void buildStreetsAndBlocks( CityModel &city );
}
