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


// Add walls defined by an upper contour into a Trimesh.
void buildWalls( TriMesh &result, const std::vector<vec3> &upperContour, float lowerHeight = 0 )
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
    std::vector<vec3> wallContour;
    std::transform( footprint.begin(), footprint.end(), std::back_inserter( wallContour ),
        [=]( const vec2 &v ) { return vec3( v, upperHeight ); } );
    buildWalls( result, wallContour, lowerHeight );
}


// This looks at holes in the unbounded face of the arrangement and extracts
// their outer bounds ignoring any divider segments sticking out.
//
// Expects the arrangement to have:
// - face data set to indicate holes
// - vertex data set with heights
std::vector< std::vector<vec3> > outlinesFromArrangement( const Arrangement_2 &arrangement )
{
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

        // The holes are CW so we need to reverse them since wall building
        // expects CCW.
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

// Duplicate all the triangles with inverted winding order. Helpful for roofs
// that extend pass the walls so they're solid when viewed from below.
void mirrorTriangles( TriMesh &mesh ) {
    std::vector<uint32_t> indices( mesh.getIndices() );
    for( size_t i = 0, size = mesh.getNumIndices(); i < size; i += 3 ) {
        std::swap( indices[i], indices[i+1] );
    }
    mesh.appendIndices( indices.data(), indices.size() );
}

void concatenateMeshes( TriMesh &lhs, const TriMesh &rhs )
{
    uint32_t base_index = lhs.getNumVertices();
    std::vector<uint32_t> indices( rhs.getIndices() );
    for( size_t i = 0, size = rhs.getNumIndices(); i < size; ++i ) {
        indices[i] += base_index;
    }
    lhs.appendIndices( indices.data(), indices.size() );
    lhs.appendPositions( rhs.getPositions<3>(), rhs.getNumVertices() );
}

// * * *

TriMesh buildingWithFlatRoof( const PolyLine2f &footprint, float wallHeight, float overhang )
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
    if ( overhang > 0.0 ) {
        mirrorTriangles( result );
    }

    buildWalls( result, footprint, wallHeight, 0 );

    return result;
}

TriMesh buildingWithHippedRoof( const PolyLine2f &footprint, float wallHeight, float slope, float overhang )
{
    CGAL::Polygon_2<InexactK> roofOutline = polygonFrom<InexactK>( footprint );
    if ( overhang > 0.0 ) {
        roofOutline = *expandPolygon( overhang, roofOutline );
    }

    // Build straight skeleton
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

    // Soffits
    if ( overhang > 0.0 ) {
        std::vector<vec3> roofline;
        for ( auto v = roofOutline.vertices_begin(); v != roofOutline.vertices_end(); ++v ) {
            roofline.push_back( vec3( vecFrom( *v ), wallHeight ) );
        }
        concatenateMeshes( result, Triangulator( roofline ).calcMesh3d( vec3( 0, 0, -1 ) ) );
    }

    // Walls
    buildWalls( result, footprint, wallHeight, 0 );

    return result;
}

TriMesh buildingWithGabledRoof( const PolyLine2f &footprint, float wallHeight, float slope, float overhang )
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
    // would need to compute the height in the middle of the face.
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
    if ( overhang > 0.0 ) {
        mirrorTriangles( result );
    }


    // Walls and gables
    Arrangement_2 wallArr = arrangementFromOutline( footprint );
    setVertexData( wallArr, wallHeight );

    Arrangement_2 gableArr;
    overlay( wallArr, dividerArr, gableArr, overlay_traits );
    for ( std::vector<vec3> outline : outlinesFromArrangement( gableArr ) ) {
        buildWalls( result, outline );
    }

    return result;
}

// TODO:
// - decide on how to orient the slope of the roof. one option is to find
//   longest side of the outline and use that to define the roof plane. another
//   would be passing in an angle.
// - get a proper formula for determining height
TriMesh buildingWithShedRoof( const PolyLine2f &footprint, float wallHeight, float slope, float overhang )
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
        // Compute the height of vertexes based on position on the roof
        roofContour.push_back( vec3( v, slope * ( v.x - leftmost ) + wallHeight ) );
    }
    TriMesh result = Triangulator( roofContour ).calcMesh3d();
    if ( overhang > 0.0 ) {
        mirrorTriangles( result );
    }

    std::vector<vec3> wallContour;
    for ( const vec2 &v : footprint ) {
        wallContour.push_back( vec3( v, slope * ( v.x - leftmost ) + wallHeight ) );
    }
    buildWalls( result, wallContour, 0 );

    return result;
}

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
TriMesh buildingWithSawtoothRoof( const PolyLine2f &wallOutline, const SawtoothSettings &settings )
{
    Arrangement_2 arrWalls = arrangementFromOutline( wallOutline );
    Arrangement_2 arrRoofEdge;
    PolyLine2f roofOutline;
    if ( settings.overhang > 0.0 ) {
        roofOutline = polyLineFrom( *expandPolygon( settings.overhang, polygonFrom<InexactK>( wallOutline ) ) );
        arrRoofEdge = arrangementFromOutline( roofOutline );
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
    // TODO: would be nice to ditch this height computation in favor of using
    // OverlayWithHeight but it doesn't handle interpolating a height in the
    // middle of a face.
    for ( auto i = arrRoofFields.vertices_begin(); i != arrRoofFields.vertices_end(); ++i ) {
        i->set_data( sawtoothHeight( settings, bounds.x1, vecFrom( i->point() ) ) );
    }
    TriMesh result = buildRoofFields( arrRoofFields );
    if ( settings.overhang > 0.0 ) {
        mirrorTriangles( result );
    }

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

ci::geom::SourceMods buildingGeometry( const ci::PolyLine2f &outline, const BuildingSettings &settings )
{
    ci::geom::SourceMods geometry;
    const float FLOOR_HEIGHT = 10.0;
    float height = FLOOR_HEIGHT * settings.floors;

    if ( outline.size() < 3 ) {
        return geometry;
    }
    PolyLine2f floorplan( outline );
    if ( floorplan.isClockwise() ) {
        floorplan.reverse();
    }
    if ( floorplan.getPoints().front() != floorplan.getPoints().back() ) {
        floorplan.push_back( floorplan.getPoints().front() );
    }

    if ( settings.roofStyle == RoofStyle::FLAT ) {
        geometry = buildingWithFlatRoof( floorplan, height, settings.overhang );
    } else if ( settings.roofStyle == RoofStyle::SHED ) {
        geometry = buildingWithShedRoof( floorplan, height, settings.slope, settings.overhang );
    } else if ( settings.roofStyle == RoofStyle::HIPPED ) {
        geometry = buildingWithHippedRoof( floorplan, height, settings.slope, settings.overhang );
    } else if ( settings.roofStyle == RoofStyle::GABLED ) {
        geometry = buildingWithGabledRoof( floorplan, height, settings.slope, settings.overhang );
    } else if ( settings.roofStyle == RoofStyle::SAWTOOTH ) {
        SawtoothSettings sawtooth = { 0 };
        sawtooth.valleyHeight = 0 + height;
        sawtooth.upWidth = 10;
        sawtooth.peakHeight = 3 + height;
        sawtooth.downWidth = 5;
        sawtooth.overhang = settings.overhang;
        geometry = buildingWithSawtoothRoof( floorplan, sawtooth );
    }

    return geometry;
}
