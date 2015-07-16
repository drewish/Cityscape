#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"

#include "cinder/Camera.h"
#include "cinder/gl/gl.h"

#include "Mode.h"
#include "RoadNetwork.h"
#include "Block.h"
#include "BuildingPlan.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class CityscapeApp : public App {
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

    CameraPersp mCamera;
    ModeRef mModeRef;
    ci::params::InterfaceGlRef mParams;
};

void prepareSettings( App::Settings *settings )
{
    settings->setHighDensityDisplayEnabled();
}

//void buildSkyMesh( ci::gl::VboMeshRef &skyMesh )
//{
//    // Sky
//    vector<vec3> positions;
//    float y = 1500;
//    float minX = -200, maxX = 800;
//    float minZ = 0, midZ = 50, maxZ = 200;
//
//    vector<Color> colors;
//    Color darkBlue = Color8u(108, 184, 251);
//    Color medBlue = Color8u(160, 212, 250);
//    Color lightBlue = Color8u(174, 214, 246);
//
//    positions.push_back( vec3( maxX, y, maxZ ) );
//    positions.push_back( vec3( minX, y, maxZ ) );
//    colors.push_back( darkBlue );
//    colors.push_back( darkBlue );
//
//    positions.push_back( vec3( minX, y, midZ ) );
//    positions.push_back( vec3( maxX, y, midZ ) );
//    positions.push_back( vec3( maxX, y, midZ ) );
//    positions.push_back( vec3( minX, y, midZ ) );
//    colors.push_back( medBlue );
//    colors.push_back( medBlue );
//    colors.push_back( medBlue );
//    colors.push_back( medBlue );
//
//    positions.push_back( vec3( minX, y, minZ ) );
//    positions.push_back( vec3( maxX, y, minZ ) );
//    colors.push_back( lightBlue );
//    colors.push_back( lightBlue );
//
//    vector<uint32_t> indices;
//    for (int i=0; i < 8; i++) {
//        indices.push_back(i);
//    }
//
//    gl::VboMesh::Layout layout;
//    layout.setStaticIndices();
//    layout.setStaticPositions();
//    layout.setStaticColorsRGB();
//    skyMesh = gl::VboMesh::create(indices.size(), positions.size(), layout, GL_QUADS);
//    skyMesh->bufferIndices(indices);
//    skyMesh->bufferPositions(positions);
//    skyMesh->bufferColorsRGB(colors);
//}

void CityscapeApp::setup()
{
    mParams = params::InterfaceGl::create( "App parameters", ivec2( 180, 300 ) );
//    mParams->maximize( false );

    resize();

    setupModeParams();
    mParams->minimize();

    mModeRef = ModeRef( new BuildingMode() );
    mModeRef->setup();
    mModeRef->addParams( mParams );

    layout();
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
    mParams->addButton( "Block Mode", [&] {
        mParams->clear();
        setupModeParams();

        mModeRef = ModeRef( new BlockMode() );
        mModeRef->setup();
        mModeRef->addParams( mParams );
    }, "key=w" );
    mParams->addButton( "Building Mode", [&] {
        mParams->clear();
        setupModeParams();

        mModeRef = ModeRef( new BuildingMode() );
        mModeRef->setup();
        mModeRef->addParams( mParams );
    }, "key=e" );
    mParams->addSeparator();
}

void CityscapeApp::resize()
{
    mCamera.setPerspective( 40.0f, getWindowAspectRatio(), 300.0f, 2000.0f );
    mCamera.lookAt( vec3( 320, -360, 400 ), vec3(320, 240, 0), vec3( 0, 1, 0 ) );
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
    if (!mModeRef) return;

    float u = ((float) event.getX()) / getWindowWidth();
    float v = ((float) (getWindowHeight() - event.getY())) / getWindowHeight();
    Ray r = mCamera.generateRay(u, v, mCamera.getAspectRatio());
    float result = 0.0f;
    vec3 point;
    if (r.calcPlaneIntersection(glm::zero<ci::vec3>(), vec3( 0, 0, 1 ), &result)) {
        point = r.calcPosition(result);
        mModeRef->addPoint( vec2( point.x, point.y ) );
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

    if (mModeRef) mModeRef->draw();

    gl::disableDepthWrite();
    gl::disableDepthRead();

	mParams->draw();
}


CINDER_APP( CityscapeApp, RendererGl( RendererGl::Options().msaa( 16 ) ), prepareSettings )
