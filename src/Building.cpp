//
//  Building.cpp
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#include "Building.h"
#include "cinder/Rand.h"
#include "cinder/Triangulate.h"

using namespace ci;
using namespace std;

void Building::layout()
{
//        mFloors = Rand::randInt(5);
    mMesh = makeMesh(mRoof, mOutline, mFloors);
}

void Building::draw( const Options &options )
{
    if ( options.drawBuildings ) {
//        gl::enableWireframe();
        gl::color( mColor );
        gl::draw( mMesh );
//        gl::disableWireframe();
    }
}

gl::VboMesh Building::makeMesh(RoofStyle roof, ci::PolyLine2f outline, uint32_t floors)
{
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();

    vector<Vec3f> verts;
    vector<uint32_t> indices;

    float roofHeight = floors * 10;

    // Build walls—we're doing this using individual triangles so we can use
    // Cinder's triangulator to build the flat roof.
    for ( auto i = outline.begin(); i != outline.end(); ++i ) {
        verts.push_back(Vec3f(i->x, i->y, 0));
        verts.push_back(Vec3f(i->x, i->y, roofHeight));
    }
    // I'm not 100% sure I got the right winding order here...
    uint32_t index;
    for ( index = 2; index < verts.size(); index += 2 ) {
        indices.push_back(index + 0);
        indices.push_back(index + 1);
        indices.push_back(index - 1);
        indices.push_back(index + 0);
        indices.push_back(index - 1);
        indices.push_back(index - 2);
    }
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(index - 1);
    indices.push_back(0);
    indices.push_back(index - 1);
    indices.push_back(index - 2);


    // Build roof
    if ( roof == FLAT ) {
        ci::Triangulator triangulator( outline );
        ci::TriMesh2d roofMesh = triangulator.calcMesh();

        std::vector<Vec2f> roofVerts = roofMesh.getVertices();
        for ( auto i = roofVerts.begin(); i != roofVerts.end(); ++i) {
            verts.push_back( Vec3f( *i, roofHeight ) );
        }

        std::vector<uint32_t> roofIndices = roofMesh.getIndices();
        for ( auto i = roofIndices.begin(); i != roofIndices.end(); ++i) {
            indices.push_back( index + *i );
        }
    }

    gl::VboMesh mesh = gl::VboMesh( verts.size(), indices.size(), layout, GL_TRIANGLES );
    mesh.bufferIndices( indices );
    mesh.bufferPositions( verts );

    return mesh;
}