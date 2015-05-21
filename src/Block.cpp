//
//  Block.cpp
//  Cityscape
//
//  Created by andrew morton on 2/16/15.
//
//

#include "Block.h"
#include "Lot.h"

#include "cinder/app/AppNative.h"
using namespace ci;

#include "CinderCGAL.h"
#include <CGAL/exceptions.h>
#include <CGAL/create_straight_skeleton_from_polygon_with_holes_2.h>
typedef CGAL::Straight_skeleton_2<K> Ss;
typedef boost::shared_ptr<Ss> SsPtr;

void drawSkeleton(const SsPtr &ss)
{
    if (!ss) return;

    float col = 0;

    gl::lineWidth(2);
    // Draw the faces
    for( auto face = ss->faces_begin(); face != ss->faces_end(); ++face ) {
        PolyLine2f line;

        Ss::Halfedge_const_handle begin = face->halfedge();
        Ss::Halfedge_const_handle edge = begin;
        do {
            line.push_back(vecFrom(edge->vertex()->point()));
            edge = edge->next();
        } while (edge != begin);
        gl::color( ColorA( 1.0-col,0,col,0.5) );
        col += 0.1;
        gl::drawSolid( line );
        gl::color( ColorA( 1,0,0,0.5 ) );
        gl::draw( line );
        gl::drawSolidCircle(vecFrom(begin->vertex()->point()), 5);
    }

    // Then the outline
    PolyLine2f outline;
    Ss::Halfedge_const_handle begin = ss->faces_begin()->halfedge()->opposite();
    Ss::Halfedge_const_handle edge = begin;
    do {
        outline.push_back(vecFrom(edge->vertex()->point()));
        edge = edge->prev();
    } while (edge != begin);

    gl::lineWidth(1);
    gl::color( ColorA( 0,1,0,0.5 ) );
    gl::draw( outline );
    gl::drawSolidCircle(vecFrom(begin->vertex()->point()), 2);
}

void Block::setup()
{
    for( auto it = mLots.begin(); it != mLots.end(); ++it ) {
        it->setup();
    }
}

void Block::draw()
{
    gl::color( ColorA( 0.0f, 0.8f, 0.2f, 0.5f ) );
    // gl::draw( mMesh );

    for( auto it = mLots.begin(); it != mLots.end(); ++it ) {
        it->draw();
    }
}

void Block::subdivide()
{
    // Assume the outline is closed and first == last.
    // Don't want to bother with less than a triangle.
    if (mShape.outline().size() < 4) return;

    Polygon_with_holes_2 poly( polygonFrom( mShape.outline() ) );
    for ( auto it = mShape.holes().begin(); it != mShape.holes().end(); ++it ) {
        poly.add_hole( polygonFrom( *it ) );
    }

    float steps = 0.0;

    try {
        SsPtr skel = CGAL::create_interior_straight_skeleton_2(poly);

        mLots.clear();
        mLots.reserve(skel->size_of_faces());


        unsigned int lot_id = 0;
        for( auto face = skel->faces_begin(); face != skel->faces_end(); ++face ) {

            PolyLine2f lotOutline;
            Ss::Halfedge_handle edge, start;

            start = face->halfedge();
            edge = start;
            do {
                lotOutline.push_back(vecFrom(edge->vertex()->point()));
                edge = edge->next();
            } while (edge != start);

            Lot l = Lot(lot_id++, lotOutline);

            l.mColor = Color( 1.0-steps,0,steps );
            steps += 0.1;

            mLots.push_back(l);
        }
    }
    catch (CGAL::Precondition_exception e) {
        return; // TODO
    }
	catch (...) {
		return;
	}
}

void Block::placeBuildings()
{
    for (auto lotIt = mLots.begin(); lotIt != mLots.end(); ++lotIt) {
        lotIt->place(Building());
    }
}