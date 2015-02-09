#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class CityscapeApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void CityscapeApp::setup()
{
}

void CityscapeApp::mouseDown( MouseEvent event )
{
}

void CityscapeApp::update()
{
}

void CityscapeApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( CityscapeApp, RendererGl )
