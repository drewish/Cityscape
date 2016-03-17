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

    setOutline( BuildingPlan::lshape() );
}

void BuildingMode::addParams( params::InterfaceGlRef params ) {
    params->addParam( "Roof", BuildingPlan::roofStyleNames(), (int*)&mOptions.building.roofStyle )
        .keyDecr( "[" ).keyIncr( "]" )
        .updateFn( [this] { requestLayout(); } );
    params->addParam( "Floors", &mFloors)
        .min( 1 ).max( 5 )
        .keyDecr( "-" ).keyIncr( "=" )
        .updateFn( [this] { requestLayout(); } );

    params->addSeparator();

    params->addButton( "Clear Points", [&] {
        setOutline( PolyLine2f() );
    }, "key=0");
    params->addButton( "Square", [&] {
        setOutline( BuildingPlan::square() );
    }, "key=1" );
    params->addButton( "Rect", [&] {
        setOutline( BuildingPlan::rectangle( 60, 40 ) );
    }, "key=2" );
    params->addButton( "L", [&] {
        setOutline( BuildingPlan::lshape() );
    }, "key=3" );
    params->addButton( "T", [&] {
        setOutline( BuildingPlan::tee() );
    }, "key=4" );
    params->addButton( "+", [&] {
        setOutline( BuildingPlan::plus() );
    }, "key=5" );
    params->addButton( "<", [&] {
        setOutline( BuildingPlan::triangle() );
    }, "key=6" );
}

void BuildingMode::layout() {
    mBuilding.reset();

    if ( mOutline.size() < 3 ) return;

// TODO: need to ensure the outline is in counterclockwise order

    mBuilding = Cityscape::Building::create( BuildingPlan( mOutline, static_cast<BuildingPlan::RoofStyle>( mOptions.building.roofStyle ) ), mFloors );

    mModel.districts.front()->blocks.front()->lots.front()->building = mBuilding;
    mCityView = CityView::create( mModel );
}

void BuildingMode::draw() {
    if ( mCityView ) mCityView->draw( mViewOptions );
}

void BuildingMode::setOutline( const ci::PolyLine2f &outline ) {
    mOutline = outline;
    layout();
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
    layout();
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
    PolyLine2f newOutline;
    for ( const auto &p : mOutline ) {
        newOutline.push_back( from == p ? to : p );
    }
    newOutline.setClosed( mOutline.isClosed() );

    mOutline = newOutline;

    layout();
}
