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
    if ( options.drawBuildings && mPlan.meshRef() ) {
        options.buildingShader->bind();
//        options.buildingShader->uniform( "zoom", 0.5f );

        gl::enable( GL_CULL_FACE );
        glCullFace( GL_BACK );

        gl::draw( mPlan.meshRef() );

        options.buildingShader->unbind();

//        gl::enableWireframe();
//        gl::draw( mMesh );
//        gl::disableWireframe();

        gl::disable( GL_CULL_FACE );
    }
}

