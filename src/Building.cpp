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

#include "CinderCGAL.h"
#include <CGAL/create_straight_skeleton_2.h>
typedef CGAL::Straight_skeleton_2<InexactK> Ss;
typedef boost::shared_ptr<Ss> SsPtr;

using namespace ci;
using namespace std;

void Building::layout()
{
    mMesh = makeMesh(mRoof, mOutline, mFloors);
}

void Building::draw( const Options &options )
{
    if ( options.drawBuildings ) {
        gl::color( mColor );
        gl::draw( mMesh );
        gl::enableWireframe();
        gl::draw( mMesh );
        gl::disableWireframe();
    }
}

// Build walls
//
// We're building the walls using individual triangles (rather than strip) so we
// can use Cinder's triangulator to build the flat roof.
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

gl::VboMesh Building::makeMesh(const RoofStyle roof, const ci::PolyLine2f &outline, uint32_t floors)
{
    vector<Vec3f> verts;
    vector<uint32_t> indices;

    float roofHeight = floors * 10;

    buildWalls(outline, roofHeight, verts, indices);

    // Build roof
    if ( roof == FLAT ) {
        buildFlatRoof(outline, roofHeight, verts, indices);
    } else if ( roof == HIPPED ) {

        // - build straight skeleton
        SsPtr skel = CGAL::create_interior_straight_skeleton_2( polygonFrom<InexactK>( outline ), InexactK() );

        // - compute skeleton vertex height based off distance from incident edges?
        std::map<std::pair<float, float>, float> heightMap;
        for( auto vert = skel->vertices_begin(); vert != skel->vertices_end(); ++vert ) {
            if (vert->is_contour()) { continue; }

            Ss::Halfedge_handle definingEdge = *(vert->defining_contour_halfedges_begin());
            InexactK::Line_2 line = InexactK::Line_2(
                definingEdge->vertex()->point(),
                definingEdge->prev()->vertex()->point()
            );
            InexactK::Point_2 p = vert->point();
            heightMap[ std::make_pair( p.x(), p.y() ) ] = CGAL::sqrt( CGAL::squared_distance( p, line ) );
        }

        // - put triangulated faces into the mesh
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
    } else if ( roof == GABLED ) {
        // - build straight skeleton
        // - find skeleton vertexes with 3 edges, 2 of which are on the contour, then move that vertex out to contour
        // - compute skeleton vertex height based off distance from incident edges?
        // - triangulate faces
    } else if ( roof == GAMBREL ) {
        // probably based off GABLED with an extra division of the faces to give it the barn look
    } else if ( roof == SHED ) {
        // probably requires customizing the wall heights
        // - find longest line and use that as intersection of roof plane
        // - determine slope of roof
        // - compute heights of outline vertexes based on their position on roof plane
    }

    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();
    gl::VboMesh mesh = gl::VboMesh( verts.size(), indices.size(), layout, GL_TRIANGLES );
    mesh.bufferIndices( indices );
    mesh.bufferPositions( verts );
    return mesh;
}