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
#include "sceneLayer.h"
#include "bbTreeLayer.h"

using namespace ci;
using namespace ci::app;
using namespace std;

static const int WIDTH = 800;
static const int HEIGHT = 600;

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
	
	SceneState mSceneState;
	TreeLayer mTreeLayer;
};

void bobbevyApp::prepareSettings( Settings* settings )
{
	settings->setWindowSize( 640, 480 );
}

void bobbevyApp::setup()
{
	mSceneState.mParams = params::InterfaceGl("bobbevy", Vec2i(225, 200));	
	mKinect.setup(mSceneState.mParams);
	
	mTreeLayer.setup(&mSceneState);
	mTreeLayer.setEnabled(true);
}

void bobbevyApp::keyDown( KeyEvent event )
{
	switch( event.getChar() ){
		case 'f':
			setFullScreen( !isFullScreen() );
			break;
		case KeyEvent::KEY_i:
			if (mSceneState.mBlackoutAmount >= 1.0)
				mTreeLayer.resetParams();
			mSceneState.mTimeline->apply(&mSceneState.mBlackoutAmount, 0.0f, 5.0f);
			break;			
	}			
	mTreeLayer.keyDown(event);	
	mKinect.keyDown(event);
}

void bobbevyApp::mouseDown( MouseEvent event )
{
}

void bobbevyApp::update()
{
	mKinect.update();
	mTreeLayer.update();
}

void bobbevyApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
	
	mTreeLayer.draw();

	mKinect.draw();
	
	// draw interface
	params::InterfaceGl::draw();	
}

CINDER_APP_BASIC( bobbevyApp, RendererGl )
