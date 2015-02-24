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

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/create_straight_skeleton_2.h>
#include <CGAL/exceptions.h>



typedef CGAL::Polygon_2<K>           Polygon_2;


ci::Vec2f vecFrom(K::Point_2 p)
{
	return ci::Vec2f(p.x(), p.y());
}

K::Point_2 pointFrom(ci::Vec2f p)
{
	return K::Point_2(p.x, p.y);
}

void Block::draw()
{
    gl::color( ColorA( 0.0f, 0.8f, 0.2f, 0.5f ) );
//    gl::draw( outline );
//
//    for( auto itL = lots.begin(); itL != lots.end(); ++itL ) {
//        itL->draw();
//    }

    drawSkeleton(mSkel);

}

void Block::drawSkeleton(const SsPtr &ss)
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
        gl::color( Color( 1.0-col,0,col ) );
        col += 0.1;
        gl::drawSolid( line );
        gl::color( Color( 1,0,0 ) );
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
    gl::color( Color( 0,1,0 ) );
    gl::draw( outline );
    gl::drawSolidCircle(vecFrom(begin->vertex()->point()), 2);

}

void Block::subdivide()
{
    Polygon_2 poly;

    // Assume the outline is closed and first == last.
    // Don't want to bother with less than a triangle.
    if (outline.size() < 4) return;

    // start at +1 to avoid overlapping last point
	for ( auto i = --outline.end(); i != outline.begin(); --i ) {
		poly.push_back(pointFrom(*i));
	}


    try {
        mSkel = CGAL::create_interior_straight_skeleton_2(poly);

        lots.clear();

        for( auto face = mSkel->faces_begin(); face != mSkel->faces_end(); ++face ) {

            PolyLine2f lotOutline;
            Ss::Halfedge_handle edge, start;

            start = face->halfedge();
            edge = start;
            do {
                lotOutline.push_back(vecFrom(edge->vertex()->point()));
                edge = edge->next();
            } while (edge != start);

            Lot l = Lot(lotOutline);
            lots.push_back(l);
        }
    }
    catch (CGAL::Precondition_exception e) {
        return; // TODO
    }

	std::vector<ci::PolyLine2f > ret;
}