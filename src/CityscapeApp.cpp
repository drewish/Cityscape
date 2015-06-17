#include "cinder/app/AppNative.h"
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"

#include "Mode.h"
#include "RoadNetwork.h"
#include "Block.h"
#include "BuildingPlan.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class CityscapeApp : public AppNative {
  public:
    void prepareSettings( Settings *settings );

    void setup();
    void setupModeParams();

    void mouseMove( MouseEvent event );
    void mouseDown( MouseEvent event );
    void resize();
    void update();

    void draw();

    void layout();

    CameraPersp     mCamera;
    ModeRef         mModeRef;
    ci::params::InterfaceGlRef  mParams;
    ci::gl::VboMesh         mSkyMesh;
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
//    mParams->maximize( false );

    setupModeParams();

    mModeRef = ModeRef( new CityMode() );
    mModeRef->setup();
    mModeRef->addParams( mParams );

    resize();
    layout();

    buildSkyMesh( mSkyMesh );
}

void CityscapeApp::setupModeParams()
{
    mParams->addButton( "City Mode", [&] {
        mParams->clear();
        setupModeParams();

        mModeRef = ModeRef( new CityMode() );
        mModeRef->setup();
        mModeRef->addParams( mParams );
    }, "key=q" );
    mParams->addButton( "Building Mode", [&] {
        mParams->clear();
        setupModeParams();

        mModeRef = ModeRef( new BuildingMode() );
        mModeRef->setup();
        mModeRef->addParams( mParams );
    }, "key=w" );
    mParams->addSeparator();
}

void CityscapeApp::resize()
{
    mCamera.setPerspective( 40.0f, getWindowAspectRatio(), 300.0f, 2000.0f );
    mCamera.lookAt( Vec3f( 320, -360, 400 ), Vec3f(320, 240, 0), Vec3f::yAxis() );
}

void CityscapeApp::update()
{
}

void CityscapeApp::layout()
{
    if (mModeRef) mModeRef->layout();
}

void CityscapeApp::mouseDown( MouseEvent event )
{
// TODO only accept clicks in city mode

    float u = ((float) event.getX()) / getWindowWidth();
    float v = ((float) (getWindowHeight() - event.getY())) / getWindowHeight();
    Ray r = mCamera.generateRay(u, v, mCamera.getAspectRatio());
    float result = 0.0f;
    Vec3f point;
    if (r.calcPlaneIntersection(Vec3f::zero(), Vec3f::zAxis(), &result)) {
        point = r.calcPosition(result);
//        console() << "Vec2f(" << point.x << "," << point.y << "),\n";
//FIXME:      mRoads.addPoint( point.xy() );
    }
}

void CityscapeApp::mouseMove( MouseEvent event )
{
    if (mModeRef) mModeRef->mMousePos = event.getPos();
}

void CityscapeApp::draw()
{
	gl::clear( Color( 0.9, 0.9, 0.9 ) );
	gl::enableAlphaBlending();
    gl::enableDepthRead();
    gl::enableDepthWrite();

    gl::setMatrices( mCamera );

    gl::draw(mSkyMesh);

    if (mModeRef) mModeRef->draw();

    gl::disableDepthWrite();
    gl::disableDepthRead();

	mParams->draw();
}


CINDER_APP_NATIVE( CityscapeApp, RendererGl )
