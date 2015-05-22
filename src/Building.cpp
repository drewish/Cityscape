//
//  Building.cpp
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#include "Building.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace std;

void Building::setup()
{
    mFloors = Rand::randInt(5);

    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();

    uint totalVertices = 2 * mOutline.size();
    uint totalIndicies = 2 * (mOutline.size() + 1);
    mMesh = gl::VboMesh( totalVertices, totalIndicies, layout, GL_QUAD_STRIP );

    vector<Vec3f> verts;
    vector<uint32_t> indices;

    int index = 0;
    for (auto i = mOutline.begin(); i != mOutline.end(); ++i) {
        indices.push_back(index++);
        indices.push_back(index++);
        verts.push_back(Vec3f(i->x, i->y, 0));
        verts.push_back(Vec3f(i->x, i->y, mFloors * 10));
    }
    indices.push_back(0);
    indices.push_back(1);

    mMesh.bufferIndices( indices );
    mMesh.bufferPositions( verts );
}

void Building::draw( const Options &options )
{
    if ( options.drawBuildings ) {
        gl::color( ColorA( 0.5f, 0.2f, 0.4f, 0.8f ) );
        gl::draw( mMesh );
    }
}