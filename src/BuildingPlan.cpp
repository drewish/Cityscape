//
//  BuildingPlan.cpp
//  Cityscape
//
//  Created by andrew morton on 6/8/15.
//
//

#include "BuildingPlan.h"

#include "cinder/Triangulate.h"

#include "CinderCGAL.h"
#include <CGAL/create_straight_skeleton_2.h>

typedef CGAL::Straight_skeleton_2<InexactK> Ss;
typedef boost::shared_ptr<Ss> SsPtr;

using namespace ci;
using namespace std;

const ci::PolyLine2f BuildingPlan::outline(const ci::Vec2f offset, const float rotation) const
{
    PolyLine2f ret = PolyLine2f();

    MatrixAffine2f matrix;
    matrix.rotate( rotation );
    matrix.translate( offset );
    for( auto it = mOutline.begin(); it != mOutline.end(); ++it ) {
        ret.push_back( matrix.transformPoint( *it ) );
    }
    return ret;
}

// Build walls
//
// We're building the walls using individual triangles (rather than strip) so we
// can use Cinder's triangulator to build the roof.
//
void buildWalls(const PolyLine2f &outline, const float roofHeight, vector<Vec3f> &verts, vector<uint32_t> &indices)
{
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
}

void buildFlatRoof(const PolyLine2f &outline, const float roofHeight, vector<Vec3f> &verts, vector<uint32_t> &indices)
{
    uint32_t index = verts.size();

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

void buildHippedRoof(const PolyLine2f &outline, const float roofHeight, vector<Vec3f> &verts, vector<uint32_t> &indices)
{
    // - build straight skeleton
    SsPtr skel = CGAL::create_interior_straight_skeleton_2( polygonFrom<InexactK>( outline ), InexactK() );

    // - compute vertex height based off distance from incident edges
    std::map<std::pair<float, float>, float> heightMap;
    for( auto vert = skel->vertices_begin(); vert != skel->vertices_end(); ++vert ) {
        if (vert->is_contour()) { continue; }

        InexactK::Point_2 p = vert->point();
        heightMap[ std::make_pair( p.x(), p.y() ) ] = vert->time();
    }

    // - triangulate roof faces and add to mesh
    for( auto face = skel->faces_begin(); face != skel->faces_end(); ++face ) {
        PolyLine2f faceOutline;
        Ss::Halfedge_handle start = face->halfedge(),
        edge = start;
        do {
            faceOutline.push_back( vecFrom( edge->vertex()->point() ) );
            edge = edge->next();
        } while (edge != start);

        uint32_t index = verts.size();
        ci::Triangulator triangulator( faceOutline );
        ci::TriMesh2d roofMesh = triangulator.calcMesh();

        std::vector<Vec2f> roofVerts = roofMesh.getVertices();
        for ( auto i = roofVerts.begin(); i != roofVerts.end(); ++i) {
            auto it = heightMap.find( std::make_pair( i->x, i->y ) );
            float h = it == heightMap.end() ? 0.0 : it->second;
            verts.push_back( Vec3f( *i, roofHeight + h ) );
        }

        std::vector<uint32_t> roofIndices = roofMesh.getIndices();
        for ( auto i = roofIndices.begin(); i != roofIndices.end(); ++i) {
            indices.push_back( index + *i );
        }
    }
}

gl::VboMeshRef BuildingPlan::makeMesh()
{
    vector<Vec3f> verts;
    vector<uint32_t> indices;

    float roofHeight = mFloors * 10;

    buildWalls(mOutline, roofHeight, verts, indices);

    // Build roof
    switch ( mRoof ) {
        case FLAT_ROOF:
            buildFlatRoof(mOutline, roofHeight, verts, indices);
            break;
        case HIPPED_ROOF:
            buildHippedRoof(mOutline, roofHeight, verts, indices);
            break;
        case GABLED_ROOF:
            // - build straight skeleton
            // - find skeleton vertexes with 3 edges, 2 of which are on the contour, then move that vertex out to contour
            // - compute skeleton vertex height based off distance from incident edges?
            // - triangulate faces
            break;
        case GAMBREL_ROOF:
            // probably based off GABLED with an extra division of the faces to give it the barn look
            break;
        case SHED_ROOF:
            // probably requires customizing the wall heights
            // - find longest line and use that as intersection of roof plane
            // - determine slope of roof
            // - compute heights of outline vertexes based on their position on roof plane
            break;
    }

    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();
    gl::VboMeshRef mesh = gl::VboMesh::create( verts.size(), indices.size(), layout, GL_TRIANGLES );
    mesh->bufferIndices( indices );
    mesh->bufferPositions( verts );
    return mesh;
}