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

#include <CGAL/Arr_overlay_2.h>

using namespace ci;
using namespace ci::geom;


ci::PolyLine2f BuildingPlan::randomOutline()
{
    switch ( randInt( 5 ) ) {
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

Arrangement_2 arrangementForOutline( const PolyLine2f &outline )
{
    std::vector<Segment_2> outlineSegments;
    contiguousSegmentsFrom( outline, back_inserter( outlineSegments ) );

    Arrangement_2 arr;
    OutlineObserver outObs( arr );
    insert_empty( arr, outlineSegments.begin(), outlineSegments.end() );
    outObs.detach();

    return arr;
}

// Add walls defined by an upper contour into a Trimesh.
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

// Build walls with a constant upper and lower height.
void buildWalls( TriMesh &result, const PolyLine2f &footprint, float upperHeight, float lowerHeight = 0 )
{
    PolyLine3f wallContour;
    for ( const vec2 &v : footprint.getPoints() ) {
        wallContour.push_back( vec3( v, upperHeight ) );
    }
    buildWalls( result, wallContour, lowerHeight );
}


// Expects the arrangement to have:
// - face data set to indicate holes
// - vertex data set with heights
std::vector< std::vector<vec3> > outlinesFromArrangement( const Arrangement_2 &arrangement )
{
    // This looks at holes in the arrangement and extracts their outer bounds
    // ignoring any divider segments sticking out.
    std::vector< std::vector<vec3> > results;
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
        std::vector<vec3> outline( { vec3From( edge->source() ) } );
        do {
            if ( edge->face()->data() == FaceRole::Hole && edge->twin()->face()->data() == FaceRole::Shape ) {
                outline.push_back( vec3From( edge->target() ) );
            }
        } while ( ++edge != start );

        std::reverse( outline.begin(), outline.end() );
        results.push_back( outline );
    }
    return results;
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
            std::vector<vec3> faceOutline = vec3sFrom( face->outer_ccb() );
            triangulator.addPolyLine( faceOutline.data(), faceOutline.size() );
        }
    }
	return triangulator.calcMesh3d();
}

// Creates gables of roof (sides)
//
// Expects the arrangement to have:
// - face data set to indicate holes
// - vertex data set with heights
void buildRoofGables( TriMesh &result, const Arrangement_2 &arrangement )
{
    assert( result.getAttribDims( geom::Attrib::POSITION ) == 3 );

    // This is a little odd since it just builds walls for holes in the
    // unbounded face. The holes are CW so we need to reverse them before we
    // build walls since it expects CCW.
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
}

// * * *

TriMesh buildFlatRoof( const PolyLine2f &footprint, float wallHeight, float overhang )
{
    PolyLine2f roofOutline;
    if ( overhang > 0.0 ) {
        roofOutline = polyLineFrom( *expandPolygon( overhang, polygonFrom<InexactK>( footprint ) ) );
    } else {
        roofOutline = footprint;
    }

    std::vector<vec3> roofContour;
    for ( const vec2 &v : roofOutline.getPoints() ) {
        roofContour.push_back( vec3( v, wallHeight ) );
    }
    TriMesh result = Triangulator( roofContour ).calcMesh3d();

    buildWalls( result, footprint, wallHeight, 0 );

    return result;
}

TriMesh buildHippedRoof( const PolyLine2f &footprint, float wallHeight, float slope, float overhang )
{
    CGAL::Polygon_2<InexactK> roofOutline = polygonFrom<InexactK>( footprint );
    if ( overhang > 0.0 ) {
        roofOutline = *expandPolygon( overhang, roofOutline );
    }

    // - build straight skeleton
    SsPtr skel = CGAL::create_interior_straight_skeleton_2( roofOutline, InexactK() );

    // Roof fields
    Triangulator triangulator;
    for( auto face = skel->faces_begin(); face != skel->faces_end(); ++face ) {
        std::vector<vec3> outline;
        Ss::Halfedge_handle edge = face->halfedge();
        do {
            auto v = edge->vertex();
            outline.push_back( vec3( v->point().x(), v->point().y(), v->time() * slope + wallHeight ) );
            edge = edge->next();
        } while (edge != face->halfedge());
        triangulator.addPolyLine( outline.data(), outline.size() );
    }
    TriMesh result = triangulator.calcMesh3d();
    // Walls
    buildWalls( result, footprint, wallHeight, 0 );

    return result;
}

TriMesh buildGabledRoof( const PolyLine2f &footprint, float wallHeight, float slope, float overhang )
{
    CGAL::Polygon_2<InexactK> roofOutline = polygonFrom<InexactK>( footprint );

    if ( overhang > 0.0 ) {
        roofOutline = *expandPolygon( overhang, roofOutline );
    }

    // Build straight skeleton
    SsPtr skel = CGAL::create_interior_straight_skeleton_2( roofOutline, InexactK() );

    // Insert the skeleton into the arrangement.
    Arrangement_2 dividerArr;
    dividerArr.unbounded_face()->set_data( FaceRole::Hole );
    std::vector<Segment_2> skeletonSegs;
    for ( auto edge = skel->halfedges_begin(); edge != skel->halfedges_end(); ++edge ) {
        const auto &currVert = edge->vertex();
        const auto &nextVert = edge->next()->vertex();

        // Ignore any degenerate segments so the arrangement doesn't complain.
        if ( currVert->time() == 0 && nextVert->time() == 0 ) continue;
        if ( currVert->point() == nextVert->point() ) continue;

        Segment_2 seg(
            Point_2( currVert->point().x(), currVert->point().y() ),
            Point_2( nextVert->point().x(), nextVert->point().y() )
        );
        // Skip it if the twin is in here
        if ( std::any_of( skeletonSegs.begin(), skeletonSegs.end(),
            [&seg]( const Segment_2 &other ) { return other.source() == seg.target() && other.target() == seg.source(); } ) ) continue;
        skeletonSegs.push_back( seg );

        // Stick all the points in now so we can associate the heights
        auto a = CGAL::insert_point( dividerArr, seg.source() );
        auto b = CGAL::insert_point( dividerArr, seg.target() );
        a->set_data( currVert->time() * slope + wallHeight );
        b->set_data( nextVert->time() * slope + wallHeight );
    }
    insert( dividerArr, skeletonSegs.begin(), skeletonSegs.end() );

    // Rather than two        *     We want one segment    *
    // segments forking out   |     going to the outline:  |
    // to the corners:        *                            *
    //                       / \                          /|\
    //                      *   *                        * * *
    // We leave the original edges so we overlap with the outline's corners and
    // get verts created with heights. If we didn't have them the overlay traits
    // would need to compute the height in th middle of the face.
    for ( auto vert = dividerArr.vertices_begin(); vert != dividerArr.vertices_end(); ++vert) {
        // look for 3 edges, two need to target data==0 and one to data>0
        if ( vert->degree() != 3 ) continue;
        // These are all edges with this vertex as the target
        // Hacky but simple way to find an above ground edge to compare with
        u_int8_t tries = 3;
        Arrangement_2::Halfedge_around_vertex_circulator circ = vert->incident_halfedges();
        while ( circ->source()->data() <= wallHeight && tries > 0 ) {
            ++circ;
            --tries;
        }
        if ( tries == 0 ) continue;
        Arrangement_2::Halfedge_handle edge = circ;
        auto e2 = ++circ;
        auto e3 = ++circ;

        // These two edges need to down at the top of the wall
        if ( e2->source()->data() > wallHeight || e3->source()->data() > wallHeight ) continue;
        Point_2 p1 = edge->source()->point();
        Point_2 p2 = e2->source()->point();
        Point_2 p3 = e3->source()->point();

        Point_2 newPoint( ( p2.x() + p3.x() ) / 2.0, ( p2.y() + p3.y() ) / 2.0 );
        auto newVert = CGAL::insert_point( dividerArr, newPoint );
        newVert->set_data( vert->data() );
        dividerArr.insert_at_vertices( Segment_2( edge->target()->point(), newPoint ), edge->target(), newVert );
    }
    setEdgeRoles( dividerArr, EdgeRole::Divider );
    setFaceRoles( dividerArr, FaceRole::Hole );


    OverlayWithHeight overlay_traits;


    // Roof fields
    std::vector<Segment_2> borderSegs;
    contiguousSegmentsFrom( roofOutline, std::back_inserter( borderSegs ) );
    Arrangement_2 roofEdgeArr;
    OutlineObserver outlineObserver( roofEdgeArr );
    insert_empty( roofEdgeArr, borderSegs.begin(), borderSegs.end() );
    outlineObserver.detach();
    setVertexData( roofEdgeArr, wallHeight );

    Arrangement_2 roofFieldArr;
    overlay( roofEdgeArr, dividerArr, roofFieldArr, overlay_traits );
    TriMesh result = buildRoofFields( roofFieldArr );


    // Walls and gables
    Arrangement_2 wallArr = arrangementForOutline( footprint );
    setVertexData( wallArr, wallHeight );

    Arrangement_2 gableArr;
    overlay( wallArr, dividerArr, gableArr, overlay_traits );
    buildRoofGables( result, gableArr );

    return result;
}

// TODO:
// - decide on how to orient the slope of the roof. one option is to find
//   longest side of the outline and use that to define the roof plane. another
//   would be passing in an angle.
// - get a proper formula for determining height
TriMesh buildShedRoof( const PolyLine2f &footprint, const float wallHeight, const float slope, const float overhang )
{
    PolyLine2f roofOutline;
    if ( overhang > 0.0 ) {
        roofOutline = polyLineFrom( *expandPolygon( overhang, polygonFrom<InexactK>( footprint ) ) );
    } else {
        roofOutline = footprint;
    }

    // For now we find the left most point and have the roof slope up along the
    // x-axis from there.
    float leftmost = Rectf( roofOutline.getPoints() ).getX1();

    std::vector<vec3> roofContour;
    for ( const vec2 &v : roofOutline ) {
        // - compute the height of vertexes based on position on the roof
        roofContour.push_back( vec3( v, slope * ( v.x - leftmost ) + wallHeight ) );
    }
    TriMesh result = Triangulator( roofContour ).calcMesh3d();

    PolyLine3f wallContour;
    for ( const vec2 &v : footprint ) {
        wallContour.push_back( vec3( v, slope * ( v.x - leftmost ) + wallHeight ) );
    }
    buildWalls( result, wallContour, 0 );

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
    arrDividers.unbounded_face()->set_data( FaceRole::Hole );
    u_int16_t step = 0;
    Rectf bounds = Rectf( roofOutline.getPoints() );
    float x = bounds.x1;
    while ( x < bounds.x2 ) {
        Segment_2 segment = Segment_2( Point_2( x, bounds.y2 ), Point_2( x, bounds.y1 ) );
        CGAL::insert_non_intersecting_curve( arrDividers, segment );

        x += (step % 2) ? settings.downWidth : settings.upWidth;
        ++step;
    };
    setEdgeRoles( arrDividers, EdgeRole::Divider );

    Arr_extended_overlay_traits overlay_traits;

    // Compute the roof fields
    Arrangement_2 arrRoofFields;
    overlay( arrRoofEdge, arrDividers, arrRoofFields, overlay_traits );
    for ( auto i = arrRoofFields.vertices_begin(); i != arrRoofFields.vertices_end(); ++i ) {
        i->set_data( sawtoothHeight( settings, bounds.x1, vecFrom( i->point() ) ) );
    }
    TriMesh result = buildRoofFields( arrRoofFields );

    // Now compute the gables
    Arrangement_2 arrRoofGables;
    overlay( arrWalls, arrDividers, arrRoofGables, overlay_traits );
    for ( auto i = arrRoofGables.vertices_begin(); i != arrRoofGables.vertices_end(); ++i ) {
        i->set_data( sawtoothHeight( settings, bounds.x1, vecFrom( i->point() ) ) );
    }
    for ( std::vector<vec3> outline : outlinesFromArrangement( arrRoofGables ) ) {
        buildWalls( result, outline );
    }

	return result;
}

ci::geom::SourceMods BuildingPlan::buildGeometry( const ci::PolyLine2f &outline, uint8_t floors, RoofStyle roofStyle, float slope, float overhang )
{
    const float FLOOR_HEIGHT = 10.0;
    float height = FLOOR_HEIGHT * floors;

    if ( roofStyle == BuildingPlan::FLAT_ROOF ) {
        return geom::SourceMods( buildFlatRoof( outline, height, overhang ) );
    } else if ( roofStyle == BuildingPlan::SHED_ROOF ) {
        return geom::SourceMods( buildShedRoof( outline, height, slope, overhang ) );
    } else if ( roofStyle == BuildingPlan::HIPPED_ROOF ) {
        return geom::SourceMods( buildHippedRoof( outline, height, slope, overhang ) );
    } else if ( roofStyle == BuildingPlan::GABLED_ROOF ) {
        return geom::SourceMods( buildGabledRoof( outline, height, slope, overhang ) );
    } else if ( roofStyle == BuildingPlan::SAWTOOTH_ROOF ) {
        SawtoothSettings settings = { 0 };
        settings.valleyHeight = 0 + height;
        settings.upWidth = 10;
        settings.peakHeight = 3 + height;
        settings.downWidth = 5;
        settings.overhang = overhang;
        return geom::SourceMods( buildSawtoothRoof( outline, settings ) );
    } else {
        return ci::geom::SourceMods();
    }
}
