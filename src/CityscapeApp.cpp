#include "cinder/app/AppNative.h"
#include "cinder/Camera.h"
#include "cinder/ConvexHull.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"

#include "CinderCGAL.h"
#include "FlatShape.h"
#include "RoadNetwork.h"
#include "Building.h"
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

    RoadNetwork     mRoads;
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
        mRoads.clear();
        mRoads.addPoints({
            Vec2f(133,41),
            Vec2f(143,451),
            Vec2f(143,451),
            Vec2f(495,424),
            Vec2f(491,421),
            Vec2f(370,254),
            Vec2f(377,262),
            Vec2f(529,131),
        });
    }, "key=1" );
    mParams->addButton( "Test 2", [&] {
        mRoads.clear();
        mRoads.addPoints({
            Vec2f(133,41),
            Vec2f(143,451),
            Vec2f(143,451),
            Vec2f(495,424),
            Vec2f(491,421),
            Vec2f(370,254),
            Vec2f(377,262),
            Vec2f(529,131),
            Vec2f(131,47),
            Vec2f(523,132),
        });
    }, "key=2" );
    mParams->addButton( "Test 3", [&] {
        mRoads.clear();
        mRoads.addPoints({
            Vec2f(119.284,17.3257),
            Vec2f(301.294,1226.4),
            Vec2f(301.294,1226.4),
            Vec2f(546.399,74.1908),
            Vec2f(544.513,79.3862),
            Vec2f(118.603,19.5102),
        });
    }, "key=3" );
    mParams->addButton( "Test 4", [&] {
        mRoads.addPoints({
            Vec2f(163.104,60.2898),
            Vec2f(306.353,918.302),
            Vec2f(306.353,918.302),
            Vec2f(490.026,113.687),
            Vec2f(490.026,113.687),
            Vec2f(163.104,60.2898),
        });
    }, "key=4" );
    mParams->addButton( "Test 5", [&] {
        mRoads.clear();
        mRoads.addPoints({
            Vec2f(0.8666,1108.26),
            Vec2f(158.236,41.0269),
            Vec2f(159.075,44.556),
            Vec2f(313.45,0.94),
            Vec2f(313.45,0.94),
            Vec2f(408.625,90.0115),
            Vec2f(408.625,90.0115),
            Vec2f(331.941,319.65),
            Vec2f(331.941,319.65),
            Vec2f(313.635,1054.66),
            Vec2f(313.635,1054.66),
            Vec2f(0.1429,1069.64),
        });
    }, "key=5" );
    mParams->addButton( "Clear Points", [&] { mRoads.clear(); }, "key=0" );

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
	mRoads.addPoint( pos );
//    console() << "mRoads.addPoint(Vec2f(" << pos.x << "," << pos.y << "));\n";
}

void CityscapeApp::layout()
{
    mRoads.layout();
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

    mRoads.draw( mOptions );

	mParams->draw();
}

CINDER_APP_NATIVE( CityscapeApp, RendererGl )
