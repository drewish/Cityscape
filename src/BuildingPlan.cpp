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
using namespace ci::geom;
using namespace std;

typedef std::map<std::pair<float, float>, vec3> OffsetMap;

const ci::PolyLine2f BuildingPlan::outline(const ci::vec2 offset, const float rotation) const
{
    PolyLine2f ret = PolyLine2f();

    glm::mat3 matrix;

    rotate( matrix, rotation );
    translate( matrix, offset );
    for( auto it = mOutline.begin(); it != mOutline.end(); ++it ) {
        vec3 transformed = matrix * vec3( *it, 1 );
        ret.push_back( vec2( transformed ) );
    }
    return ret;
}

// * * *
class WallMesh : public Source {
public:
    WallMesh( const PolyLine2f &outline, const OffsetMap &topOffsets, const float defaultTopHeight )
    {
        uint32_t base = 0;
        for ( auto i = outline.begin(); i != outline.end(); ++i ) {
            vec3 bottom = vec3( i->x, i->y, 0.0 );
            auto it = topOffsets.find( std::make_pair( i->x, i->y ) );
            vec3 topOffset = ( it == topOffsets.end() ) ? vec3( 0.0, 0.0, defaultTopHeight ) : it->second;

            mPositions.push_back( bottom );
            mPositions.push_back( bottom + topOffset );
        }
        uint16_t totalVerts = mPositions.size();

        uint32_t i;
        for ( i = base + 2; i < totalVerts; i += 2 ) {
            mIndices.push_back( i + 0 );
            mIndices.push_back( i + 1 );
            mIndices.push_back( i - 1 );
            mIndices.push_back( i + 0 );
            mIndices.push_back( i - 1 );
            mIndices.push_back( i - 2 );
        }
        mIndices.push_back( base + 0 );
        mIndices.push_back( base + 1 );
        mIndices.push_back( i - 1 );
        mIndices.push_back( base + 0 );
        mIndices.push_back( i - 1 );
        mIndices.push_back( i - 2 );
    };

    size_t    getNumVertices() const override { return mPositions.size(); }
    size_t    getNumIndices() const override { return mIndices.size(); }
    Primitive getPrimitive() const override { return Primitive::TRIANGLES; }
    uint8_t   getAttribDims( Attrib attr ) const override
    {
        switch( attr ) {
            case Attrib::POSITION: return 3;
            default: return 0;
        }
    }

    AttribSet getAvailableAttribs() const override { return { Attrib::POSITION }; }
    void    loadInto( Target *target, const AttribSet &requestedAttribs ) const override
    {
        target->copyAttrib( Attrib::POSITION, 3, 0, (const float*)mPositions.data(), mPositions.size() );
        target->copyIndices( Primitive::TRIANGLES, mIndices.data(), mIndices.size(), 4 );
    }

protected:
    std::vector<vec3>       mPositions;
    std::vector<uint32_t>   mIndices;
};
// * * *

// Build walls
//
// We're building the walls using individual triangles (rather than strip) so we
// can use Cinder's triangulator to build the roof.
//
void buildWallsFromOutlineAndTopOffsets(const PolyLine2f &outline, const OffsetMap &topOffsets, const float defaultTopHeight, vector<vec3> &verts, vector<uint32_t> &indices)
{
    uint32_t base = verts.size();
    for ( auto i = outline.begin(); i != outline.end(); ++i ) {
        vec3 bottom = vec3( i->x, i->y, 0.0 );
        auto it = topOffsets.find( std::make_pair( i->x, i->y ) );
        vec3 topOffset = ( it == topOffsets.end() ) ? vec3( 0.0, 0.0, defaultTopHeight ) : it->second;

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

// Use cinder's triangulator to convert a polygon into a face of a roof.
// The offsets adjust the positions of the verticies, allowing a 2d outline to
// fill a 3d space
void buildRoofFaceFromOutlineAndOffsets( const PolyLine2f &outline, const OffsetMap &offsets, vector<vec3> &verts, vector<uint32_t> &indices )
{
    uint32_t index = verts.size();
    ci::Triangulator triangulator( outline );
    ci::TriMesh roofMesh = triangulator.calcMesh();

    const vec2 *positions = roofMesh.getPositions<2>();
    uint32_t numVerts = roofMesh.getNumVertices();
    for ( uint32_t i = 0; i < numVerts ; i++ ) {
        vec3 position( positions[i], 0 );
        auto it = offsets.find( std::make_pair( position.x, position.y ) );
        vec3 offset = it == offsets.end() ? vec3(0) : it->second;
        verts.push_back( offset + position );
    }

    std::vector<uint32_t> roofIndices = roofMesh.getIndices();
    for ( auto i = roofIndices.begin(); i != roofIndices.end(); ++i) {
        indices.push_back( index + *i );
    }
}

// Compute vertex height based off distance from incident edges
OffsetMap heightOfSkeleton( const SsPtr &skel )
{
    OffsetMap heightMap;
    for( auto vert = skel->vertices_begin(); vert != skel->vertices_end(); ++vert ) {
        if (vert->is_contour()) { continue; }

        InexactK::Point_2 p = vert->point();
        heightMap[ std::make_pair( p.x(), p.y() ) ] = vec3( 0, 0, vert->time() );
    }
    return heightMap;
}

// - triangulate roof faces and add to mesh
void buildRoofFromSkeletonAndOffsets( const SsPtr &skel, const OffsetMap &offsets, vector<vec3> &verts, vector<uint32_t> &indices )
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

void buildFlatRoof(const PolyLine2f &outline, vector<vec3> &verts, vector<uint32_t> &indices)
{
    OffsetMap empty;
    buildRoofFaceFromOutlineAndOffsets( outline, empty, verts, indices );
}

void buildHippedRoof(const PolyLine2f &outline, vector<vec3> &verts, vector<uint32_t> &indices)
{
    // - build straight skeleton
    SsPtr skel = CGAL::create_interior_straight_skeleton_2( polygonFrom<InexactK>( outline ), InexactK() );

    // - compute vertex height based off distance from incident edges
    OffsetMap offsetMap = heightOfSkeleton( skel );

    // - triangulate roof faces and add to mesh
    buildRoofFromSkeletonAndOffsets(skel, offsetMap, verts, indices);
}

void buildGabledRoof(const PolyLine2f &outline, vector<vec3> &verts, vector<uint32_t> &indices)
{
    // - build straight skeleton
    SsPtr skel = CGAL::create_interior_straight_skeleton_2( polygonFrom<InexactK>( outline ), InexactK() );

    // - compute vertex height based off distance from incident edges
    OffsetMap offsetMap = heightOfSkeleton( skel );

    // - find faces with 3 edges: 1 skeleton and 2 contour
    for( auto face = skel->faces_begin(); face != skel->faces_end(); ++face ) {
        // Move around the face until we get to an edge with a skeleton
        // (seems to be the second edge).
        Ss::Halfedge_handle skelEdge = face->halfedge();
        do {
            skelEdge = skelEdge->next();
        } while ( !skelEdge->vertex()->is_skeleton() );

        // Bail if we don't have two contour verts followed by the skeleton vert.
        Ss::Halfedge_handle contourA = skelEdge->next();
        Ss::Halfedge_handle contourB = contourA->next();
        if (!contourA->vertex()->is_contour()) continue;
        if (!contourB->vertex()->is_contour()) continue;
        if (contourB->next() != skelEdge) continue;

        // Find point where skeleton vector intersects contour edge
        vec2 A = vecFrom( contourA->vertex()->point() );
        vec2 B = vecFrom( contourB->vertex()->point() );
        vec2 C = vecFrom( skelEdge->vertex()->point() );
        vec2 adjustment =  ( ( B + A ) / vec2( 2.0 ) ) - C;

        // Adjust the position
        auto it = offsetMap.find( std::make_pair( C.x, C.y ) );
        if ( it != offsetMap.end() ) {
            it->second += vec3( adjustment, 0.0 );
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
void buildShedRoof(const PolyLine2f &outline, const float slope, vector<vec3> &verts, vector<uint32_t> &indices)
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
        offsetMap[ std::make_pair( i->x, i->y ) ] = vec3( 0, 0, z );
    }

    // - triangulate roof faces and add to mesh
    buildRoofFaceFromOutlineAndOffsets( outline, offsetMap, verts, indices );
    buildWallsFromOutlineAndTopOffsets( outline, offsetMap, 0.0, verts, indices );
}

void buildSawtoothRoof(const PolyLine2f &outline, const float upWidth, const float downWidth, vector<vec3> &verts, vector<uint32_t> &indices)
{
    vector<PolyLine2f> outlines({ outline });
    OffsetMap offsets;

    // TODO Submit Cinder PR for PolyLine bounds
    Rectf bounds( outline.getPoints() );

    float x = bounds.x1;
    while ( x < bounds.x2 ) {
        vector<PolyLine2f> slice, intersection;
        float width;

        width = upWidth;
        slice = {
            PolyLine2f( {
                vec2(x+width, bounds.y1),
                vec2(x+width, bounds.y2),
                vec2(x, bounds.y2),
                vec2(x, bounds.y1),
            } ),
        };
        x += width;

        intersection = PolyLine2f::calcIntersection( slice, outlines );
        if (intersection.size()) {
            vector<vec2> points = intersection.front().getPoints();

            // HACKY: Find the points with the x value of the up point and set
            // those to our elevated height.
            for ( auto i = points.begin(); i != points.end(); ++i ) {
                if ( i->x == x ) {
                    offsets[ std::make_pair( i->x, i->y ) ] = vec3( 0, 0, 3 );
                }
            }

            reverse(points.begin(), points.end());
            buildRoofFaceFromOutlineAndOffsets( PolyLine2f(points), offsets, verts, indices );
        }

        width = downWidth;
        slice = {
            PolyLine2f( {
                vec2(x+width, bounds.y1),
                vec2(x+width, bounds.y2),
                vec2(x, bounds.y2),
                vec2(x, bounds.y1),
            } ),
        };
        x += width;

        intersection = PolyLine2f::calcIntersection( slice, outlines );
        if (intersection.size()) {
            // TODO Submit Cinder PR for PolyLine reverse.
            vector<vec2> points = intersection.front().getPoints();
            reverse(points.begin(), points.end());
            buildRoofFaceFromOutlineAndOffsets( PolyLine2f(points), offsets, verts, indices );
        }
    }

    // TODO can't use this yet because our outline doesn't have all the new points in it
    //    buildWallsFromOutlineAndTopOffsets( outline, offsets, 0.0, verts, indices );
}

class RoofMesh : public Source {
public:
    RoofMesh( const PolyLine2f &outline, BuildingPlan::RoofStyle roof )
    {
        switch ( roof ) {
            case BuildingPlan::FLAT_ROOF:
                buildFlatRoof( outline, mPositions, mIndices );
                break;
            case BuildingPlan::HIPPED_ROOF:
                buildHippedRoof( outline, mPositions, mIndices );
                break;
            case BuildingPlan::GABLED_ROOF:
                buildGabledRoof( outline, mPositions, mIndices );
                break;
            case BuildingPlan::SAWTOOTH_ROOF:
                buildSawtoothRoof( outline, 2, 3, mPositions, mIndices );
                break;
            case BuildingPlan::SHED_ROOF:
                // Make slope configurable... might be good for other angled roofs.
                buildShedRoof( outline, 0.2, mPositions, mIndices );
                break;
            case BuildingPlan::GAMBREL_ROOF:
                // probably based off GABLED with an extra division of the faces to give it the barn look
                break;
        }
    };

    size_t    getNumVertices() const override { return mPositions.size(); }
    size_t    getNumIndices() const override { return mIndices.size(); }
    Primitive getPrimitive() const override { return Primitive::TRIANGLES; }
    uint8_t   getAttribDims( Attrib attr ) const override
    {
        switch( attr ) {
            case Attrib::POSITION: return 3;
            default: return 0;
        }
    }

    AttribSet getAvailableAttribs() const override { return { Attrib::POSITION }; }
    void    loadInto( Target *target, const AttribSet &requestedAttribs ) const override
    {
        target->copyAttrib( Attrib::POSITION, 3, 0, (const float*)mPositions.data(), mPositions.size() );
        target->copyIndices( Primitive::TRIANGLES, mIndices.data(), mIndices.size(), 4 );
    }

protected:
    std::vector<vec3>       mPositions;
    std::vector<uint32_t>   mIndices;
};


void BuildingPlan::makeMesh()
{
    // Build the walls
    mWallMeshRef = gl::VboMesh::create( WallMesh( mOutline, {}, mFloorHeight ) );

    // Build roof
    mRoofMeshRef = gl::VboMesh::create( RoofMesh( mOutline, mRoof ) );
}