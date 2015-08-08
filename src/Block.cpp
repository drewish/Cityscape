//
//  Block.cpp
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#include "Block.h"
#include "Lot.h"

#include "cinder/app/App.h"
using namespace ci;

#include "CinderCGAL.h"
#include <CGAL/create_straight_skeleton_from_polygon_with_holes_2.h>
#include <CGAL/Polygon_set_2.h>

#include "CgalArrangement.h"


typedef CGAL::Straight_skeleton_2<InexactK> Ss;
typedef boost::shared_ptr<Ss> SsPtr;

typedef std::map<std::pair<float, float>, vec3> OffsetMap;

void Block::layout( const Options &options )
{
    switch (options.block.division) {
    case BlockOptions::BLOCK_DIVIDED:
        subdivideForReal();
        break;
    case BlockOptions::NO_BLOCK_DIVISION:
        subdivideNotReally();
        break;
    }

    for( auto it = mLots.begin(); it != mLots.end(); ++it ) {
        it->layout( options );
    }
}

void Block::draw( const Options &options )
{
    if ( options.drawBlocks ) {
        gl::color( ColorA( 0.0f, 0.8f, 0.2f, 0.5f ) );
        gl::draw( mShape.mesh() );
    }
}

// Use the entire block for a lot.
void Block::subdivideNotReally()
{
    mLots.clear();
    PolyLine2f lotOutline = mShape.outline();
    Lot lot = Lot( lotOutline, ColorA( CM_HSV, 1, 1.0, 0.75, 0.5 ) );
    mLots.push_back(lot);
}

// Old straight skeleton based lot sub divisioning
void Block::subdivideSkeleton()
{
    // Assume the outline is closed and first == last.
    // Don't want to bother with less than a triangle.
    if (mShape.outline().size() < 4) return;

    float steps = 0.0;

    SsPtr skel = CGAL::create_interior_straight_skeleton_2( mShape.polygon_with_holes<InexactK>() );

    mLots.clear();
    mLots.reserve(skel->size_of_faces());

    // Avoid triangular lots on the ends of blocks.
    OffsetMap offsetMap;
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

        // Record the adjusted postion
        offsetMap[ std::make_pair( C.x, C.y ) ] = vec3( adjustment, 0 );
    }


    for( auto face = skel->faces_begin(); face != skel->faces_end(); ++face ) {
        PolyLine2f lotOutline;
        Ss::Halfedge_handle start = face->halfedge(),
            edge = start;
        do {
            vec2 position = vecFrom( edge->vertex()->point() );
            auto it = offsetMap.find( std::make_pair( position.x, position.y ) );
            vec2 offset = it == offsetMap.end() ? vec2( 0 ) : vec2( it->second.x, it->second.y );
            lotOutline.push_back( offset + position );
            edge = edge->next();
        } while ( edge != start );

        // Skip small lots
        if (lotOutline.calcArea() < 1) continue;

        Lot l = Lot( lotOutline, ColorA( CM_HSV, steps, 1.0, 0.75, 0.5 ) );
        mLots.push_back(l);

        steps += 0.17;
        if (steps > 1) steps -= 1.0;
    }
}


// Gives back pairs of points to divide the shape with lines of a given angle.
std::vector<vec2> makeDividers( const std::vector<vec2> &outline, const float angle = 0, const float width = 100 )
{
    // Rotate the shape to the desired angle...
    Rectf outlineBounds( outline );
    vec2 center = vec2( outlineBounds.getWidth() / 2.0, outlineBounds.getHeight() / 2.0 );
    glm::mat3 matrix;
    matrix = translate( rotate( translate( matrix, -center ), angle ), center );

    // ...then find the bounding box...
    std::vector<vec2> rotated;
    for( auto it = outline.begin(); it != outline.end(); ++it ) {
        rotated.push_back( vec2( matrix * vec3( *it, 1 ) ) );
    }
    Rectf bounds( rotated );

    // ...now figure out where the left edge of that box would be in the
    // unrotated space...
    mat3 reverse = inverse( matrix );
    vec2 topLeft =    vec2( reverse * vec3( bounds.getUpperLeft(), 1 ) );
    vec2 bottomLeft = vec2( reverse * vec3( bounds.getLowerLeft(), 1 ) );
    vec2 direction = normalize( vec2( reverse * ( vec3( 1, 0, 0 ) ) ) );

    // ...and work across from those points finding dividers
    std::vector<vec2> result;
    for ( float distance = width; distance < bounds.getWidth(); distance += width ) {
        vec2 thing = direction * distance;
        result.push_back( thing + topLeft );
        result.push_back( thing + bottomLeft );
    }

    return result;
}

void Block::subdivideForReal()
{
    float steps = 0.0;

    Arrangement_2 mArr;
    std::vector<vec2> mDividers;

//    // Assume the outline is closed and first == last.
//    // Don't want to bother with less than a triangle.
//    if (mShape.outline().size() < 4) return;

    // Put the outline onto the arrangment.
    std::list<Segment_2> outlineSegments = contiguousSegmentsFrom( mShape.outline().getPoints() );
    insert_empty( mArr, outlineSegments.begin(), outlineSegments.end() );

    // ...and a list of segements to intersect with.
    std::list<Segment_2> intersect;
    intersect.insert( intersect.begin(), outlineSegments.begin(), outlineSegments.end() );

    std::list<Segment_2> newEdges;
    std::list<Point_2> newPoints;

    float angle = 0;

    if ( mShape.outline().size() > 3 ) {
        // Build straight skeleton
//        // TODO figure out why we need to reverse this...
//        CGAL::Polygon_2<InexactK> poly = polygonFrom<InexactK>( mShape.polygon_with_holes<InexactK>() );
//        if ( poly.is_clockwise_oriented() ) {
//            poly.reverse_orientation();
//        }
        SsPtr skel = CGAL::create_interior_straight_skeleton_2( mShape.polygon_with_holes<InexactK>() );

        // Put all the skeleton lines in
        Ss::Halfedge longest = *skel->halfedges_begin();
        float len = 0;

        for( auto edge = skel->halfedges_begin(); edge != skel->halfedges_end(); ++edge ) {
            auto curr = edge->vertex(), next = edge->next()->vertex();
            auto a = curr->point(), b = next->point();
            if ( curr->is_skeleton()
            //&& next->is_skeleton()
            ) {
                newEdges.push_back( Segment_2( Point_2( a.x(), a.y() ), Point_2( b.x(), b.y() ) ) );
            }

            // Find the angle of the longest segment
            if ( edge->is_border() ) {
                vec2 v = vecFrom(b) - vecFrom(a);
                if (glm::length(v) > len) {
                    len = glm::length(v);
                    angle = atan2(v.y, v.x);
                    longest = *edge;
                }
            }
        }

        // Gind faces with 3 edges: 1 skeleton and 2 contour
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
            vec2 adjustment =  ( ( B + A ) / vec2( 2.0 ) );

            // Create a segment for the adjusted edge
//            newEdges.push_back( Segment_2( Point_2( C.x, C.y ), Point_2( adjustment.x, adjustment.y ) ) );
        }
    }

    mDividers = makeDividers( mShape.outline().getPoints(), angle, 25 );

    // Then start walking across the outline looking for the intersections...
    auto segs = segmentsFrom( mDividers );
    for ( auto i = segs.begin(); i != segs.end(); ++i ) {
        intersect.push_back( *i );
        findIntersections( intersect, newEdges, newPoints );
        intersect.pop_back();
    }

    // Add the new edges all at once for better performance.
    if (newEdges.size()) insert( mArr, newEdges.begin(), newEdges.end() );


//    // Convert into lots
//    for ( auto i = mArr.faces_begin(); i != mArr.faces_end(); ++i ) {
//        for ( auto j = i->holes_begin(); j != i->holes_end(); ++j ) {
//            PolyLine2f faceOutline;
//            Arrangement_2::Ccb_halfedge_circulator cc = *j;
//            do {
//                Arrangement_2::Halfedge_handle he = cc;
//                faceOutline.push_back( vecFrom( he->target()->point() ) );
//            } while ( ++cc != *j );
//            gl::drawSolid( faceOutline );
//        }
//    }

    for( auto face = mArr.faces_begin(); face != mArr.faces_end(); ++face ) {
        for ( auto j = face->outer_ccbs_begin(); j != face->outer_ccbs_end(); ++j ) {
            PolyLine2f lotOutline;
            Arrangement_2::Ccb_halfedge_circulator cc = *j;
//            lotOutline.push_back( vecFrom( cc->source()->point() ) );
            do {
                Arrangement_2::Halfedge_handle he = cc;
                lotOutline.push_back( vecFrom( he->target()->point() ) );
            } while ( ++cc != *j );

            // Skip small lots
            if (lotOutline.calcArea() < 1) continue;

            Lot l = Lot( lotOutline, ColorA( CM_HSV, steps, 1.0, 0.75, 0.5 ) );
            mLots.push_back(l);

            steps += 0.17;
            if (steps > 1) steps -= 1.0;
        }
    }
}