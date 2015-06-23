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

typedef std::map<std::pair<float, float>, Vec3f> OffsetMap;

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
void buildWallsFromOutlineAndTopOffsets(const PolyLine2f &outline, const OffsetMap &topOffsets, const float defaultTopHeight, vector<Vec3f> &verts, vector<uint32_t> &indices)
{
    uint32_t base = verts.size();
    for ( auto i = outline.begin(); i != outline.end(); ++i ) {
        Vec3f bottom = Vec3f( i->x, i->y, 0.0 );
        auto it = topOffsets.find( std::make_pair( i->x, i->y ) );
        Vec3f topOffset = ( it == topOffsets.end() ) ? Vec3f( 0.0, 0.0, defaultTopHeight ) : it->second;

        verts.push_back( bottom );
        verts.push_back( bottom + topOffset );
    }
    uint16_t totalVerts = verts.size();

    uint32_t i = base + 2;
    for ( ; i < totalVerts; i += 2 ) {
        indices.push_back( i + 0 );
        indices.push_back( i + 1 );
        indices.push_back( i - 1 );
        indices.push_back( i + 0 );
        indices.push_back( i - 1 );
        indices.push_back( i - 2 );
    }
    indices.push_back( base + 0 );
    indices.push_back( base + 1 );
    indices.push_back( i - 1 );
    indices.push_back( base + 0 );
    indices.push_back( i - 1 );
    indices.push_back( i - 2 );
}

void buildRoofFaceFromOutlineAndOffsets( const PolyLine2f &outline, const OffsetMap &offsets, vector<Vec3f> &verts, vector<uint32_t> &indices )
{
    uint32_t index = verts.size();
    ci::Triangulator triangulator( outline );
    ci::TriMesh2d roofMesh = triangulator.calcMesh();

    std::vector<Vec2f> roofVerts = roofMesh.getVertices();
    for ( auto i = roofVerts.begin(); i != roofVerts.end(); ++i) {
        auto it = offsets.find( std::make_pair( i->x, i->y ) );
        Vec3f offset = it == offsets.end() ? Vec3f::zero() : it->second;
        verts.push_back( offset + Vec3f( *i, 0.0 ) );
    }

    std::vector<uint32_t> roofIndices = roofMesh.getIndices();
    for ( auto i = roofIndices.begin(); i != roofIndices.end(); ++i) {
        indices.push_back( index + *i );
    }
}

// compute vertex height based off distance from incident edges
OffsetMap heightOfSkeleton( const SsPtr &skel )
{
    OffsetMap heightMap;
    for( auto vert = skel->vertices_begin(); vert != skel->vertices_end(); ++vert ) {
        if (vert->is_contour()) { continue; }

        InexactK::Point_2 p = vert->point();
        heightMap[ std::make_pair( p.x(), p.y() ) ] = Vec3f( 0, 0, vert->time() );
    }
    return heightMap;
}

// - triangulate roof faces and add to mesh
void buildRoofFromSkeletonAndOffsets( const SsPtr &skel, const OffsetMap &offsets, vector<Vec3f> &verts, vector<uint32_t> &indices )
{
    for( auto face = skel->faces_begin(); face != skel->faces_end(); ++face ) {
        PolyLine2f faceOutline;
        Ss::Halfedge_handle start = face->halfedge(),
        edge = start;
        do {
            faceOutline.push_back( vecFrom( edge->vertex()->point() ) );
            edge = edge->next();
        } while (edge != start);

        buildRoofFaceFromOutlineAndOffsets( faceOutline, offsets, verts, indices );
    }
}

void buildFlatRoof(const PolyLine2f &outline, vector<Vec3f> &verts, vector<uint32_t> &indices)
{
    OffsetMap empty;
    buildRoofFaceFromOutlineAndOffsets( outline, empty, verts, indices );
}

void buildHippedRoof(const PolyLine2f &outline, vector<Vec3f> &verts, vector<uint32_t> &indices)
{
    // - build straight skeleton
    SsPtr skel = CGAL::create_interior_straight_skeleton_2( polygonFrom<InexactK>( outline ), InexactK() );

    // - compute vertex height based off distance from incident edges
    OffsetMap offsetMap = heightOfSkeleton( skel );

    // - triangulate roof faces and add to mesh
    buildRoofFromSkeletonAndOffsets(skel, offsetMap, verts, indices);
}

void buildGabledRoof(const PolyLine2f &outline, vector<Vec3f> &verts, vector<uint32_t> &indices)
{
    // - build straight skeleton
    SsPtr skel = CGAL::create_interior_straight_skeleton_2( polygonFrom<InexactK>( outline ), InexactK() );

    // - compute vertex height based off distance from incident edges
    OffsetMap offsetMap = heightOfSkeleton( skel );

    // - find faces with 3 edges: 1 skeleton and 2 contour
    for( auto face = skel->faces_begin(); face != skel->faces_end(); ++face ) {
        uint16_t skeletonVerts = 0;
        uint16_t contourVerts = 0;
        uint16_t otherVerts = 0;
        Ss::Vertex_handle skelVert;
        Ss::Halfedge_handle skelEdge;

        Ss::Halfedge_handle start = face->halfedge();
        Ss::Halfedge_handle edge = start;
        do {
            Ss::Vertex_handle vert = edge->vertex();
            if ( vert->is_contour() ) {
                ++contourVerts;
            } else if ( vert->is_skeleton() ) {
                ++skeletonVerts;
                skelVert = vert;
                skelEdge = edge;
            } else {
                ++otherVerts;
            }
            edge = edge->next();
        } while (edge != start);

        if ( skeletonVerts == 1 && contourVerts == 2 && otherVerts == 0) {
            // Find point where skeleton vector intersects contour edge
            Ss::Halfedge_handle contourA = skelEdge->next();
            Ss::Halfedge_handle contourB = contourA->next();
            Vec2f A = vecFrom( contourA->vertex()->point() );
            Vec2f B = vecFrom( contourB->vertex()->point() );
            Vec2f C = vecFrom( skelEdge->vertex()->point() );
            Vec2f adjustment =  ( (B + A) / 2.0 ) - C;

            // Adjust the position
            auto it = offsetMap.find( std::make_pair( skelVert->point().x(), skelVert->point().y() ) );
            if ( it != offsetMap.end() ) {
                it->second += Vec3f( adjustment, 0.0);
            }
        }
    }

    // - triangulate roof faces and add to mesh
    buildRoofFromSkeletonAndOffsets(skel, offsetMap, verts, indices);
}

// TODO:
// - decide on how to orient the slope of the roof. one option is to find
//   longest side of the outline and use that to define the roof plane. another
//   would be passing in an angle.
// - get a proper formula for determining height
void buildShedRoof(const PolyLine2f &outline, const float slope, vector<Vec3f> &verts, vector<uint32_t> &indices)
{
    // For now we find the left most point and have the roof slope up along the
    // x-axis from there.
    assert( outline.size() > 0 );
    float leftmost = outline.begin()->x;
    for (auto i = outline.begin() + 1; i != outline.end(); ++i) {
        if (i->x < leftmost) leftmost = i->x;
    }

    // - compute the height of vertexes base on position on the roof
    OffsetMap offsetMap;
    for (auto i = outline.begin(); i != outline.end(); ++i) {
        float z = slope * (i->x - leftmost);
        offsetMap[ std::make_pair( i->x, i->y ) ] = Vec3f( 0, 0, z );
    }

    // - triangulate roof faces and add to mesh
    buildRoofFaceFromOutlineAndOffsets( outline, offsetMap, verts, indices );
    buildWallsFromOutlineAndTopOffsets( outline, offsetMap, 0.0, verts, indices );
}

void BuildingPlan::makeMesh()
{
    vector<Vec3f> verts;
    vector<uint32_t> indices;

    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();


    // Build the walls
    buildWallsFromOutlineAndTopOffsets(mOutline, {}, mFloorHeight, verts, indices);
    mWallMeshRef = gl::VboMesh::create( verts.size(), indices.size(), layout, GL_TRIANGLES );
    mWallMeshRef->bufferIndices( indices );
    mWallMeshRef->bufferPositions( verts );


    // Build roof
    verts.clear();
    indices.clear();
    switch ( mRoof ) {
        case FLAT_ROOF:
            buildFlatRoof(mOutline, verts, indices);
            break;
        case HIPPED_ROOF:
            buildHippedRoof(mOutline, verts, indices);
            break;
        case GABLED_ROOF:
            buildGabledRoof(mOutline, verts, indices);
            break;
        case SAWTOOTH_ROOF:
            break;
        case SHED_ROOF:
            // Make slope configurable... might be good for other angled roofs.
            buildShedRoof(mOutline, 0.2, verts, indices);
            break;
        case GAMBREL_ROOF:
            // probably based off GABLED with an extra division of the faces to give it the barn look
            break;
    }
    mRoofMeshRef = gl::VboMesh::create( verts.size(), indices.size(), layout, GL_TRIANGLES );
    mRoofMeshRef->bufferIndices( indices );
    mRoofMeshRef->bufferPositions( verts );
}