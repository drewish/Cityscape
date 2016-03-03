#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"

#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/GeomIo.h"
#include "cinder/gl/gl.h"

#include "CityData.h"

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
    void setupModeParams( ModeRef mode );
    void buildBackground();

    void keyUp( KeyEvent event );
    void mouseMove( MouseEvent event );
    void mouseDown( MouseEvent event );
    void mouseUp( MouseEvent event );
    void mouseDrag( MouseEvent event );
    void mouseWheel( MouseEvent event );
    void calcMouseOnPlane( vec2 mouse );
    void resize();
    void update();

    void draw();
    void drawCursor();
    void layout();

  protected:
    CameraPersp         mEditCamera;
    CameraUi            mEditCameraUI;
    CameraPersp         mViewCamera;
    CameraUi            mViewCameraUI;
    bool                mIsEditing = true;

    ModeRef             mModeRef;
    ci::params::InterfaceGlRef mParams;
    ci::gl::BatchRef    mSkyBatch;
    ci::gl::BatchRef    mGroundBatch;
    ci::vec3            mCenter = vec3( 0, 0, 0 );
    // These names aren't great but it's for seeing where the mouse would
    // intersect with the ground plane.
    bool                mIsMouseOnPlane;
    ci::vec2            mMouseOnPlaneAt;
    ci::vec2            mMouseMoveFrom;
    enum action { ACTION_ADD, ACTION_HOVER, ACTION_MOVE, ACTION_PAN };
    action              mEditAction;
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
    positions.push_back( vec3( +0.5, +0.2, +0.0 ) );
    positions.push_back( vec3( -0.5, +0.2, +0.0 ) );
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
        .origin( mCenter - vec3( 0, 0, 0.1 ) )
        .axes( vec3( 1, 0, 0 ), vec3( 0, 1, 0 ) );

    mGroundBatch = gl::Batch::create( plane, shader );
}

void CityscapeApp::setup()
{
    setWindowSize( 800, 600 );

    mParams = params::InterfaceGl::create( "Cityscape", ivec2( 350, 700 ) );
    mParams->minimize();
    setupModeParams( ModeRef( new CityMode() ) );

    mEditCamera.setPerspective( 60.0f, getWindowAspectRatio(), 10.0f, 4000.0f );
    mEditCamera.lookAt( mCenter + vec3( 0, 0, 1000 ), mCenter, vec3( 0, 1, 0 ) );
    mEditCameraUI = CameraUi( &mEditCamera );
    mViewCameraUI.enable( mIsEditing );

    mViewCamera.setPerspective( 40.0f, getWindowAspectRatio(), 10.0f, 4000.0f );
    mViewCamera.lookAt( vec3( 0, -800, 300 ), mCenter, vec3( 0, 0, 1 ) );
    mViewCameraUI = CameraUi( &mViewCamera );
    mViewCameraUI.enable( ! mIsEditing );

    hideCursor();

    buildBackground();
}

void CityscapeApp::setupModeParams( ModeRef newMode )
{
    mParams->clear();
    mParams->addButton( "City Mode", [this] {
        setupModeParams( ModeRef( new CityMode() ) );
    }, "key=q" );
    mParams->addButton( "Block Mode", [this] {
        setupModeParams( ModeRef( new BlockMode() ) );
    }, "key=w" );
    mParams->addButton( "Building Mode", [this] {
        setupModeParams( ModeRef( new BuildingMode() ) );
    }, "key=e" );

    mModeRef = newMode;
    mModeRef->setup();
    mModeRef->addParams( mParams );

    mParams->addSeparator();
}

void CityscapeApp::resize()
{
    mEditCamera.setAspectRatio( getWindowAspectRatio() );
    mViewCamera.setAspectRatio( getWindowAspectRatio() );
}

void CityscapeApp::update()
{
}

void CityscapeApp::layout()
{
    if ( mModeRef ) mModeRef->layout();
}

void CityscapeApp::keyUp( KeyEvent event )
{
    if ( event.getCode() == KeyEvent::KEY_SPACE ) {
        mIsEditing = !mIsEditing;
        if ( mIsEditing ) {
            mViewCameraUI.disable();
            mEditCameraUI.enable();
        } else {
            mViewCameraUI.enable();
            mEditCameraUI.disable();
        }
    } else if ( event.getCode() == KeyEvent::KEY_TAB ) {
        mParams->maximize( ! mParams->isMaximized() );
    }
}

void CityscapeApp::mouseMove( MouseEvent event )
{
    // Don't bother computing the hover point when viewing.
    if ( ! mIsEditing || ! mModeRef ) return;

    calcMouseOnPlane( event.getPos() );

    // If this is true it'll snap to the point it's over
    // TODO: The "margin" param should be coupled to the cursor size
    if ( mModeRef->isOverMovablePoint( mMouseOnPlaneAt, 10.0f ) ) {
        mEditAction = ACTION_HOVER;
    } else {
        mEditAction = ACTION_ADD;
    }
}

void CityscapeApp::mouseDown( MouseEvent event )
{
    mViewCameraUI.mouseDown( event );

    if ( ! mIsEditing || ! mModeRef ) return;

    if ( mEditAction == ACTION_HOVER ) {
        mMouseMoveFrom = mMouseOnPlaneAt;
    } else {
        mEditCameraUI.mouseDown( event.getPos() );
    }
}

void CityscapeApp::mouseDrag( MouseEvent event )
{
    mViewCameraUI.mouseDrag( event );

    if ( ! mIsEditing || ! mModeRef ) return;

    calcMouseOnPlane( event.getPos() );

    // We only want panning if they're not moving a point.
    if ( mEditAction == ACTION_HOVER ) {
        mEditAction = ACTION_MOVE;
    } else if ( mEditAction == ACTION_MOVE ) {
        // ?
    } else {
        mEditAction = ACTION_PAN;
        mEditCameraUI.mouseDrag( event.getPos(), false, true, false );
    }
}

void CityscapeApp::mouseUp( MouseEvent event )
{
    mViewCameraUI.mouseUp( event );

    if ( ! mIsEditing ) return;

    switch ( mEditAction ) {
        case ACTION_HOVER:
        case ACTION_ADD:
            mModeRef->addPoint( mMouseOnPlaneAt );
            mEditAction = ACTION_HOVER;
            break;
        case ACTION_MOVE:
            // TODO: This minimum drag distance should be coupled to the cursor size
            if ( glm::distance2( mMouseMoveFrom, mMouseOnPlaneAt ) > 10.0 * 10.0 ) {
                mModeRef->movePoint( mMouseMoveFrom, mMouseOnPlaneAt );
                mEditAction = ACTION_HOVER;
            } else {
                mEditAction = ACTION_ADD;
            }
            break;
        case ACTION_PAN:
            mEditCameraUI.mouseUp( event.getPos() );
            mEditAction = ACTION_ADD;
            break;
    }
}

void CityscapeApp::mouseWheel( MouseEvent event )
{
    mViewCameraUI.mouseWheel( event );
    mEditCameraUI.mouseWheel( event );
}

void CityscapeApp::calcMouseOnPlane( vec2 mouse )
{
    float u = mouse.x / (float) getWindowWidth();
    float v = ( getWindowHeight() - mouse.y ) / (float) getWindowHeight();
    Ray ray = mEditCamera.generateRay( u, v, mEditCamera.getAspectRatio() );
    float distance = 0.0f;
    if ( ! ray.calcPlaneIntersection( glm::zero<ci::vec3>(), vec3( 0, 0, 1 ), &distance ) ) {
        mIsMouseOnPlane = false;
        return;
    }
    vec3 intersection = ray.calcPosition( distance );
    mMouseOnPlaneAt = vec2( intersection.x, intersection.y );
    mIsMouseOnPlane = true;
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
        if ( mIsEditing ) {
            gl::ScopedColor scopedColor( Color::white() );
            gl::setMatrices( mEditCamera );

            if ( mModeRef && mModeRef->getPoints().size() ) {
                for ( const vec2 &p : mModeRef->getPoints() ) {
                    gl::drawSolidCircle( p, 8.0f, 12 );
                }
                // Draw a second outline for the last point since additions will
                // follow it.
                gl::drawStrokedCircle( mModeRef->getPoints().back(), 12.0f, 4.0f, 12 );
            }

            drawCursor();
        } else {
            gl::setMatrices( mViewCamera );
        }

        {
            gl::ScopedColor scopedColor( Color8u(233, 203, 151) );
            mGroundBatch->draw();
        }

        if ( mModeRef ) mModeRef->draw();
    }

	mParams->draw();
}

void CityscapeApp::drawCursor()
{
    gl::ScopedModelMatrix scopedMatrix;

    gl::translate( mMouseOnPlaneAt );

    gl::scale( vec2( 4.0 ) );

    switch ( mEditAction ) {
        case ACTION_MOVE:
            gl::drawSolidCircle( vec2( 0 ), 2.0f, 12 );
            break;
        case ACTION_ADD:
            gl::drawSolidRect( Rectf( vec2( -1, 4 ), vec2( 1, -4 ) ) );
            gl::drawSolidRect( Rectf( vec2( -4, 1 ), vec2( 4, -1 ) ) );
            break;
        case ACTION_HOVER:
            gl::drawSolidCircle( vec2( 0 ), 2.0f, 12 );
            // no break
        case ACTION_PAN:
            // TODO: should avoid this second scaling and just size the
            // triangles to match everything else.
            gl::scale( vec2( 1.5 ) );

            gl::drawSolidTriangle( vec2( 0,  5 ), vec2( -2,  3 ), vec2(  2,  3 ) );
            gl::drawSolidTriangle( vec2( 0, -5 ), vec2(  2, -3 ), vec2( -2, -3 ) );
            gl::drawSolidTriangle( vec2(  5, 0 ), vec2(  3, -2 ), vec2(  3,  2 ) );
            gl::drawSolidTriangle( vec2( -5, 0 ), vec2( -3,  2 ), vec2( -3, -2 ) );
            break;
    }
}

void prepareSettings( App::Settings *settings )
{
    settings->setHighDensityDisplayEnabled();
}

CINDER_APP( CityscapeApp, RendererGl( RendererGl::Options().msaa( 16 ) ), prepareSettings )
