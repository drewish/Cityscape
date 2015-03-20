//
//  Building.cpp
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#include "Building.h"

using namespace ci;
using namespace std;

void Building::setup()
{
/*
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();

    uint totalVertices = 2 * mOutline.size();
    uint totalIndicies = 2 * mOutline.size();
    mMesh = gl::VboMesh::create( 4, 4, layout, GL_QUAD_STRIP );

    vector<Vec3f> verts;
    vector<uint32_t> indices;

    int index = 0;
    indices.push_back(index++);
    indices.push_back(index++);
    indices.push_back(index++);
    indices.push_back(index++);
    verts.push_back(Vec3f(0, 0, 0));
    verts.push_back(Vec3f(0, 0, 100));
    verts.push_back(Vec3f(100, 100, 0));
    verts.push_back(Vec3f(100, 100, 100));
//    for (auto i = mOutline.begin(); i != mOutline.end(); ++i) {
//        indices.push_back(index++);
//        indices.push_back(index++);
//        verts.push_back(Vec3f(i->x, i->y, 0));
//        verts.push_back(Vec3f(i->x, i->y, mFloors * 10));
//    }

//    mMesh->bufferIndices( indices );
    mMesh->bufferPositions( verts );
*/

/*
    // We can't use gl::drawCube since it uses triangle faces so we'll build
    // one using quads.
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();

    int vertCount = 24;
    int quadCount = 6;
    mMesh = gl::VboMesh(vertCount, quadCount * 4, layout, GL_QUADS);

    vector<uint32_t> indices;
    for (int i=0; i < vertCount; i++) {
        indices.push_back(i);
    }
    mMesh.bufferIndices(indices);

    vector<Vec3f> positions;
    positions.push_back( Vec3f( -0.5,  0.5, -0.5 ) );
    positions.push_back( Vec3f(  0.5,  0.5, -0.5 ) );
    positions.push_back( Vec3f(  0.5, -0.5, -0.5 ) );
    positions.push_back( Vec3f( -0.5, -0.5, -0.5 ) );

    positions.push_back( Vec3f(  0.5,  0.5, -0.5 ) );
    positions.push_back( Vec3f(  0.5,  0.5,  0.5 ) );
    positions.push_back( Vec3f(  0.5, -0.5,  0.5 ) );
    positions.push_back( Vec3f(  0.5, -0.5, -0.5 ) );

    positions.push_back( Vec3f(  0.5,  0.5,  0.5 ) );
    positions.push_back( Vec3f( -0.5,  0.5,  0.5 ) );
    positions.push_back( Vec3f( -0.5, -0.5,  0.5 ) );
    positions.push_back( Vec3f(  0.5, -0.5,  0.5 ) );

    positions.push_back( Vec3f( -0.5,  0.5,  0.5 ) );
    positions.push_back( Vec3f( -0.5,  0.5, -0.5 ) );
    positions.push_back( Vec3f( -0.5, -0.5, -0.5 ) );
    positions.push_back( Vec3f( -0.5, -0.5,  0.5 ) );

    positions.push_back( Vec3f( -0.5,  0.5,  0.5 ) );
    positions.push_back( Vec3f(  0.5,  0.5,  0.5 ) );
    positions.push_back( Vec3f(  0.5,  0.5, -0.5 ) );
    positions.push_back( Vec3f( -0.5,  0.5, -0.5 ) );

    positions.push_back( Vec3f( -0.5, -0.5,  0.5 ) );
    positions.push_back( Vec3f(  0.5, -0.5,  0.5 ) );
    positions.push_back( Vec3f(  0.5, -0.5, -0.5 ) );
    positions.push_back( Vec3f( -0.5, -0.5, -0.5 ) );
    mMesh.bufferPositions(positions);
*/
}

void Building::draw()
{
    gl::color( ColorA( 0.5f, 0.2f, 0.4f, 0.8f ) );
    float size = 25;
    gl::drawCube(Vec3f(-0, 0, 0) * size, Vec3f::one() * size);

    gl::color( ColorA( 0.0f, 0.0f, 0.0f, 0.8f ) );
//    gl::draw( mMesh );
}