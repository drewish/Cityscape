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

#include <CGAL/exceptions.h>

void Block::setup()
{
    for( auto it = lots.begin(); it != lots.end(); ++it ) {
        it->setup();
    }
}

void Block::draw()
{
//    drawSkeleton(mSkel);

    gl::color( ColorA( 0.0f, 0.8f, 0.2f, 0.5f ) );
    gl::draw( outline );

    for( auto it = lots.begin(); it != lots.end(); ++it ) {
        it->draw();
    }
}



void Block::subdivide()
{
    // Assume the outline is closed and first == last.
    // Don't want to bother with less than a triangle.
    if (outline.size() < 4) return;

    Polygon_2 poly = polyFrom(outline);

    float steps = 0.0;

    try {
        mSkel = CGAL::create_interior_straight_skeleton_2(poly);

        lots.clear();
        lots.reserve(mSkel->size_of_faces());

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

            l.mColor = Color( 1.0-steps,0,steps );
            steps += 0.1;

            l.place(Building());
            lots.push_back(l);
        }
    }
    catch (CGAL::Precondition_exception e) {
        return; // TODO
    }
}