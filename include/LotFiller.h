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
    // in Lots
    // out Buildings, Trees
    void fillLots( CityModel &city );

    class LotDeveloper {
      public:
        virtual ~LotDeveloper() {};

        virtual const std::string name() const { return "Un-developer"; }
        virtual bool isValidFor( LotRef &lot ) const { return false; }
        virtual void buildIn( LotRef &lot ) const {};
    };
}