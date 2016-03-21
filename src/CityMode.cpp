#include "CityMode.h"
#include "CityData.h"
#include "FlatShape.h"
#include "RoadBuilder.h"
#include "BlockSubdivider.h"
#include "LotFiller.h"

using namespace ci;
using namespace ci::app;

void CityMode::setup() {
    mViewOptions.drawBlocks = false;
    mViewOptions.drawLots = false;
    mViewOptions.drawBuildings = true;

    layout();
}

void CityMode::addParams( ci::params::InterfaceGlRef params) {

    Cityscape::ZoningPlanRef plan = mModel.zoningPlans.front();

    params->addSeparator("Road");

    params->addParam( "Highway Width", &mModel.highwayWidth )
        .min( 10 ).max( 50 ).step( 1 ).updateFn( [this] { requestLayout(); } );
    params->addParam( "Sidestreet Width", &plan->district.grid.roadWidth )
        .min( 10 ).max( 50 ).step( 1 ).updateFn( [this] { requestLayout(); } );

    params->addSeparator("District");

    params->addParam( "Street Division", {"None", "Grid"}, (int*)&plan->district.streetDivision )
        .updateFn( [this] { requestLayout(); } );
    params->addParam( "Avenue Angle", &plan->district.grid.avenueAngle )
        .min( -180 ).max( 180 ).step( 5 ).updateFn( [this] { requestLayout(); } );
    params->addParam( "Street Angle", &plan->district.grid.streetAngle )
        .min( -90 ).max( 90 ).step( 15 ).updateFn( [this] { requestLayout(); } );
    params->addParam( "Avenue Spacing", &plan->district.grid.avenueSpacing ).step( 5 )
        .min( 15 ).max( 400 ).updateFn( [this] { requestLayout(); } );
    params->addParam( "Street Spacing", &plan->district.grid.streetSpacing ).step( 5 )
        .min( 15 ).max( 400 ).updateFn( [this] { requestLayout(); } );

    params->addSeparator("Block");

    params->addParam( "Lot Division", {"None", "Skeleton"}, (int*)&plan->block.lotDivision )
        .updateFn( [this] { requestLayout(); } );
    params->addParam( "lotWidth", &plan->block.lotWidth ).step( 5 )
        .min( 10 ).max( 400 ).updateFn( [this] { requestLayout(); } );

    params->addSeparator();

    params->addParam( "Draw Roads",    &mViewOptions.drawRoads,     "key=a" );
    params->addParam( "Draw District", &mViewOptions.drawDistricts, "key=s" );
    params->addParam( "Draw Block",    &mViewOptions.drawBlocks,    "key=d" );
    params->addParam( "Draw Lot",      &mViewOptions.drawLots,      "key=f" );
    params->addParam( "Draw Trees",    &mViewOptions.drawTrees,     "key=z" );
    params->addParam( "Draw Building", &mViewOptions.drawBuildings, "key=x" );

    params->addSeparator();

    params->addButton( "Clear Points", [&] {
        mHighwayPoints.clear();
        layout();
    }, "key=0" );
    params->addButton( "Test 1", [&] {
        mHighwayPoints = std::vector<ci::vec2>({
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
        mHighwayPoints = std::vector<ci::vec2>({
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
        std::vector<ci::vec2> add({
            vec2(-9.6225,498.446),
            vec2(-519.615,-336.788),
            vec2(-519.615,-336.788),
            vec2(533.087,-159.734),
            vec2(533.087,-159.734),
            vec2(-9.6225,498.446),
        });
        mHighwayPoints.insert( mHighwayPoints.end(), add.begin(), add.end() );
        layout();
    }, "key=3" );
    params->addButton( "Test 4", [&] {
        // Intentionally don't clear so we can combine with other shapes
        std::vector<ci::vec2> add({
            vec2( -576, 575 ),
            vec2( 573, 572 ),
            vec2( 573, 572 ),
            vec2( 573, -569 ),
            vec2( 573, -569 ),
            vec2( -573, -578 ),
            vec2( -573, -578 ),
            vec2( -576, 575 ),
        });
        mHighwayPoints.insert( mHighwayPoints.end(), add.begin(), add.end() );
        layout();
    }, "key=4" );
    params->addButton( "Test 5", [&] {
        mHighwayPoints = std::vector<ci::vec2>({
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
    // Translate from RoadNetwork into Highways
    mModel.highways.clear();
    for ( size_t i = 1, size = mHighwayPoints.size(); i < size; i += 2 ) {
        mModel.highways.push_back( Cityscape::Highway::create( mHighwayPoints[i - 1], mHighwayPoints[i] ) );
    }

    // TODO: Should have better way to partially update
    // figure out how to mark progress so we can do this across a few
    // frame updates instead of blocking
    Cityscape::buildHighwaysAndDistricts( mModel );
    Cityscape::buildStreetsAndBlocks( mModel );
    Cityscape::subdivideBlocks( mModel );
    Cityscape::fillLots( mModel );

    mCityView = CityView::create( mModel );
}

void CityMode::draw() {
    if ( mCityView ) mCityView->draw( mViewOptions );
}

std::vector<ci::vec2> CityMode::getPoints()
{
    return mHighwayPoints;
}

void CityMode::addPoint( ci::vec2 point )
{
    console() << "vec2(" << point.x << "," << point.y << "),\n";
    mHighwayPoints.push_back( point );
    layout();
}

bool CityMode::isOverMovablePoint( ci::vec2 &point, float margin )
{
    for ( const auto &other : mHighwayPoints ) {
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
    for ( const auto &p : mHighwayPoints ) {
        newPoints.push_back( from == p ? to : p );
    }
    mHighwayPoints = newPoints;
    layout();
}

bool CityMode::isOverOutline( const ci::vec2 &point, ci::PolyLine2f &outline ) {
    for ( const auto &district : mModel.districts ) {
        if ( ! district->shape->contains( point ) ) continue;

        for ( const auto &block : district->blocks ) {
            if ( block->shape->contains( point ) ) {
                outline = block->shape->outline();
                return true;
            }
        }
    }
    return false;
}
