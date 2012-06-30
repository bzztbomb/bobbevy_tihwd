#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Surface.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/params/Params.h"

#include "CinderOpenCV.h"
#include "Kinect.h"

#include "bbKinectWrapper.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class bobbevyApp : public AppBasic {
public:
	void prepareSettings( Settings* settings );
	void setup();
	void keyDown( KeyEvent event );
	void mouseDown( MouseEvent event );	
	void update();
	void draw();	
private:
	KinectWrapper mKinect;
	params::InterfaceGl mParams;
};

void bobbevyApp::prepareSettings( Settings* settings )
{
	settings->setWindowSize( 640, 480 );
}

void bobbevyApp::setup()
{
	mParams = params::InterfaceGl("bobbevy", Vec2i(225, 200));	
	mKinect.setup(mParams);
}

void bobbevyApp::keyDown( KeyEvent event )
{
	switch( event.getChar() ){
		case 'f':
			setFullScreen( !isFullScreen() );
			break;
	}			
	mKinect.keyDown(event);
}

void bobbevyApp::mouseDown( MouseEvent event )
{
}

void bobbevyApp::update()
{
	mKinect.update();
}

void bobbevyApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
	mKinect.draw();
	
	// draw interface
	params::InterfaceGl::draw();	
}

CINDER_APP_BASIC( bobbevyApp, RendererGl )
