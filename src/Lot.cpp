//
//  Lot.cpp
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#include "Lot.h"

using namespace ci;

void Lot::layout()
{
    // TODO: just placing it in the center for now. would be good to take
    // the street into consideration.
    buildingPosition = mShape.centroid();

    // Vary the floors based on the space... TODO should check for available
    // space
    Polygon_2 p = mShape.polygon();
    float area = CGAL::polygon_area_2( p.vertices_begin(), p.vertices_end(), K() ).floatValue();
    if ( area < 200 ) {
        mBuilding.mFloors = 0;
    }
    else {
        mBuilding.mFloors = sqrt(area) / 20;//Rand::randInt(5);
    }

    mBuilding.layout();
}

void Lot::draw( const Options &options )
{
    if ( options.drawLots ) {
        gl::lineWidth( 1 );
        gl::color( ColorA( mColor, 0.4 ) );
        gl::draw( mShape.mesh() );
    }

    gl::pushModelView();
    gl::translate(buildingPosition);
    mBuilding.draw( options );
    gl::popModelView();
}
