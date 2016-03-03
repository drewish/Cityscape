//
//  Building.cpp
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#include "Building.h"

using namespace ci;

void Building::layout( const Options &options ) {}

void Building::draw( const Options &options ) const
{
    if ( options.drawBuildings ) {
        float hue = 0.1;
        options.buildingShader->uniform( "darkColor",   Color( CM_HSV, hue, 0.60, 0.25 ) );
        options.buildingShader->uniform( "mediumColor", Color( CM_HSV, hue, 0.55, 0.66 ) );
        options.buildingShader->uniform( "lightColor",  Color( CM_HSV, hue, 0.15, 1.00 ) );

        gl::ScopedGlslProg glslScope( options.buildingShader );
        gl::ScopedFaceCulling faceCullScope( true, GL_BACK );
//        gl::enableWireframe();

        gl::pushModelMatrix();
        gl::translate(mPosition);
        gl::rotate(mRotation);

        // Walls
        gl::pushModelMatrix();
        gl::scale( 1.0, 1.0, 1.0 * mFloors );
        gl::draw( mPlan.wallMeshRef() );
        gl::popModelMatrix();

        // Roof
        gl::pushModelMatrix();
        gl::translate( 0.0, 0.0, mFloors * mPlan.floorHeight() );
        gl::draw( mPlan.roofMeshRef() );
        gl::popModelMatrix();

//        gl::disableWireframe();
        gl::popModelMatrix();
    }
}

