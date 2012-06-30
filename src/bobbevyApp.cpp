#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class bobbevyApp : public AppBasic {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void bobbevyApp::setup()
{
}

void bobbevyApp::mouseDown( MouseEvent event )
{
}

void bobbevyApp::update()
{
}

void bobbevyApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}


CINDER_APP_BASIC( bobbevyApp, RendererGl )
