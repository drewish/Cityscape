//
//  LotFiller.cpp
//  Cityscape
//
//  Created by Andrew Morton on 3/7/16.
//
//

#include "LotFiller.h"
#include "FlatShape.h"
#include "cinder/Rand.h"

using namespace ci;

namespace Cityscape {

const LotDeveloperRef pickDeveloper( LotRef &lot, const ZoningPlanRef &zoning ) {
    std::vector<LotDeveloperRef> developerPool;

    // Figure out which developers are applicable then seed a pool...
    for ( auto &usage : zoning->lotUsages ) {
        // If there's no developer
        if ( !usage.developer || ( usage.developer && usage.developer->isValidFor( lot ) ) ) {
            developerPool.insert( developerPool.end(), usage.ratio, usage.developer );
        }
    }

    if ( developerPool.size() ) {
        return developerPool.at( randInt( 0, developerPool.size() ) );
    }
    return nullptr;
}

void fillLots( CityModel &city )
{
    for ( const auto &district : city.districts ) {
        ZoningPlanRef zoning = district->zoningPlan;

        for ( const auto &block : district->blocks ) {
            for ( auto &lot : block->lots ) {
                LotDeveloperRef developer = pickDeveloper( lot, zoning );
                if ( developer ) developer->buildIn( lot );
            }
        }
    }
}

} // Cityscape namespace
