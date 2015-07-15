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
        gl::ScopedGlslProg glslScope( options.buildingShader );
        gl::ScopedFaceCulling faceCullScope( true, GL_BACK );
        gl::ScopedLineWidth(5);
//        gl::enableWireframe();

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
    }
}

