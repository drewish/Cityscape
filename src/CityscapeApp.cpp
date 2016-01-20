#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"

#include "cinder/Camera.h"
#include "cinder/gl/gl.h"

#include "Mode.h"
#include "CityMode.h"
#include "BlockMode.h"
#include "BuildingMode.h"

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
    void buildSky();

    void mouseMove( MouseEvent event );
    void mouseDown( MouseEvent event );
    void resize();
    void update();

    void draw();

    void layout();

    CameraPersp mCamera;
    ModeRef mModeRef;
    ci::params::InterfaceGlRef mParams;
    ci::gl::BatchRef mSkyBatch;
    ci::vec3 mCenter = vec3(320, 240, 0);
};

// TODO: Consider making this a general purpose gradient generator.
void CityscapeApp::buildSky()
{
    vector<vec3> positions;
    vector<Color> colors;
    Color darkBlue = Color8u(108, 184, 251);
    Color medBlue = Color8u(160, 212, 250);
    Color lightBlue = Color8u(174, 214, 246);

    positions.push_back( vec3( +0.5, -0.5, +0.0 ) );
    positions.push_back( vec3( -0.5, -0.5, +0.0 ) );
    colors.push_back( darkBlue );
    colors.push_back( darkBlue );
    positions.push_back( vec3( +0.5, -0.2, +0.0 ) );
    positions.push_back( vec3( -0.5, -0.2, +0.0 ) );
    colors.push_back( medBlue );
    colors.push_back( medBlue );
    positions.push_back( vec3( +0.5, +0.1, +0.0 ) );
    positions.push_back( vec3( -0.5, +0.1, +0.0 ) );
    colors.push_back( medBlue );
    colors.push_back( medBlue );
    positions.push_back( vec3( +0.5, +0.5, +0.0 ) );
    positions.push_back( vec3( -0.5, +0.5, +0.0 ) );
    colors.push_back( lightBlue );
    colors.push_back( lightBlue );

    vector<gl::VboMesh::Layout> bufferLayout = {
        gl::VboMesh::Layout().usage( GL_STATIC_DRAW ).attrib( geom::Attrib::POSITION, 3 ),
        gl::VboMesh::Layout().usage( GL_STATIC_DRAW ).attrib( geom::Attrib::COLOR, 3 ),
    };
    gl::VboMeshRef mesh = gl::VboMesh::create( positions.size(), GL_TRIANGLE_STRIP, bufferLayout );
    mesh->bufferAttrib( geom::Attrib::POSITION, positions );
    mesh->bufferAttrib( geom::Attrib::COLOR, colors );

    gl::GlslProgRef shader = gl::getStockShader( gl::ShaderDef().color() );

    mSkyBatch = gl::Batch::create( mesh, shader );
}

void CityscapeApp::setup()
{
    mParams = params::InterfaceGl::create( "App parameters", ivec2( 350, 700 ) );

    resize();

    setupModeParams();
    mParams->minimize();

    buildSky();

    mModeRef = ModeRef( new CityMode() );
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
    mCamera.lookAt( vec3( 320, -360, 180 ), mCenter, vec3( 0, 1, 0 ) );
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
    // Clear with our ground color...
    gl::clear( Color8u(233, 203, 151) );
    {
        // ...then draw our sky up top.
        gl::ScopedMatrices matrixScope;
        vec2 window = getWindowSize();
        gl::setMatricesWindow( window );

        gl::translate( window.x / 2.0, 0.125 * window.y );
        gl::scale( window.x, 0.25 * window.y, 1 );
        mSkyBatch->draw();
    }

    gl::enableAlphaBlending();
    gl::ScopedDepth depthScope(true);
    gl::ScopedMatrices matrixScope;
    gl::setMatrices( mCamera );


    if (mModeRef) mModeRef->draw();

	mParams->draw();
}

void prepareSettings( App::Settings *settings )
{
    settings->setHighDensityDisplayEnabled();
}

CINDER_APP( CityscapeApp, RendererGl( RendererGl::Options().msaa( 16 ) ), prepareSettings )
