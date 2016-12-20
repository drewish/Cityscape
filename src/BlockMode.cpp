#include "BlockMode.h"
#include "CgalArrangement.h"
#include "CgalStraightSkeleton.h"
#include "GeometryHelpers.h"
#include "FlatShape.h"

#include "RoadBuilder.h"
#include "BlockSubdivider.h"
#include "LotFiller.h"

using namespace ci;
using namespace ci::app;

void BlockMode::setup() {
    mViewOptions.drawBlocks = false;
    mViewOptions.drawLots = true;
    mViewOptions.drawLotEdges = true;
    mViewOptions.drawBuildings = true;

    mModel = Cityscape::CityModel();

    mPlan = mModel.zoningPlans.front();

    FlatShapeRef fs = FlatShape::create( PolyLine2f( {
        vec2( -600, -600 ), vec2(  600, -600 ),
        vec2(  600,  600 ), vec2( -600,  600 )
    } ) );
    Cityscape::DistrictRef district = Cityscape::District::create( fs, mPlan );
    district->blocks.push_back( Cityscape::Block::create( fs ) );
    mModel.districts.push_back( district );

    layout();
}

void BlockMode::addParams( ci::params::InterfaceGlRef params) {

    params->addSeparator("Block");

    params->addParam( "Lot Division", {"None", "Skeleton", "OOB"}, (int*)&mPlan->block.lotDivision )
        .updateFn( [this] { requestLayout(); } );
    params->addParam( "Lot Width", &mPlan->block.lotWidth ).step( 5 )
        .min( 10 ).max( 400 ).updateFn( [this] { requestLayout(); } );
    params->addParam( "Lot Area Min", &mPlan->block.lotAreaMin ).step( 100 )
        .min( 100 ).max( 1000000 ).updateFn( [this] { requestLayout(); } );
    params->addParam( "Lot Area Max", &mPlan->block.lotAreaMax ).step( 1000 )
        .min( 10000 ).max( 1000000 ).updateFn( [this] { requestLayout(); } );

// TODO: should alter zoning
//    params->addSeparator("Lot");
//
//    params->addParam( "Placement", {"Center", "Fill"}, (int*)&mOptions.lot.buildingPlacement )
//        .updateFn( [this] { requestLayout(); } );

    params->addSeparator();

    params->addParam( "Draw Roads",    &mViewOptions.drawRoads,     "key=a" );
    params->addParam( "Draw District", &mViewOptions.drawDistricts, "key=s" );
    params->addParam( "Draw Block",    &mViewOptions.drawBlocks,    "key=d" );
    params->addParam( "Draw Lot",      &mViewOptions.drawLots,      "key=f" );
    params->addParam( "Draw Plants",   &mViewOptions.drawPlants,    "key=z" );
    params->addParam( "Draw Building", &mViewOptions.drawBuildings, "key=x" );

    params->addButton( "Clear Points", [&] {
        mOutline = PolyLine2f();
        mHoles = {};
        layout();
    }, "key=0");
    params->addButton( "Test 1", [&] {
        mOutline = PolyLine2f({
            vec2( -153, -213 ),
            vec2( -143, 197 ),
            vec2( 209, 170 ),
            vec2( 84, 0 ),
            vec2( 243, -123 ),
            vec2( -153, -213 ),
        });
        mHoles = {};
        layout();
    }, "key=1" );
    params->addButton( "Test 2", [&] {
        mOutline = PolyLine2f({
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
        mHoles = {};
        layout();
    }, "key=2" );
    params->addButton( "Test 3", [&] {
        mOutline = PolyLine2f({
            vec2(-9.6225,498.446),
            vec2(-519.615,-336.788),
            vec2(533.087,-159.734),
            vec2(-9.6225,498.446),
        });
        mHoles = {
            PolyLine2f({
                vec2( -154, -213 ),
                vec2( -144, 197 ),
                vec2( 208, 170 ),
                vec2( 83, 0 ),
                vec2( 242, -123 ),
                vec2( -154, -213 ),
            })
        };
        layout();
    }, "key=3" );
}

void BlockMode::layout() {
    mCachedPoints.clear();

    auto district = mModel.districts.front();
    district->blocks.clear();
    district->blocks.push_back( Cityscape::Block::create( FlatShape::create( mOutline, mHoles ) ) );

    Cityscape::subdivideBlocks( mModel );
    Cityscape::fillLots( mModel );

    mCityView = CityView::create( mModel );
}

void BlockMode::draw() {
    if ( mCityView ) mCityView->draw( mViewOptions );

    // * * *
/*
    // Visually debug a CGAL arrangement.
    const Arrangement_2 arr = mArr;

    gl::color( 1, 1, 0 );
    for ( auto i = arr.vertices_begin(); i != arr.vertices_end(); ++i ) {
        gl::ScopedColor color( Color::gray( 0.5 ) );
        gl::drawSolidCircle( vecFrom( i->point() ), 5 );
    }

    gl::color(0, 0, 0 );
    for ( auto i = arr.edges_begin(); i != arr.edges_end(); ++i ) {
        gl::ScopedColor color( Color::white() );
        PolyLine2f p = PolyLine2f( { vecFrom( i->source()->point() ), vecFrom( i->target()->point() ) } );
        gl::draw( p );
    }

    float steps = 0;
    for ( auto face = arr.faces_begin(); face != arr.faces_end(); ++face ) {
        if ( face->is_unbounded() ) continue;

        gl::color( ColorA( CM_HSV, steps, 1.0, 0.75, 0.25 ) );
        steps += 0.27;
        if (steps > 1) steps -= 1.0;

        gl::drawSolid( polyLineFrom( face->outer_ccb() ) );
    }

    // * * *

    // Visually debug a CGAL straight skeleton.
    SsPtr &ss = mSkel;
    float hue = 0;

    for( auto face = ss->faces_begin(); face != ss->faces_end(); ++face ) {
        PolyLine2f shape;

        Ss::Halfedge_const_handle begin = face->halfedge();
        Ss::Halfedge_const_handle edge = begin;
        do {
            vec2 p = vecFrom( edge->vertex()->point() );

            if ( edge->vertex()->is_skeleton() ) {
                gl::color( Color::black() );
            } else if ( edge->vertex()->is_contour() ) {
                gl::color( Color::white() );
            } else {
                gl::color( Color( 0, 1, 1 ) );
            }
            gl::drawSolidCircle( p, 5 );

            shape.push_back( p );
            edge = edge->next();
        } while ( edge != begin );

        gl::color( ColorA( CM_HSV, hue, 1.0, 0.75, 0.25 ) );
        hue += 0.17;
        if (hue > 1) hue -= 1.0;
        gl::drawSolid( shape );

        gl::color( Color( 1, 0, 0 ) );
        gl::draw( shape );
    }
*/
}

std::vector<ci::vec2> BlockMode::getPoints()
{
    if ( mCachedPoints.empty() ) {
        for ( auto &hole : mHoles ) {
            std::copy( hole.begin(), hole.end(), std::back_inserter( mCachedPoints ) );
        }
        // Put the outline in last since it assume we build off the last point
        std::copy( mOutline.begin(), mOutline.end(), std::back_inserter( mCachedPoints ) );
    }

    return mCachedPoints;
}

void BlockMode::addPoint( ci::vec2 point ) {
    console() << "vec2(" << point.x << "," << point.y << "),\n";
    mOutline.push_back( point );

    layout();
}

bool BlockMode::isOverMovablePoint( ci::vec2 &point, float margin )
{
    for ( const auto &other : mCachedPoints ) {
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
    for ( auto &p : mOutline ) {
        if ( p == from ) { p = to; }
    }
    for ( auto &hole : mHoles ) {
        for ( auto &p : hole ) {
            if ( p == from ) { p = to; }
        }
    }

    layout();
}
