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

typedef CGAL::Straight_skeleton_2<InexactK> Ss;
typedef boost::shared_ptr<Ss> SsPtr;

typedef std::map<std::pair<float, float>, vec3> OffsetMap;

void Block::layout( const Options &options )
{
    if (options.blockDivision == 1) {
        subdivideSkeleton();
    }
    else {
        subdivideNotReally();
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

