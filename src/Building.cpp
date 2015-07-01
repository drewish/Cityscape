//
//  Building.cpp
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#include "Building.h"

using namespace ci;

void Building::layout()
{
//    mMeshRef = makeMesh(mRoof, mOutline, mFloors);
}

void Building::draw( const Options &options ) const
{
    if ( options.drawBuildings && mPlan.wallMeshRef() ) {

        gl::ScopedGlslProg glslScope( options.buildingShader );

        gl::enable( GL_CULL_FACE );
        glCullFace( GL_BACK );

        // Walls
        gl::pushModelView();
        gl::scale( 1.0, 1.0, 1.0 * mFloors );

        gl::draw( mPlan.wallMeshRef() );
//        gl::enableWireframe();
//        gl::draw( mPlan.wallMeshRef() );
//        gl::disableWireframe();

        gl::popModelView();


        // Roof
        gl::pushModelView();
        gl::translate( 0.0, 0.0, mFloors * mPlan.mFloorHeight );

        gl::draw( mPlan.roofMeshRef() );
//        gl::enableWireframe();
//        gl::draw( mPlan.roofMeshRef() );
//        gl::disableWireframe();

        gl::popModelView();

        // * * *

        gl::disable( GL_CULL_FACE );
    }
}

