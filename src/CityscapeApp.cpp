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
#include "VehicleMode.h"

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
    // keep track of time so physics can be independent of frame rate
    double mCurrentSeconds;

    CameraPersp         mEditCamera;
    CameraUi            mEditCameraUI;
    CameraPersp         mViewCamera;
    CameraUi            mViewCameraUI;
    bool                mIsEditing = true;

    ModeRef             mModeRef;
    ci::params::InterfaceGlRef mParams;
    // These names aren't great but it's for seeing where the mouse would
    // intersect with the ground plane.
    bool                mIsMouseOnPlane;
    ci::vec2            mMouseOnPlaneAt;
    ci::vec2            mMouseMoveFrom;
    enum action { ACTION_ADD, ACTION_HOVER, ACTION_MOVE, ACTION_PAN };
    action              mEditAction;
};

void CityscapeApp::setup()
{
    setWindowSize( 800, 600 );

    mParams = params::InterfaceGl::create( "Cityscape", ivec2( 350, 700 ) );
    mParams->minimize();
    setupModeParams( ModeRef( new BlockMode() ) );

    mEditCamera.setPerspective( 60.0f, getWindowAspectRatio(), 10.0f, 4000.0f );
    mEditCamera.lookAt( vec3( 0, 0, 1000 ), vec3( 0 ), vec3( 0, 1, 0 ) );
    mEditCameraUI = CameraUi( &mEditCamera );
    mViewCameraUI.enable( mIsEditing );

    mViewCamera.setPerspective( 40.0f, getWindowAspectRatio(), 10.0f, 4000.0f );
    mViewCamera.lookAt( vec3( 0, -600, 200 ), vec3( 0, 0, 0 ), vec3( 0, 0, 1 ) );
    mViewCameraUI = CameraUi( &mViewCamera );
    mViewCameraUI.enable( ! mIsEditing );

    // Track current time so we can calculate elapsed time.
    mCurrentSeconds = getElapsedSeconds();
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
    mParams->addButton( "Vehicle Mode", [this] {
        setupModeParams( ModeRef( new VehicleMode() ) );
    }, "key=r" );

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
	// Calculate elapsed time.
    double elapsed = getElapsedSeconds() - mCurrentSeconds;
    mCurrentSeconds += elapsed;

    if ( mModeRef ) mModeRef->update( elapsed );
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
        gl::ScopedMatrices matrixScope;
        if ( mIsEditing ) {
            gl::ScopedColor scopedColor( Color::white() );
            gl::setMatrices( mEditCamera );

            if ( mModeRef ) {
                mModeRef->draw();

                PolyLine2f hoverOutline;
                if ( mModeRef->isOverOutline( mMouseOnPlaneAt, hoverOutline ) ) {
                    gl::draw( hoverOutline );
                }

                if ( mModeRef->getPoints().size() ) {
                    for ( const vec2 &p : mModeRef->getPoints() ) {
                        gl::drawSolidCircle( p, 8.0f, 12 );
                    }
                    // Draw a second outline for the last point since additions will
                    // follow it.
                    gl::drawStrokedCircle( mModeRef->getPoints().back(), 12.0f, 4.0f, 12 );
                }
            }

            drawCursor();

        } else {
            gl::setMatrices( mViewCamera );
            if ( mModeRef ) mModeRef->draw();
        }
    }

    // Little indicator for when the FPS drops too low
    if ( App::getAverageFps() < 50.0 ) {
        gl::ScopedColor scopedColor( Color( 1, 0, 0 ) );
        gl::ScopedModelMatrix scopedMatrix;

        gl::translate( vec2( getWindowWidth() - 50, 50 ) );
        gl::rotate( M_PI / 8.0 );
        gl::drawSolidCircle( vec2( 0 ), 25.0, 8 );
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
