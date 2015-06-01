/*
 Next steps:
 - Create a roadways class
 - City: sub-divide large block with more streets (i guess start with a manhattan grid?)
 - Lot: scale building to fit free space
 - Building: optimize mesh building and rendering. migh need a floorplan class
   so we only build the mesh once. if we had separate roof and wall meshes we 
   could scale walls based on height.
 - Lot: randomly choose building outlines, roof style, height
 - Building: expand roof outline to overlap house
 - Block: use skeleton to find propertly line then subdivide lots.
 - Building: put roofs on buildings:
     - gabled
     - gambrel
     - shed
 - Lot: mark portions of lot that face a road
 - Lot: orient buildings toward street
 */

#include "cinder/app/AppNative.h"
#include "cinder/Camera.h"
#include "cinder/ConvexHull.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"

#include "CinderCGAL.h"
#include <CGAL/Polygon_set_2.h>
#include "FlatShape.h"
#include "Road.h"
#include "Building.h"
#include "Lot.h"
#include "Block.h"
#include "Options.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class CityscapeApp : public AppNative {
  public:
    void prepareSettings( Settings *settings );
    void setup();

    void mouseMove( MouseEvent event );
    void mouseDown( MouseEvent event );
    void mouseDrag( MouseEvent event );
    void resize();
    void update();
    void draw();

    void addPoint(Vec2f);
    void layout();

    Options         mOptions;

    CameraPersp     mCamera;

    vector<Vec2f>   mPoints;
    PolyLine2f      mDivider;
    vector<Road>    mRoads;
    vector<Block>   mBlocks;
    ci::gl::VboMesh mMesh;

    params::InterfaceGlRef  mParams;

    Vec2i mMousePos;
};

void CityscapeApp::prepareSettings( Settings *settings )
{
    settings->enableHighDensityDisplay();
}

void CityscapeApp::setup()
{
    mParams = params::InterfaceGl::create( "App parameters", Vec2i( 180, 300 ) );
    mParams->addParam( "Roads", &mOptions.drawRoads, "key=a" );
    mParams->addParam( "Block", &mOptions.drawBlocks, "key=s" );
    mParams->addParam( "Lot", &mOptions.drawLots, "key=d" );
    mParams->addParam( "Building", &mOptions.drawBuildings, "key=f" );
    mParams->addParam( "Clip City", &mOptions.clipCityLimit, "key=c" );
    mParams->addButton( "Test 1", [&] {
        mPoints.clear();
        mPoints.push_back(Vec2f(133,41));
        mPoints.push_back(Vec2f(143,451));
        mPoints.push_back(Vec2f(143,451));
        mPoints.push_back(Vec2f(495,424));
        mPoints.push_back(Vec2f(491,421));
        mPoints.push_back(Vec2f(370,254));
        mPoints.push_back(Vec2f(377,262));
        mPoints.push_back(Vec2f(529,131));
        layout();
    }, "key=1" );
    mParams->addButton( "Test 2", [&] {
        mPoints.clear();
        mPoints.push_back(Vec2f(133,41));
        mPoints.push_back(Vec2f(143,451));
        mPoints.push_back(Vec2f(143,451));
        mPoints.push_back(Vec2f(495,424));
        mPoints.push_back(Vec2f(491,421));
        mPoints.push_back(Vec2f(370,254));
        mPoints.push_back(Vec2f(377,262));
        mPoints.push_back(Vec2f(529,131));
        mPoints.push_back(Vec2f(131,47));
        mPoints.push_back(Vec2f(523,132));
        layout();
    }, "key=2" );
    mParams->addButton( "Test 3", [&] {
        mPoints.clear();
        mPoints.push_back(Vec2f(119.284,17.3257));
        mPoints.push_back(Vec2f(301.294,1226.4));
        mPoints.push_back(Vec2f(301.294,1226.4));
        mPoints.push_back(Vec2f(546.399,74.1908));
        mPoints.push_back(Vec2f(544.513,79.3862));
        mPoints.push_back(Vec2f(118.603,19.5102));
        layout();
    }, "key=3" );
    mParams->addButton( "Test 4", [&] {
        mPoints.push_back(Vec2f(163.104,60.2898));
        mPoints.push_back(Vec2f(306.353,918.302));
        mPoints.push_back(Vec2f(306.353,918.302));
        mPoints.push_back(Vec2f(490.026,113.687));
        mPoints.push_back(Vec2f(490.026,113.687));
        mPoints.push_back(Vec2f(163.104,60.2898));
        layout();
    }, "key=4" );
    mParams->addButton( "Test 5", [&] {
        mPoints.clear();
        mPoints.push_back(Vec2f(0.8666,1108.26));
        mPoints.push_back(Vec2f(158.236,41.0269));
        mPoints.push_back(Vec2f(159.075,44.556));
        mPoints.push_back(Vec2f(313.45,0.94));
        mPoints.push_back(Vec2f(313.45,0.94));
        mPoints.push_back(Vec2f(408.625,90.0115));
        mPoints.push_back(Vec2f(408.625,90.0115));
        mPoints.push_back(Vec2f(331.941,319.65));
        mPoints.push_back(Vec2f(331.941,319.65));
        mPoints.push_back(Vec2f(313.635,1054.66));
        mPoints.push_back(Vec2f(313.635,1054.66));
        mPoints.push_back(Vec2f(0.1429,1069.64));
        layout();
    }, "key=5" );
    mParams->addButton( "Clear Points", [&] { mPoints.clear(); layout(); }, "key=0" );

    resize();
    layout();

    // For some fucking reason i have to build and render a mesh in this class
    // to be able to have the buildings render without dying....
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();
    int quadCount = 1;
    int vertCount = quadCount * 4;
    mMesh = gl::VboMesh(vertCount, quadCount * 4, layout, GL_QUADS);
    vector<uint32_t> indices;
    for (int i=0; i < vertCount; i++) {
        indices.push_back(i);
    }
    mMesh.bufferIndices(indices);
    vector<Vec3f> positions;
    positions.push_back( Vec3f( -0.5,  0.5, -0.5 ) );
    positions.push_back( Vec3f(  0.5,  0.5, -0.5 ) );
    positions.push_back( Vec3f(  0.5, -0.5, -0.5 ) );
    positions.push_back( Vec3f( -0.5, -0.5, -0.5 ) );
    mMesh.bufferPositions(positions);
}

void CityscapeApp::resize()
{
    mCamera.setPerspective( 40.0f, getWindowAspectRatio(), 300.0f, 2000.0f );
    mCamera.lookAt( Vec3f( 320, 240 - 600, 400.0f ), Vec3f(320, 240,0.0), Vec3f::yAxis() );
}

void CityscapeApp::update()
{
}

void CityscapeApp::addPoint(Vec2f pos)
{
	mPoints.push_back( pos );
//    console() << "mPoints.push_back(Vec2f(" << pos.x << "," << pos.y << "));\n";
	layout();
}

void buildHighways(const vector<Vec2f> &points, vector<Road> &roads)
{
    float roadWidth = 20.0;
    for( uint i = 1, size = points.size(); i < size; i += 2 ) {
        Road road(points[i-1], points[i], roadWidth);
        roads.push_back(road);
    }
}

void buildSideStreets(vector<Road> &roads)
{
}

void buildBlocks()
{
}

void CityscapeApp::layout()
{
    mRoads.clear();
    mBlocks.clear();

    CGAL::Polygon_set_2<ExactK> roadways, unpaved;

    buildHighways( mPoints, mRoads );

    // Add some secondary streets
    //    buildSideStreets( mRoads );
    if ( mRoads.size() != 0 ) {
        // - find bounding box for roads
        auto r = mRoads.begin();
        Rectf bounds( r->bounds() );
        for ( ++r; r != mRoads.end(); ++r ) {
            bounds.include( r->bounds() );
        }

        // - create narrow roads to cover the bounding box
        float roadWidth = 10.0;
        for ( float x = bounds.x1; x < bounds.x2; x += 50 ) {
            Road road( Vec2f( x, bounds.y1 ), Vec2f( x, bounds.y2 ), roadWidth);
            mRoads.push_back( road );
//            roadways.join( polygonFrom<ExactK>( road.outline ) );
        }
        for ( float y = bounds.y1; y < bounds.y2; y += 100 ) {
            Road road( Vec2f( bounds.x1, y ), Vec2f( bounds.x2, y ), roadWidth);
            mRoads.push_back( road );
//            roadways.join( polygonFrom<ExactK>( road.outline ) );
        }

        // - make roadway orientation configurable globally

        // - make roadway orientation configurable at the block level (meaning we
        //   run this for each block)

    }

    for ( auto r = mRoads.begin(); r != mRoads.end(); ++r ) {
        roadways.join( polygonFrom<ExactK>( r->outline ) );
    }
    unpaved.complement(roadways);

    if (mOptions.clipCityLimit) {
        Vec2i windowSize = getWindowSize();
        CGAL::Polygon_2<ExactK> window;
        window.push_back( ExactK::Point_2( 0, 0 ) );
        window.push_back( ExactK::Point_2( windowSize.x, 0 ) );
        window.push_back( ExactK::Point_2( windowSize.x, windowSize.y ) );
        window.push_back( ExactK::Point_2( 0, windowSize.y ) );
        
        unpaved.intersection(window); // Intersect with the clipping rectangle.
    }

    unsigned int block_id = 0;
    std::list<CGAL::Polygon_with_holes_2<ExactK>> res;
    unpaved.polygons_with_holes( std::back_inserter( res ) );
    for ( auto it = res.begin(); it != res.end(); ++it ) {
        CGAL::Polygon_with_holes_2<ExactK> pwh = *it;

        if ( pwh.is_unbounded() ) {
            console() << "Polygon is unbounded...\n";
            continue;
        }
        FlatShape fs(pwh);

        console() << "Block: " << block_id << std::endl;

        Block b( block_id++, fs );
        mBlocks.push_back(b);
    }


    for( auto it = mRoads.begin(); it != mRoads.end(); ++it ) {
        it->layout();
    }
    for( auto it = mBlocks.begin(); it != mBlocks.end(); ++it ) {
        it->layout();
    }
}

void CityscapeApp::mouseDown( MouseEvent event )
{
    float u = ((float) event.getX()) / getWindowWidth();
    float v = ((float) (getWindowHeight() - event.getY())) / getWindowHeight();
    Ray r = mCamera.generateRay(u, v, mCamera.getAspectRatio());
    float result = 0.0f;
    Vec3f point;
    if (r.calcPlaneIntersection(Vec3f::zero(), Vec3f::zAxis(), &result)) {
        point = r.calcPosition(result);
        addPoint( point.xy() );
    }
}

void CityscapeApp::mouseDrag( MouseEvent event )
{
}

void CityscapeApp::mouseMove( MouseEvent event )
{
    mMousePos = event.getPos();
}

void CityscapeApp::draw()
{
	gl::clear( Color( 0.9, 0.9, 0.9 ) );
	gl::enableAlphaBlending();

    gl::setMatrices( mCamera );

    gl::draw(mMesh);

	for( auto it = mRoads.begin(); it != mRoads.end(); ++it ) {
        it->draw( mOptions );
	}

	for( auto block = mBlocks.begin(); block != mBlocks.end(); ++block ) {
        block->draw( mOptions );
        for( auto lot = block->mLots.begin(); lot != block->mLots.end(); ++lot ) {
            lot->draw( mOptions );
        }
    }

	mParams->draw();
}

CINDER_APP_NATIVE( CityscapeApp, RendererGl )
