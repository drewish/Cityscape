#include "CityMode.h"

using namespace ci;
using namespace ci::app;

void CityMode::setup() {
    mOptions.drawBlocks = false;
    mOptions.drawLots = false;
    mOptions.drawBuildings = true;

    layout();
}

void CityMode::addParams( ci::params::InterfaceGlRef params) {
    params->addSeparator();

    // TODO: Don't redo layout on every change, set a timer to update every half
    // second or so.
    params->addParam( "highwayWidth", &mOptions.road.highwayWidth )
        .min( 10 ).max( 50 ).step( 1 ).updateFn( std::bind( &CityMode::layout, this ) );
    params->addParam( "sidestreetWidth", &mOptions.road.sidestreetWidth )
        .min( 10 ).max( 50 ).step( 1 ).updateFn( std::bind( &CityMode::layout, this ) );
    params->addParam( "sidestreetAngle1", &mOptions.road.sidestreetAngle1 )
        .min( -180 ).max( 180 ).step( 5 ).updateFn( std::bind( &CityMode::layout, this ) );
    params->addParam( "sidestreetAngle2", &mOptions.road.sidestreetAngle2 )
        .min( -90 ).max( 90 ).step( 15 ).updateFn( std::bind( &CityMode::layout, this ) );
    params->addParam( "blockWidth", &mOptions.road.blockWidth ).step( 5 )
        .min( 15 ).max( 400 ).updateFn( std::bind( &CityMode::layout, this ) );
    params->addParam( "blockHeight", &mOptions.road.blockHeight ).step( 5 )
        .min( 15 ).max( 400 ).updateFn( std::bind( &CityMode::layout, this ) );

    params->addSeparator();

    params->addParam( "Division", {"None", "Divided"}, (int*)&mOptions.block.division )
        .updateFn( std::bind( &CityMode::layout, this ) );

    params->addSeparator();

    params->addParam( "lotWidth", &mOptions.block.lotWidth ).step( 5 )
        .min( 10 ).max( 400 ).updateFn( std::bind( &CityMode::layout, this ) );
    params->addParam( "Placement", {"Center", "Fill"}, (int*)&mOptions.lot.buildingPlacement )
        .updateFn( std::bind( &CityMode::layout, this ) );

    params->addSeparator();

    params->addParam( "Roof", BuildingPlan::roofStyleNames(), (int*)&mOptions.building.roofStyle )
        .keyDecr( "[" ).keyIncr( "]" )
        .updateFn( std::bind( &CityMode::layout, this ) );

    params->addSeparator();

    params->addParam( "Roads", &mOptions.drawRoads, "key=a" );
    params->addParam( "Block", &mOptions.drawBlocks, "key=s" );
    params->addParam( "Lot", &mOptions.drawLots, "key=d" );
    params->addParam( "Trees", &mOptions.drawTrees, "key=f" );
    params->addParam( "Building", &mOptions.drawBuildings, "key=g" );

    params->addSeparator();

    params->addButton( "Clear Points", [&] {
        mRoads.clear();
        layout();
    }, "key=0" );
    params->addButton( "Test 1", [&] {
        mRoads.clear();
        mRoads.addPoints({
            vec2( -154, -213 ),
            vec2( -144, 197 ),
            vec2( -144, 197 ),
            vec2( 208, 170 ),
            vec2( 204, 167 ),
            vec2( 83, 0 ),
            vec2( 90, 8 ),
            vec2( 242, -123 ),
        });
        layout();
    }, "key=1" );
    params->addButton( "Test 2", [&] {
        mRoads.clear();
        mRoads.addPoints({
            vec2( -154, -213 ),
            vec2( -144, 197 ),
            vec2( -144, 197 ),
            vec2( 208, 170 ),
            vec2( 204, 167 ),
            vec2( 83, 0 ),
            vec2( 90, 8 ),
            vec2( 242, -123 ),
            vec2( -156, -207 ),
            vec2( 236, -122 ),
        });
        layout();
    }, "key=2" );
    params->addButton( "Test 3", [&] {
        // Intentionally don't clear so we can combine with other shapes
        mRoads.addPoints({
            vec2(-9.6225,498.446),
            vec2(-519.615,-336.788),
            vec2(-519.615,-336.788),
            vec2(533.087,-159.734),
            vec2(533.087,-159.734),
            vec2(-9.6225,498.446),
        });
        layout();
    }, "key=3" );
    params->addButton( "Test 4", [&] {
        // Intentionally don't clear so we can combine with other shapes
        mRoads.addPoints({
            vec2( -586, 585 ),
            vec2( 583, 582 ),
            vec2( 583, 582 ),
            vec2( 573, -569 ),
            vec2( 573, -569 ),
            vec2( -573, -578 ),
            vec2( -573, -578 ),
            vec2( -586, 585 ),
        });
        layout();
    }, "key=4" );
    params->addButton( "Test 5", [&] {
        mRoads.clear();
        mRoads.addPoints({
            vec2( -206.133, 539.26 ),
            vec2( -48.764, -527.973 ),
            vec2( -47.925, -524.444 ),
            vec2( 106.45, -568.06 ),
            vec2( 106.45, -568.06 ),
            vec2( 201.625, -478.988 ),
            vec2( 201.625, -478.988 ),
            vec2( 124.941, -249.35 ),
            vec2( 124.941, -249.35 ),
            vec2( 106.635, 485.66 ),
            vec2( 106.635, 485.66 ),
            vec2( -206.857, 500.64 ),
        });
        layout();
    }, "key=5" );
}

void CityMode::layout() {
    mRoads.layout( mOptions );

    mCityView = CityScape::CityView::create( mRoads );
}

void CityMode::draw() {
//    mRoads.draw( mOptions );
    if ( mCityView ) mCityView->draw( mOptions );
}

std::vector<ci::vec2> CityMode::getPoints()
{
    return mRoads.getPoints();
}

void CityMode::addPoint( ci::vec2 point )
{
    console() << "vec2(" << point.x << "," << point.y << "),\n";
    mRoads.addPoint( point );
    mRoads.layout( mOptions );
}

bool CityMode::isOverMovablePoint( ci::vec2 &point, float margin )
{
    for ( const auto &other : mRoads.getPoints() ) {
        if ( length2( point - other ) < margin * margin ) {
            // Snap their point to ours
            point = other;
            return true;
        }
    }
    return false;
}

void CityMode::movePoint( ci::vec2 from, ci::vec2 to )
{
    std::vector<vec2> newPoints;
    for ( const auto &p : mRoads.getPoints() ) {
        newPoints.push_back( from == p ? to : p );
    }
    // TODO: would be nice to move this logic into the roadnetwork
    mRoads.clear();
    mRoads.addPoints( newPoints );
    mRoads.layout( mOptions );
}

