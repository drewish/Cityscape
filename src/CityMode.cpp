#include "CityMode.h"
#include "CityData.h"
#include "FlatShape.h"
#include "ZoningPlanner.h"
#include "RoadBuilder.h"
#include "BlockSubdivider.h"
#include "LotFiller.h"
#include "GeometryHelpers.h"

using namespace ci;
using namespace ci::app;


void CityMode::setup()
{
    mViewOptions.drawBlocks = false;
    mViewOptions.drawLots = false;
    mViewOptions.drawBuildings = true;

    mModel.zoningPlans = {
        Cityscape::zoneFarming(),
        Cityscape::zoneMajesticHeights(),
        Cityscape::zoneDowntown()
    };

    layout();
}

void CityMode::addParams( ci::params::InterfaceGlRef params)
{
    Cityscape::ZoningPlanRef plan = mModel.zoningPlans.front();

    // Hacky but hopefully good enough for now.
    params->addButton( "New Road", [this] {
        mHighways.push_back( PolyLine2f() );
        isAddingRoad = true;
    }, "key=n" );
    params->addButton( "Finish Road", [this] {
        isAddingRoad = false;
    }, "key=ESC" );

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

    params->addParam( "Lot Division", {"None", "Skeleton", "OOB"}, (int*)&plan->block.lotDivision )
        .updateFn( [this] { requestLayout(); } );
    params->addParam( "Lot Width", &plan->block.lotWidth ).step( 5 )
        .min( 10 ).max( 400 ).updateFn( [this] { requestLayout(); } );
    params->addParam( "Lot Area Min", &plan->block.lotAreaMin ).step( 100 )
        .min( 100 ).max( 100000 ).updateFn( [this] { requestLayout(); } );
    params->addParam( "Lot Area Max", &plan->block.lotAreaMax ).step( 1000 )
        .min( 1000 ).max( 1000000 ).updateFn( [this] { requestLayout(); } );

    params->addSeparator();

    params->addParam( "Draw Roads",    &mViewOptions.drawRoads,     "key=a" );
    params->addParam( "Draw District", &mViewOptions.drawDistricts, "key=s" );
    params->addParam( "Draw Block",    &mViewOptions.drawBlocks,    "key=d" );
    params->addParam( "Draw Lot",      &mViewOptions.drawLots,      "key=f" );
    params->addParam( "Draw Plants",   &mViewOptions.drawPlants,    "key=z" );
    params->addParam( "Draw Building", &mViewOptions.drawBuildings, "key=x" );

    params->addSeparator();

    params->addButton( "Clear Highways", [&] {
        mHighways.clear();
        layout();
    }, "key=0" );
    params->addButton( "Test 1", [&] {
        mHighways = std::vector<PolyLine2>( {
            PolyLine2( {
                vec2( -154, -213 ),
                vec2( -144, 197 ),
                vec2( 208, 170 ),
                vec2( 83, 0 ),
                vec2( 242, -123 ),
            } ),
        } );
        layout();
    }, "key=1" );
    params->addButton( "Test 2", [&] {
        mHighways = std::vector<PolyLine2>( {
            PolyLine2( {
                vec2( -154, -213 ),
                vec2( -144, 197 ),
                vec2( 208, 170 ),
                vec2( 83, 0 ),
                vec2( 242, -123 ),
            } ),
            PolyLine2( {
                vec2( -156, -207 ),
                vec2( 236, -122 ),
            } ),
        } );
        layout();
    }, "key=2" );
    params->addButton( "Test 3", [&] {
        // Intentionally don't clear so we can combine with other shapes
        mHighways.push_back( PolyLine2( {
            vec2( -9.6225, 498.446 ),
            vec2( -519.615,-336.788 ),
            vec2( 533.087,-159.734 ),
            vec2( -9.6225,498.446 ),
        } ) );
        layout();
    }, "key=3" );
    params->addButton( "Test 4", [&] {
        // Intentionally don't clear so we can combine with other shapes
        mHighways.push_back( PolyLine2( {
            vec2( -576, 575 ),
            vec2( 573, 572 ),
            vec2( 573, -569 ),
            vec2( -573, -578 ),
            vec2( -576, 575 ),
        } ) );
        layout();
    }, "key=4" );
    params->addButton( "Test 5", [&] {
        mHighways = std::vector<PolyLine2>( {
            PolyLine2( {
                vec2( -206.133, 539.26 ),
                vec2( -48.764, -527.973 ),
                vec2( 106.45, -568.06 ),
                vec2( 201.625, -478.988 ),
                vec2( 124.941, -249.35 ),
                vec2( 106.635, 485.66 ),
                vec2( -206.857, 500.64 ),
            } ),
        } );
        layout();
    }, "key=5" );
}

void CityMode::layout() {
    mModel.highways.clear();
    for ( auto line : mHighways ) {
        // Translate from PolyLines into Highways... would be nice if highways
        // weren't just simple segments :/
        pointsInPairs<vec2>( line,
            [&](const vec2 &a, const vec2 &b) {
                mModel.highways.push_back( Cityscape::Highway::create( a, b ) );
            }
        );
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

std::vector<ci::vec2> CityMode::getPoints()
{
    std::vector<vec2> points;
    auto inserter( std::back_inserter( points ) );

    for ( auto &line : mHighways ) {
        std::copy( line.begin(), line.end(), inserter );
    }

    return points;
}

void CityMode::addPoint( ci::vec2 point )
{
    if ( isAddingRoad ) {
        console() << "vec2(" << point.x << "," << point.y << "),\n";
        mHighways.back().push_back( point );
        mLayoutNeeded = true;
    }
}

bool CityMode::isOverMovablePoint( ci::vec2 &point, float margin )
{
    for ( const auto &other : getPoints() ) {
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
    if ( isAddingRoad ) {
        return;
    }

    for ( auto &highway : mHighways ) {
        for ( auto &p : highway ) {
            if ( p == from ) {
                p = to;
                mLayoutNeeded = true;
            }
        }
    }
}

bool CityMode::isOverOutline( const ci::vec2 &point, ci::PolyLine2f &outline )
{
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
