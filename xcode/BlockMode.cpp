#include "BlockMode.h"
#include "CgalArrangement.h"
#include "CgalStraightSkeleton.h"
#include "GeometryHelpers.h"

using namespace ci;
using namespace ci::app;

// This is hacky but we only use one instance of it at a time.
Arrangement_2 mArr;
std::vector<vec2> mDividers;

void BlockMode::setup() {
    mOptions.drawBlocks = false;
    mOptions.drawLots = true;
    mOptions.drawBuildings = true;

    layout();
}

void BlockMode::addParams( ci::params::InterfaceGlRef params) {
    params->addParam( "Roads", &mOptions.drawRoads, "key=a" );
    params->addParam( "Block", &mOptions.drawBlocks, "key=s" );
    params->addParam( "Lot", &mOptions.drawLots, "key=d" );
    params->addParam( "Building", &mOptions.drawBuildings, "key=f" );

    params->addButton( "Clear Points", [&] {
        mOutline = PolyLine2f();
        layout();
    }, "key=0");
    params->addButton( "Test 1", [&] {
        mOutline = PolyLine2f({
            vec2(133,41),
            vec2(143,451),
            vec2(495,424),
            vec2(370,254),
            vec2(529,131),
        });
        layout();
    }, "key=1" );
    params->addButton( "Test 2", [&] {
        mOutline = PolyLine2f({
            vec2(133,41),
            vec2(143,451),
            vec2(495,424),
            vec2(370,254),
            vec2(529,131),
            vec2(133,41), // The difference is this point to close the loop
        });
        layout();
    }, "key=2" );
    params->addButton( "Test 3", [&] {
        mOutline = PolyLine2f({
            vec2(179.795,607.902),
            vec2(174.921,200.486),
            vec2(405.803,358.815),
            vec2(388.601,79.3861),
            vec2(193.129,146.753),
            vec2(216.225,10.8599),
            vec2(205.467,-41.0754),
            vec2(467.135,-22.5028),
            vec2(516.892,456.916),
            vec2(179.795,607.902),
        });
        layout();
    }, "key=3" );
}

void BlockMode::addPoint( ci::vec2 point ) {
    console() << "vec2(" << point.x << "," << point.y << "),\n";
    mOutline.push_back( point );
    layout();
}

void BlockMode::layout() {
    mBlock = Block::create( mOutline );

//    mDividers = computeDividers( mOutline.getPoints(), mAngle );
    // TODO: Need to split this functionality in the class out into more functions
    mArr = mBlock->arrangementSubdividing( mBlock->mShape, mOptions.block.lotWidth );
}

void BlockMode::draw() {
    gl::color( 1, 0, 1 );
    assert( mDividers.size() % 2 == 0 );
    for ( auto i = mDividers.begin(); i != mDividers.end(); ++i) {
        gl::drawLine( *i, *++i );
    }

    gl::color( 1, 1, 0 );
    for ( auto i = mArr.vertices_begin(); i != mArr.vertices_end(); ++i ) {
        gl::drawSolidCircle( vecFrom( i->point() ), 5 );
    }

    gl::color(0, 0, 0 );
    for ( auto i = mArr.edges_begin(); i != mArr.edges_end(); ++i ) {
        PolyLine2f p = PolyLine2f({ vecFrom( i->source()->point() ), vecFrom( i->target()->point() ) } );
        gl::draw( p );
    }

    float steps = 0;
//    std::cout << "\n\n------\nfaces: " << mArr.number_of_faces() << std::endl;
    for ( auto i = mArr.faces_begin(); i != mArr.faces_end(); ++i ) {
//        std::cout << "\tunbounded: " << i->is_unbounded() << " fictitious: " << i->is_fictitious() << std::endl;
//        std::cout << "\touter_ccbs:" << i->number_of_outer_ccbs() << " holes: " << i->number_of_holes() << std::endl;
        /*
         for ( auto j = i->holes_begin(); j != i->holes_end(); ++j ) {
         PolyLine2f faceOutline;
         Arrangement_2::Ccb_halfedge_circulator cc = *j;
         do {
         Arrangement_2::Halfedge_handle he = cc;
         faceOutline.push_back( vecFrom( he->target()->point() ) );
         } while ( ++cc != *j );

         gl::color( ColorA( CM_HSV, steps, 1.0, 0.75, 0.5 ) );
         steps += 0.17;
         if (steps > 1) steps -= 1.0;
         std::cout << "\t\t" << num << faceOutline << "\n";
         gl::drawSolid( faceOutline );
         }
         */

        for ( auto j = i->outer_ccbs_begin(); j != i->outer_ccbs_end(); ++j ) {
            PolyLine2f faceOutline;
            Arrangement_2::Ccb_halfedge_circulator cc = *j;
            faceOutline.push_back( vecFrom( cc->source()->point() ) );
            do {
                Arrangement_2::Halfedge_handle he = cc;
                faceOutline.push_back( vecFrom( he->target()->point() ) );
            } while ( ++cc != *j );

            gl::color( ColorA( CM_HSV, steps, 1.0, 0.75, 0.5 ) );
            steps += 0.27;
            if (steps > 1) steps -= 1.0;
//            std::cout << "\t\t" << num << ": " << faceOutline << "\n";
            gl::drawSolid( faceOutline );
        }
    }
}
