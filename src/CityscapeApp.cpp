#include "cinder/app/AppNative.h"
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/params/Params.h"
#include "cinder/Rand.h"

#include "Options.h"
#include "Resources.h"
#include "RoadNetwork.h"

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
    ci::gl::VboMesh mSkyMesh;

    gl::GlslProgRef	mBuildingShader;
    params::InterfaceGlRef  mParams;

    Vec2i mMousePos;
};

void CityscapeApp::prepareSettings( Settings *settings )
{
    settings->enableHighDensityDisplay();
}

void buildSkyMesh( ci::gl::VboMesh &skyMesh )
{
    // Sky
    vector<Vec3f> positions;
    float y = 1500;
    float minX = -200, maxX = 800;
    float minZ = 0, midZ = 50, maxZ = 200;

    vector<Color> colors;
    Color darkBlue = Color8u(108, 184, 251);
    Color medBlue = Color8u(160, 212, 250);
    Color lightBlue = Color8u(174, 214, 246);

    positions.push_back( Vec3f( maxX, y, maxZ ) );
    positions.push_back( Vec3f( minX, y, maxZ ) );
    colors.push_back( darkBlue );
    colors.push_back( darkBlue );

    positions.push_back( Vec3f( minX, y, midZ ) );
    positions.push_back( Vec3f( maxX, y, midZ ) );
    positions.push_back( Vec3f( maxX, y, midZ ) );
    positions.push_back( Vec3f( minX, y, midZ ) );
    colors.push_back( medBlue );
    colors.push_back( medBlue );
    colors.push_back( medBlue );
    colors.push_back( medBlue );

    positions.push_back( Vec3f( minX, y, minZ ) );
    positions.push_back( Vec3f( maxX, y, minZ ) );
    colors.push_back( lightBlue );
    colors.push_back( lightBlue );

    vector<uint32_t> indices;
    for (int i=0; i < 8; i++) {
        indices.push_back(i);
    }

    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();
    layout.setStaticColorsRGB();
    skyMesh = gl::VboMesh(indices.size(), positions.size(), layout, GL_QUADS);
    skyMesh.bufferIndices(indices);
    skyMesh.bufferPositions(positions);
    skyMesh.bufferColorsRGB(colors);
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
            Vec2f(-391.031,1191.03),
            Vec2f(1026.58,1173.85),
            Vec2f(1026.58,1173.85),
            Vec2f(538.783,-52.5473),
            Vec2f(538.783,-52.5473),
            Vec2f(103.206,-48.1886),
            Vec2f(103.206,-48.1886),
            Vec2f(-391.031,1191.03),
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

//    try {
        mOptions.buildingShader = gl::GlslProg::create( loadResource( RES_VERT ), loadResource( RES_FRAG ) );
//    }
//    catch( gl::GlslProgCompileExc &exc ) {
//        console() << "Shader compile error: " << std::endl;
//        console() << exc.what();
//    }
//    catch( ... ) {
//        console() << "Unable to load shader" << std::endl;
//    }

    resize();
    layout();

    buildSkyMesh( mSkyMesh );
}

void CityscapeApp::resize()
{
    mCamera.setPerspective( 40.0f, getWindowAspectRatio(), 300.0f, 2000.0f );
    mCamera.lookAt( Vec3f( 320, -360, 400 ), Vec3f(320, 240, 0), Vec3f::yAxis() );
}

void CityscapeApp::update()
{
}

void CityscapeApp::addPoint(Vec2f pos)
{
	mRoads.addPoint( pos );
    console() << "Vec2f(" << pos.x << "," << pos.y << "),\n";
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
    gl::enableDepthRead();
    gl::enableDepthWrite();

    gl::setMatrices( mCamera );

    gl::draw(mSkyMesh);

    mRoads.draw( mOptions );

    gl::disableDepthWrite();
    gl::disableDepthRead();

	mParams->draw();
}

CINDER_APP_NATIVE( CityscapeApp, RendererGl )
