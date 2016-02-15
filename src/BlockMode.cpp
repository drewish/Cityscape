#include "BlockMode.h"
#include "CgalArrangement.h"
#include "CgalStraightSkeleton.h"
#include "GeometryHelpers.h"

using namespace ci;
using namespace ci::app;

void BlockMode::setup() {
    mOptions.drawBlocks = false;
    mOptions.drawLots = true;
    mOptions.drawBuildings = false;

    layout();
}

void BlockMode::addParams( ci::params::InterfaceGlRef params) {
    params->addSeparator();

    params->addParam( "Division", {"None", "Divided"}, (int*)&mOptions.block.division )
        .updateFn( std::bind( &BlockMode::layout, this ) );

    params->addSeparator();

    params->addParam( "lotWidth", &mOptions.block.lotWidth ).step( 5 )
        .min( 10 ).max( 400 ).updateFn( std::bind( &BlockMode::layout, this ) );
    params->addParam( "Placement", {"Center", "Fill"}, (int*)&mOptions.lot.buildingPlacement )
        .updateFn( std::bind( &BlockMode::layout, this ) );

    params->addSeparator();

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
            vec2( -153, -213 ),
            vec2( -143, 197 ),
            vec2( 209, 170 ),
            vec2( 84, 0 ),
            vec2( 243, -123 ),
        });
        layout();
    }, "key=1" );
    params->addButton( "Test 2", [&] {
        mOutline = PolyLine2f({
            vec2( -153, -213 ),
            vec2( -143, 197 ),
            vec2( 209, 170 ),
            vec2( 84, 0 ),
            vec2( 243, -123 ),
            vec2( -153, -213 ), // The difference is this point to close the loop
        });
        layout();
    }, "key=2" );
    params->addButton( "Test 3", [&] {
        PolyLine2f({
            vec2( -156.205, 324.902 ),
            vec2( -161.079, -82.514 ),
            vec2( 69.803, 75.815 ),
            vec2( 52.601, -203.614 ),
            vec2( -142.871, -136.247 ),
            vec2( -119.775, -272.14 ),
            vec2( -130.533, -324.075 ),
            vec2( 131.135, -305.503 ),
            vec2( 180.892, 173.916 ),
            vec2( -156.205, 324.902 ),
        });
        layout();
    }, "key=3" );
}

void BlockMode::layout() {
    mBlock.reset();
    if ( mOutline.size() < 3 ) return;

    mBlock = Block::create( mOutline );
    mBlock->layout( mOptions );
}

void BlockMode::draw() {
    if ( !mBlock ) return;

    mBlock->draw( mOptions );

    // Disabling the rest of this since it's only really helpful for debugging
    // the cgal arrangement.
    return;

    Arrangement_2 &arr = mBlock->mArr;

    gl::color( 1, 1, 0 );
    for ( auto i = arr.vertices_begin(); i != arr.vertices_end(); ++i ) {
        gl::drawSolidCircle( vecFrom( i->point() ), 5 );
    }

    gl::color(0, 0, 0 );
    for ( auto i = arr.edges_begin(); i != arr.edges_end(); ++i ) {
        PolyLine2f p = PolyLine2f({ vecFrom( i->source()->point() ), vecFrom( i->target()->point() ) } );
        gl::draw( p );
    }

    float steps = 0;
    for ( auto i = arr.faces_begin(); i != arr.faces_end(); ++i ) {
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
            gl::drawSolid( faceOutline );
        }
    }
}

void BlockMode::addPoint( ci::vec2 point ) {
    console() << "vec2(" << point.x << "," << point.y << "),\n";
    mOutline.push_back( point );
    layout();
}

bool BlockMode::isOverMovablePoint( ci::vec2 &point, float margin )
{
    for ( const auto &other : mOutline ) {
        if ( length2( point - other ) < margin * margin ) {
            // Snap their point to ours
            point = other;
            return true;
        }
    }
    return false;
}

void BlockMode::movePoint( ci::vec2 from, ci::vec2 to )
{
    PolyLine2f newOutline;
    for ( const auto &p : mOutline ) {
        newOutline.push_back( from == p ? to : p );
    }
    mOutline = newOutline;
    layout();
}
