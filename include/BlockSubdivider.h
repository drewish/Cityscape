//
//  BlockSubdivider.hpp
//  Cityscape
//
//  Created by Andrew Morton on 3/6/16.
//
//

#pragma once

#include "CityData.h"
#include "CgalArrangement.h"

namespace Cityscape {
    // in Blocks
    // out Lots
    void subdivideBlocks( CityModel &city );

    // Little hack to see the last arrangment we touched, useful for debugging.
    const Arrangement_2& lastArrangement();
}