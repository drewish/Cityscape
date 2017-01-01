//
//  BuildingPlan.cpp
//  Cityscape
//
//  Created by andrew morton on 6/8/15.
//
//

#include "BuildingPlan.h"
#include "CgalPolygon.h"
#include "CgalArrangement.h"
#include "CgalStraightSkeleton.h"
#include "GeometryHelpers.h"

#include "cinder/Rand.h"
#include "cinder/Triangulate.h"

#include <CGAL/Sweep_line_2_algorithms.h> // TODO: can drop soon?
#include <CGAL/Arr_overlay_2.h>

using namespace ci;
using namespace ci::geom;
using namespace std;


ci::PolyLine2f BuildingPlan::randomOutline()
{
    switch (randInt(5)) {
        case 0:
            return polyLineTriangle();
        case 1:
            return polyLineSquare();
        case 2:
            return polyLineLShape();
        case 3:
            return polyLinePlus();
        case 4:
            return polyLineTee();
        default:
            return polyLineRectangle( 10 * randInt( 3 ), 10 * randInt( 4 ) );
    }
}

// * * *

// Add walls defined by an upper contour into a Trimesh
void buildWalls( TriMesh &result, const PolyLine3f &upperContour, float lowerHeight = 0 )
{
    pointsInPairs<vec3>( upperContour, [&]( const vec3 &a, const vec3 &b ) {
        u_int32_t index_base = result.getNumVertices();
        vec3 verts[4] = { a, vec3( a.x, a.y, lowerHeight ), b, vec3( b.x, b.y, lowerHeight ) };
        result.appendPositions( verts, 4 );
        result.appendTriangle( index_base + 0, index_base + 1, index_base + 2 );
        result.appendTriangle( index_base + 1, index_base + 3, index_base + 2 );
    } );
}

// * * *

typedef std::map<std::pair<float, float>, vec3> OffsetMap;

// Build side faces of the roof
//
// We're building the walls using individual triangles (rather than strip) so we
// can use Cinder's triangulator to build the roof.
//
void buildSidesFromOutlineAndTopOffsets(const PolyLine2f &outline, const OffsetMap &topOffsets, vector<vec3> &verts, vector<uint32_t> &indices)
{
    const float defaultTopHeight = 0.0;
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
    for ( auto &i : roofIndices ) {
        indices.push_back( index + i );
    }
}

// Compute vertex height based off distance from incident edges
OffsetMap heightOfSkeleton( const SsPtr &skel, float slope )
{
    OffsetMap heightMap;
    for( auto vert = skel->vertices_begin(); vert != skel->vertices_end(); ++vert ) {
        if (vert->is_contour()) { continue; }

        InexactK::Point_2 p = vert->point();
        heightMap[ std::make_pair( p.x(), p.y() ) ] = vec3( 0, 0, vert->time() * slope );
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

TriMesh buildFlatRoof( const PolyLine2f &footprint, float height, float overhang )
{
    PolyLine2f roofOutline;
    if ( overhang > 0.0 ) {
        roofOutline = polyLineFrom( *expandPolygon( overhang, polygonFrom<InexactK>( footprint ) ) );
    } else {
        roofOutline = footprint;
    }

    PolyLine3f roofContour;
    for ( const vec2 &v : roofOutline.getPoints() ) {
        roofContour.push_back( vec3( v, height ) );
    }
    TriMesh result = Triangulator( roofContour ).calcMesh3d();

    PolyLine3f wallContour;
    for ( const vec2 &v : footprint.getPoints() ) {
        wallContour.push_back( vec3( v, height ) );
    }
    buildWalls( result, wallContour, 0 );

    return result;
}

void buildHippedRoof( const PolyLine2f &wallOutline, float slope, float overhang, vector<vec3> &verts, vector<uint32_t> &indices )
{
    CGAL::Polygon_2<InexactK> roofOutline = polygonFrom<InexactK>( wallOutline );

    if ( overhang > 0.0 ) {
        roofOutline = *expandPolygon( overhang, roofOutline );
    }

    // - build straight skeleton
    SsPtr skel = CGAL::create_interior_straight_skeleton_2( roofOutline, InexactK() );

    // - compute vertex height based off distance from incident edges
    OffsetMap offsetMap = heightOfSkeleton( skel, slope );

    // - triangulate roof faces and add to mesh
    buildRoofFromSkeletonAndOffsets(skel, offsetMap, verts, indices);
}

void buildGabledRoof( const PolyLine2f &wallOutline, float slope, float overhang, vector<vec3> &verts, vector<uint32_t> &indices )
{
    CGAL::Polygon_2<InexactK> roofOutline = polygonFrom<InexactK>( wallOutline );

    if ( overhang > 0.0 ) {
        // TODO: The geometry we generate from this isn't quite right. Since we
        // just moving the point on the skeleton's out to the outline--turning a
        // roof face into the gable face--the gable doesn't align with the walls.
        // Need to figure out how to remove the face, move the point, then
        // create new gables along the wall outline (probably using
        // buildSidesFromOutlineAndTopOffsets()).
        roofOutline = *expandPolygon( overhang, roofOutline );
    }

    // - build straight skeleton
    SsPtr skel = CGAL::create_interior_straight_skeleton_2( roofOutline, InexactK() );

    // - compute vertex height based off distance from incident edges
    OffsetMap offsetMap = heightOfSkeleton( skel, slope );

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

        vec2 A = vecFrom( contourA->vertex()->point() );
        vec2 B = vecFrom( contourB->vertex()->point() );
        vec2 C = vecFrom( skelEdge->vertex()->point() );

        // Adjust the skeleton vertex's position...
        auto it = offsetMap.find( std::make_pair( C.x, C.y ) );
        if ( it != offsetMap.end() ) {
            // ...to be between the other two points.
            vec2 adjustment = ( ( B + A ) / vec2( 2.0 ) ) - C;
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
void buildShedRoof( const PolyLine2f &wallOutline, float slope, float overhang, vector<vec3> &verts, vector<uint32_t> &indices )
{
    PolyLine2f roofOutline;
    if ( overhang > 0.0 ) {
        roofOutline = polyLineFrom( *expandPolygon( overhang, polygonFrom<InexactK>( roofOutline ) ) );
    } else {
        roofOutline = wallOutline;
    }

    // For now we find the left most point and have the roof slope up along the
    // x-axis from there.
    float leftmost = wallOutline.begin()->x;
    for ( auto &i : wallOutline ) {
        if (i.x < leftmost) leftmost = i.x;
    }

    OffsetMap offsetMap;
    // - compute the height of vertexes based on position on the roof
    for ( auto &i : wallOutline ) {
        offsetMap[ std::make_pair( i.x, i.y ) ] = vec3( 0, 0, slope * ( i.x - leftmost ) );
    }
    if ( overhang > 0.0 ) {
        for ( auto &i : roofOutline ) {
            offsetMap[ std::make_pair( i.x, i.y ) ] = vec3( 0, 0, slope * ( i.x - leftmost ) );
        }
    }

    // - triangulate roof faces and add to mesh
    buildRoofFaceFromOutlineAndOffsets( roofOutline, offsetMap, verts, indices );
    buildSidesFromOutlineAndTopOffsets( wallOutline, offsetMap, verts, indices );
}


Arrangement_2 arrangementForOutline( const PolyLine2f &outline )
{
    Arrangement_2 arr;
    OutlineObserver outObs( arr );

    std::vector<Segment_2> outlineSegments;
    contiguousSegmentsFrom( outline, back_inserter( outlineSegments ) );
    insert_empty( arr, outlineSegments.begin(), outlineSegments.end() );

    outObs.detach();

    return arr;
}

// Creates fields of roof (top parts)
//
// Expects the arrangement to have:
// - face data set to indicate holes
// - vertex data set with heights
TriMesh buildRoofFields( const Arrangement_2 &arrangement )
{
    // Now turn the arrangment into a mesh (there should just be one face with
    // an single hole).
    Triangulator triangulator;
    for ( auto face = arrangement.faces_begin(); face != arrangement.faces_end(); ++face ) {
        if ( !face->is_unbounded() && face->data() != FaceRole::Hole ) {
            PolyLine3f faceOutline( polyLine3fFrom( face->outer_ccb() ) );
            triangulator.addPolyLine( faceOutline );
        }
    }
	return triangulator.calcMesh3d();
}

// Creates gables of roof (sides)
//
// Expects the arrangement to have:
// - face data set to indicate holes
// - vertex data set with heights
TriMesh buildRoofGables( TriMesh result, const Arrangement_2 &arrangement )
{
    assert( result.getAttribDims( geom::Attrib::POSITION ) == 3 );

    // This is a little odd since it just builds walls for holes in the
    // unbounded face. The segments are in reverse order so need to reverse them
    // before we build walls.
    auto face = arrangement.unbounded_face();
    for ( auto hole = face->inner_ccbs_begin(); hole != face->inner_ccbs_end(); ++hole ) {
        auto start = *hole, edge = start;
        // Go around the loop until we find a valid edge to use as the
        // starting point. If nothing is valid then bail.
        bool ready = false;
        do {
            ready = edge->face()->data() == FaceRole::Hole && edge->twin()->face()->data() == FaceRole::Shape;
        } while( !ready && ++edge != start );
        if( !ready ) continue;

        start = edge;
        std::vector<vec3> outline;
        outline.push_back( vec3From( edge->source() ) );
        do {
            if ( edge->face()->data() == FaceRole::Hole && edge->twin()->face()->data() == FaceRole::Shape ) {
                outline.push_back( vec3From( edge->target() ) );
            }
        } while ( ++edge != start );

        // Ideally we'd do this as a Polyline but it's got a bug: https://github.com/cinder/Cinder/pull/1698
        std::reverse( outline.begin(), outline.end() );
        buildWalls( result, PolyLine3f( outline ), 0 );
    }

	return result;
}

struct SawtoothSettings {
    float downWidth;
    float upWidth;
    float valleyHeight;
    float peakHeight;
    float overhang;
};

float sawtoothHeight( const SawtoothSettings &settings, const float leftEdge, const vec2 &v ) {
    float p = fmod( v.x - leftEdge, settings.upWidth + settings.downWidth );
    float deltaY = settings.peakHeight - settings.valleyHeight;
    if ( p < settings.upWidth ) {
        // upslope
        return ( deltaY / settings.upWidth ) * p + settings.valleyHeight;
    } else {
        // downslope
        return ( -deltaY / settings.downWidth ) * ( p - settings.upWidth ) + settings.peakHeight;
    }
}

// TODO:
// - make the roof orientation configurable (requires a more complicated formula
//   for determining height)
TriMesh buildSawtoothRoof( const PolyLine2f &wallOutline, const SawtoothSettings &settings )
{
    if (wallOutline.size() < 3) return TriMesh();

    Arrangement_2 arrWalls = arrangementForOutline( wallOutline );
    Arrangement_2 arrRoofEdge;
    PolyLine2f roofOutline;
    if ( settings.overhang > 0.0 ) {
        roofOutline = polyLineFrom( *expandPolygon( settings.overhang, polygonFrom<InexactK>( wallOutline ) ) );
        arrRoofEdge = arrangementForOutline( roofOutline );
    } else {
        roofOutline = wallOutline;
        arrRoofEdge = arrWalls;
    }

    // TODO: would be nice to replace this with computeDividers() but that would need
    // an inital offset from the edge to work:
    // - down dividers (start with 0 offset, repeat every upWidth+downWidth)
    // - up dividers (start offset by upWidth, repeat every upWidth+downWidth)
    Arrangement_2 arrDividers;
    u_int16_t step = 0;
    Rectf bounds = Rectf( roofOutline.getPoints() );
    float x = bounds.x1;
    while ( x < bounds.x2 ) {
        Segment_2 segment = Segment_2( Point_2( x, bounds.y2 ), Point_2( x, bounds.y1 ) );
        CGAL::insert_non_intersecting_curve( arrDividers, segment );

        x += (step % 2) ? settings.downWidth : settings.upWidth;
        ++step;
    };

    Arr_extended_overlay_traits overlay_traits;

    // Compute the roof fields
    Arrangement_2 arrRoofFields;
    overlay( arrRoofEdge, arrDividers, arrRoofFields, overlay_traits );
    for ( auto i = arrRoofFields.vertices_begin(); i != arrRoofFields.vertices_end(); ++i ) {
        i->set_data( sawtoothHeight( settings, bounds.x1, vecFrom( i->point() ) ) );
    }

    // Now compute the gables
    Arrangement_2 arrRoofGables;
    overlay( arrWalls, arrDividers, arrRoofGables, overlay_traits );
    for ( auto i = arrRoofGables.vertices_begin(); i != arrRoofGables.vertices_end(); ++i ) {
        i->set_data( sawtoothHeight( settings, bounds.x1, vecFrom( i->point() ) ) );
    }

	return buildRoofGables( buildRoofFields( arrRoofFields ), arrRoofGables );
}

class RoofMesh : public Source {
public:
    RoofMesh( const PolyLine2f &outline, BuildingPlan::RoofStyle roof, float slope = 0.8, float overhang = 0.0f )
    {
        switch ( roof ) {
            case BuildingPlan::HIPPED_ROOF:
                buildHippedRoof( outline, slope, overhang, mPositions, mIndices );
                break;
            case BuildingPlan::GABLED_ROOF:
                buildGabledRoof( outline, slope, overhang, mPositions, mIndices );
                break;
            case BuildingPlan::SHED_ROOF:
                buildShedRoof( outline, slope, overhang, mPositions, mIndices );
                break;
//            case BuildingPlan::GAMBREL_ROOF:
//                // probably based off GABLED with an extra division of the faces to give it the barn look
//                break;
            default:
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
    RoofMesh* clone() const override { return new RoofMesh( *this ); }

protected:
    std::vector<vec3>       mPositions;
    std::vector<uint32_t>   mIndices;
};


ci::geom::SourceMods BuildingPlan::buildGeometry( const ci::PolyLine2f &outline, uint8_t floors, RoofStyle roofStyle, float slope, float overhang )
{
    const float FLOOR_HEIGHT = 10.0;
    float height = FLOOR_HEIGHT * floors;

    if ( roofStyle == BuildingPlan::FLAT_ROOF ) {
        return geom::SourceMods( buildFlatRoof( outline, height, overhang ) );
    } else if ( roofStyle == BuildingPlan::SAWTOOTH_ROOF ) {
        SawtoothSettings settings = { 0 };
        settings.valleyHeight = 0 + height;
        settings.upWidth = 10;
        settings.peakHeight = 3 + height;
        settings.downWidth = 5;
        settings.overhang = overhang;
        return geom::SourceMods( buildSawtoothRoof( outline, settings ) );
    } else {
        // Extrude centers on the origin so half the walls will be below ground
        geom::SourceMods walls = geom::Extrude( shapeFrom( outline ), height, 1.0f ).caps( false ) >> geom::Translate( vec3( 0, 0, height / 2.0f ) );
        geom::SourceMods roof = RoofMesh( outline, roofStyle, slope, overhang ) >> geom::Translate( 0, 0, height );
        return roof & walls;
    }
}
