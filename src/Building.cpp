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
        options.buildingShader->bind();
//        options.buildingShader->uniform( "zoom", 0.5f );

        gl::enable( GL_CULL_FACE );
        glCullFace( GL_BACK );

        gl::pushModelView();
        gl::scale( 1.0, 1.0, 1.0 * mFloors );
        gl::draw( mPlan.wallMeshRef() );
        gl::popModelView();

        gl::pushModelView();
        gl::translate( 0.0, 0.0, mFloors * mPlan.mFloorHeight );
        gl::draw( mPlan.roofMeshRef() );
        gl::popModelView();

        options.buildingShader->unbind();

//        gl::enableWireframe();
//        gl::draw( mPlan.meshRef() );
//        gl::disableWireframe();

        gl::disable( GL_CULL_FACE );
    }
}

