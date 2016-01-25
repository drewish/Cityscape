#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"

#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/GeomIo.h"
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
    void setup();
    void setupModeParams();
    void buildBackground();

    void mouseMove( MouseEvent event );
    void mouseDown( MouseEvent event );
    void resize();
    void update();

    void draw();

    void layout();

  protected:
    CameraPersp         mCamera;
    CameraUi            mCamUi;

    ModeRef             mModeRef;
    ci::params::InterfaceGlRef mParams;
    ci::gl::BatchRef    mSkyBatch;
    ci::gl::BatchRef    mGroundBatch;
    ci::vec3            mCenter = vec3( 320, 240, 0 );
};

// TODO: Consider making this a general purpose gradient generator.
void CityscapeApp::buildBackground()
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

    geom::Plane plane = geom::Plane()
        .size( vec2( 1200 ) )
        .origin( mCenter - vec3( 0, 0, 0.01 ) )
        .axes( vec3( 1, 0, 0 ), vec3( 0, 1, 0 ) );

    mGroundBatch = gl::Batch::create( plane, shader );
}

void CityscapeApp::setup()
{
    // Create params ahead of CameraUI so it gets first crack at the signals
    mParams = params::InterfaceGl::create( "Cityscape", ivec2( 350, 700 ) );
    mParams->minimize();
    setupModeParams();

    mCamera.setPerspective( 40.0f, getWindowAspectRatio(), 100.0f, 2000.0f );
    mCamera.lookAt( vec3( 320, -360, 180 ), mCenter, vec3( 0, 1, 0 ) );
    mCamera.setWorldUp( vec3( 0, 0, 1 ) );
    mCamUi = CameraUi( &mCamera, getWindow() );

    buildBackground();

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
    mCamera.setAspectRatio( getWindowAspectRatio() );
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
// Disabling until I figure out how to toggle between view and edit mode
return;
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
// Disabling until I figure out how to toggle between view and edit mode
return;
    if (mModeRef) mModeRef->mMousePos = event.getPos();
}

void CityscapeApp::draw()
{
    gl::clear( Color::white() );
    {
        // Fill the screen with our sky... at some point it should probably
        // become a skybox since the gradient moves witht the camera right now.
        vec2 window = getWindowSize();
        gl::ScopedMatrices matrixScope;

        gl::setMatricesWindow( window );
        gl::translate( window.x / 2.0, 0.5 * window.y );
        gl::scale( window.x, window.y, 1 );

        mSkyBatch->draw();
    }

    {
        gl::ScopedBlendAlpha scopedAlpha;
        gl::ScopedDepth depthScope(true);
        gl::ScopedMatrices matrixScope;
        gl::setMatrices( mCamera );

        {
            gl::ScopedColor scopedColor( Color8u(233, 203, 151) );
            mGroundBatch->draw();
        }

        if (mModeRef) mModeRef->draw();
    }

	mParams->draw();
}

void prepareSettings( App::Settings *settings )
{
    settings->setHighDensityDisplayEnabled();
}

CINDER_APP( CityscapeApp, RendererGl( RendererGl::Options().msaa( 16 ) ), prepareSettings )
