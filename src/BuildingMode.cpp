#include "BuildingMode.h"
#include "FlatShape.h"

using namespace ci;
using namespace ci::app;


void BuildingMode::setup() {
    mViewOptions.drawBuildings = true;

    mModel = Cityscape::CityModel();
    FlatShapeRef fs = FlatShape::create( PolyLine2f( {
        vec2( -600, -600 ), vec2(  600, -600 ),
        vec2(  600,  600 ), vec2( -600,  600 )
    } ) );
    Cityscape::DistrictRef district = Cityscape::District::create( fs, mModel.zoningPlans.front() );
    Cityscape::BlockRef    block    = Cityscape::Block::create( fs );
    Cityscape::LotRef      lot      = Cityscape::Lot::create( fs );
    mModel.districts.push_back( district );
    district->blocks.push_back( block );
    block->lots.push_back( lot );

    mOutline = polyLineLShape();
}

void BuildingMode::addParams( params::InterfaceGlRef params ) {
    params->addParam( "Roof", BuildingPlan::roofStyleNames(), (int*)&mRoof )
        .keyDecr( "[" ).keyIncr( "]" )
        .updateFn( [this] { requestLayout(); } );
    params->addParam( "Roof Slope", &mRoofSlope )
        .min( 0.0 ).max( 5.0 ).step( 0.05 )
        .updateFn( [this] { requestLayout(); } );
    params->addParam( "Roof Overhang", &mRoofOverhang )
        .min( 0.0 ).max( 5.0 ).step( 0.25 )
        .updateFn( [this] { requestLayout(); } );
    params->addParam( "Floors", &mFloors)
        .min( 0 ).max( 5 )
        .keyDecr( "-" ).keyIncr( "=" )
        .updateFn( [this] { requestLayout(); } );

    params->addSeparator();

    params->addButton( "Clear Points", [&] {
        setOutline( PolyLine2f() );
    }, "key=0");
    params->addButton( "Square", [&] {
        setOutline( polyLineSquare() );
    }, "key=1" );
    params->addButton( "Rect", [&] {
        setOutline( polyLineRectangle( 60, 40 ) );
    }, "key=2" );
    params->addButton( "L", [&] {
        setOutline( polyLineLShape() );
    }, "key=3" );
    params->addButton( "T", [&] {
        setOutline( polyLineTee() );
    }, "key=4" );
    params->addButton( "+", [&] {
        setOutline( polyLinePlus() );
    }, "key=5" );
    params->addButton( "<", [&] {
        setOutline( polyLineTriangle() );
    }, "key=6" );
}

void BuildingMode::layout() {
    auto lot = mModel.districts.front()->blocks.front()->lots.front();
    lot->buildings.clear();

    if ( mOutline.size() < 3 ) return;

    auto plan = BuildingPlan::create( mOutline, mFloors, mRoof, mRoofSlope, mRoofOverhang );
    lot->buildings.push_back( plan->createInstace( ci::vec2( 0 ) ) );
    mCityView = CityView::create( mModel );
}

void BuildingMode::setOutline( const ci::PolyLine2f &outline ) {
    mOutline = outline;
    requestLayout();
}

std::vector<ci::vec2> BuildingMode::getPoints()
{
    return mOutline.getPoints();
}

void BuildingMode::addPoint( ci::vec2 point ) {
    // Don't add on to a closed outline
    if ( mOutline.isClosed() ) return;

    console() << "vec2(" << point.x << "," << point.y << "),\n";
    mOutline.push_back( point );
    requestLayout();
}

bool BuildingMode::isOverMovablePoint( ci::vec2 &point, float margin )
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

void BuildingMode::movePoint( ci::vec2 from, ci::vec2 to )
{
    for ( auto &p : mOutline ) {
        if ( p == from ) { p = to; }
    }

    requestLayout();
}
