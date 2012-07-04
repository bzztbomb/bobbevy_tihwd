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
#include "bbIntroLight.h"
#include "bbParticles.h"

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

	// Debugging
	bool mDebugDraw;
	bool mShowFPS;
	bool mShowParams;

	// Layers
	SceneState mSceneState;
	TreeLayer mTreeLayer;
	IntroLight mIntroLight;
	SkeletonParticles mCloseSwarm;
	SkeletonParticles mFarSwarm;

	gl::Texture texBlackout;
};

void bobbevyApp::prepareSettings( Settings* settings )
{
	settings->setFrameRate( 30 );
	settings->setWindowSize( WIDTH, HEIGHT );
	settings->setTitle( "bobbevy" );
	settings->enableSecondaryDisplayBlanking(false);
}

void bobbevyApp::setup()
{
	mSceneState.mParams = params::InterfaceGl("bobbevy", Vec2i(225, 200));	
	mSceneState.mParams.addParam("DebugDraw", &mDebugDraw, "keyIncr=d");
	mSceneState.mParams.addParam("ShowParams", &mShowParams, "keyIncr=p");
	mSceneState.mParams.addParam("ShowFPS", &mShowFPS, "keyIncr=m");
	
	mSceneState.mTimeline = Timeline::create();
	mSceneState.mTimeline->setDefaultAutoRemove(true);

	mDebugDraw = false;
	mShowFPS = false;
	mShowParams = false;

	mKinect.setup(mSceneState.mParams);
	
	mSceneState.mKinect = &mKinect;
	
	mSceneState.mBlackoutAmount = 0.0f;
	gl::Texture::Format hiQFormat;
	hiQFormat.enableMipmapping();
	hiQFormat.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
	hiQFormat.setMagFilter(GL_LINEAR_MIPMAP_LINEAR);
	
	// Blackout overlay
	texBlackout = gl::Texture(loadImage(loadAsset("blackout.png")), hiQFormat);
	
	mTreeLayer.setup(&mSceneState);
	mTreeLayer.setEnabled(true);
	
	mIntroLight.setup(&mSceneState);
	mIntroLight.setEnabled(true);	

    mCloseSwarm.setName("near");
    mCloseSwarm.setup(&mSceneState);
    mCloseSwarm.followUser(KinectWrapper::utClosest);
    mFarSwarm.setName("far");
    mFarSwarm.setup(&mSceneState);
    mFarSwarm.followUser(KinectWrapper::utFurthest);
}

void bobbevyApp::keyDown( KeyEvent event )
{
	switch( event.getChar() )
	{
		case KeyEvent::KEY_ESCAPE:
			this->quit();
			this->shutdown();
			break;			
		case KeyEvent::KEY_f:
			setFullScreen( !isFullScreen() );
			break;
		case KeyEvent::KEY_o:
			mSceneState.mTimeline->apply(&mSceneState.mBlackoutAmount, 1.0f, 5.0f);
			break;			
		case KeyEvent::KEY_i:
			if (mSceneState.mBlackoutAmount >= 1.0)
				mTreeLayer.resetParams();
			mSceneState.mTimeline->apply(&mSceneState.mBlackoutAmount, 0.0f, 5.0f);
			break;			
		case KeyEvent::KEY_t:
			mTreeLayer.setEnabled(!mTreeLayer.getEnabled());
			break;
		case KeyEvent::KEY_l:
			mIntroLight.setEnabled(!mIntroLight.getEnabled());
			break;			
		case KeyEvent::KEY_b:
			mCloseSwarm.setEnabled(!mCloseSwarm.getEnabled());
			break;			
        case KeyEvent::KEY_n:
            mFarSwarm.setEnabled(!mFarSwarm.getEnabled());
            break;
	}			
	mTreeLayer.keyDown(event);
	mIntroLight.keyDown(event);
	mKinect.keyDown(event);
	mCloseSwarm.keyDown(event);
    mFarSwarm.keyDown(event);
}

void bobbevyApp::mouseDown( MouseEvent event )
{
}

void bobbevyApp::update()
{
	mSceneState.mTimeline->step(0.05);
	mKinect.update();
	mTreeLayer.update();
	mIntroLight.update();
	mCloseSwarm.update();
    mFarSwarm.update();
}

void bobbevyApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 

	if (mDebugDraw)
		mKinect.draw();
	
	mTreeLayer.draw();
	mIntroLight.draw();
    mFarSwarm.draw();
	mCloseSwarm.draw();

	if (mSceneState.mBlackoutAmount > 0.0)
	{
		gl::color( cinder::ColorA(1, 1, 1, mSceneState.mBlackoutAmount) );
		gl::draw(texBlackout, getWindowBounds());
		gl::color( cinder::ColorA(1, 1, 1, 1) );
	}
	
	// Params
	if (mShowParams)
	{
		gl::setMatricesWindowPersp( getWindowWidth(), getWindowHeight());
		gl::disableDepthWrite();
		gl::disableDepthRead();
		params::InterfaceGl::draw();
		//		showCursor();
	} else {
		//		hideCursor();
	}
	
	if (mShowFPS)
	{
		gl::setMatricesWindowPersp( getWindowWidth(), getWindowHeight());
		std::string s = boost::lexical_cast<std::string>(getAverageFps());
		gl::drawString(s, Vec2f(100,100));
	}
}

CINDER_APP_BASIC( bobbevyApp, RendererGl )
