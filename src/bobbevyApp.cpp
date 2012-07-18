#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Surface.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/params/Params.h"

#include "CinderOpenCV.h"
#include "Kinect.h"
#include "OscListener.h"

#include "bbKinectWrapper.h"
#include "sceneLayer.h"
#include "bbTreeLayer.h"
#include "bbIntroLight.h"
#include "bbParticles.h"
#include "bbParticleField.h"


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
    
    // Osc
    osc::Listener mListener;
    map<int, int> mMessageMap;

    // Simulation
    double mCurrentTime;
    double mAccumlator;
    double mDT;
    
	// Layers
	SceneState mSceneState;
	TreeLayer mTreeLayer;
	IntroLight mIntroLight;
	SkeletonParticles mCloseSwarm;
	SkeletonParticles mFarSwarm;
    ParticleField mField;

    ColorA mBlackoutColor;
    float mFadeInSlow;
    float mFadeInNormal;
    
    void handleOSC();
    void initMsgMap();
};

void bobbevyApp::prepareSettings( Settings* settings )
{
	settings->setFrameRate( 60 );
	settings->setWindowSize( WIDTH, HEIGHT );
	settings->setTitle( "bobbevy" );
	settings->enableSecondaryDisplayBlanking(false);
}

void bobbevyApp::setup()
{
	mSceneState.mParams = params::InterfaceGl("bobbevy", Vec2i(225, 200));	
	mSceneState.mParams.addParam("DebugDraw", &mDebugDraw, "keyIncr=d");
	mSceneState.mParams.addParam("ShowParams", &mShowParams, "keyIncr=p");
	mSceneState.mParams.addParam("ShowFPS", &mShowFPS);
    mSceneState.mParams.addParam("FadeInNormal", &mFadeInNormal);
    mSceneState.mParams.addParam("FadeInSlow", &mFadeInSlow);
	
	mSceneState.mTimeline = Timeline::create();
	mSceneState.mTimeline->setDefaultAutoRemove(true);

	mDebugDraw = false;
	mShowFPS = true;
	mShowParams = false;
    mBlackoutColor = ColorA(0.0f, 0.0f, 0.0f, 1.0f);
    
    mListener.setup(23232);
    initMsgMap();

	mKinect.setup(mSceneState.mParams);
	
	mSceneState.mKinect = &mKinect;
	
	mSceneState.mBlackoutAmount = 0.0f;
	gl::Texture::Format hiQFormat;
	hiQFormat.enableMipmapping();
	hiQFormat.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
	hiQFormat.setMagFilter(GL_LINEAR_MIPMAP_LINEAR);
	
	// Blackout overlay	
	mTreeLayer.setup(&mSceneState);
	mTreeLayer.setSwarms(&mCloseSwarm, &mFarSwarm);
    
	mIntroLight.setup(&mSceneState);

    mCloseSwarm.setName("near");
    mCloseSwarm.setup(&mSceneState);
    mCloseSwarm.followUser(KinectWrapper::utClosest);
    mFarSwarm.setName("far");
    mFarSwarm.setup(&mSceneState);
    mFarSwarm.followUser(KinectWrapper::utFurthest);
    mField.setup(&mSceneState);
    
    mCurrentTime = getElapsedSeconds();
    mAccumlator = 0.0;
    mDT = 1.0/30.0;
    
    mFadeInNormal = 10.0f;
    mFadeInSlow = 40.0f; 
}

void bobbevyApp::initMsgMap()
{
	mMessageMap['0'] = KeyEvent::KEY_0;
	mMessageMap['1'] = KeyEvent::KEY_1;
	mMessageMap['2'] = KeyEvent::KEY_2;
	mMessageMap['3'] = KeyEvent::KEY_3;
	mMessageMap['4'] = KeyEvent::KEY_4;
	mMessageMap['5'] = KeyEvent::KEY_5;
	mMessageMap['6'] = KeyEvent::KEY_6;
	mMessageMap['7'] = KeyEvent::KEY_7;
	mMessageMap['8'] = KeyEvent::KEY_8;
	mMessageMap['9'] = KeyEvent::KEY_9;
	mMessageMap['A'] = KeyEvent::KEY_a;
	mMessageMap['B'] = KeyEvent::KEY_b;
	mMessageMap['C'] = KeyEvent::KEY_c;
	mMessageMap['D'] = KeyEvent::KEY_d;
	mMessageMap['E'] = KeyEvent::KEY_e;
	mMessageMap['F'] = KeyEvent::KEY_f;
	mMessageMap['G'] = KeyEvent::KEY_g;
	mMessageMap['H'] = KeyEvent::KEY_h;
	mMessageMap['I'] = KeyEvent::KEY_i;
	mMessageMap['J'] = KeyEvent::KEY_j;
	mMessageMap['K'] = KeyEvent::KEY_k;
	mMessageMap['L'] = KeyEvent::KEY_l;
	mMessageMap['M'] = KeyEvent::KEY_m;
	mMessageMap['N'] = KeyEvent::KEY_n;
	mMessageMap['O'] = KeyEvent::KEY_o;
	mMessageMap['P'] = KeyEvent::KEY_p;
	mMessageMap['Q'] = KeyEvent::KEY_q;
	mMessageMap['R'] = KeyEvent::KEY_r;
	mMessageMap['S'] = KeyEvent::KEY_s;
	mMessageMap['T'] = KeyEvent::KEY_t;
	mMessageMap['U'] = KeyEvent::KEY_u;
	mMessageMap['V'] = KeyEvent::KEY_v;
	mMessageMap['W'] = KeyEvent::KEY_w;
	mMessageMap['X'] = KeyEvent::KEY_x;
	mMessageMap['Y'] = KeyEvent::KEY_y;
	mMessageMap['Z'] = KeyEvent::KEY_z;
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
		case KeyEvent::KEY_t:
			mTreeLayer.setEnabled(!mTreeLayer.getEnabled());
            mTreeLayer.setLeaves(false);
			break;
        case KeyEvent::KEY_e:
            mTreeLayer.setEnabled(true);
            mTreeLayer.setLeaves(true);
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
        case KeyEvent::KEY_v:
            mField.setEnabled(!mField.getEnabled());
            break;
        // Fade in/out
        case KeyEvent::KEY_c:
            mBlackoutColor = ColorA(1.0, 0.85f, 0.85f, 1.0f);
            break;
        case KeyEvent::KEY_d:
            mBlackoutColor = ColorA(0.0, 0.0f, 0.0f, 1.0f);
            break;            
		case KeyEvent::KEY_o:
			mSceneState.mTimeline->apply(&mSceneState.mBlackoutAmount, 1.0f, 5.0f);
			break;			
		case KeyEvent::KEY_i:
			mSceneState.mTimeline->apply(&mSceneState.mBlackoutAmount, 0.0f, mFadeInNormal);
			break;
		case KeyEvent::KEY_g:
			mSceneState.mTimeline->apply(&mSceneState.mBlackoutAmount, 0.0f, mFadeInSlow);
			break;	
        case KeyEvent::KEY_h:
            mSceneState.mTimeline->clear();
            break;
	}			
	mTreeLayer.keyDown(event);
	mIntroLight.keyDown(event);
	mKinect.keyDown(event);
	mCloseSwarm.keyDown(event);
    mFarSwarm.keyDown(event);
    mField.keyDown(event);
}

void bobbevyApp::mouseDown( MouseEvent event )
{
    mFarSwarm.mouseDown(event);
}

void bobbevyApp::update()
{
    handleOSC();
    double newTime = getElapsedSeconds();
    double frameTime = newTime - mCurrentTime;
    mCurrentTime = newTime;
    mAccumlator += frameTime;
//    int i = 0;
//    int max_ticks = 0;
    while (mAccumlator >= mDT)
    {
        mAccumlator -= mDT;
        
        mSceneState.mTimeline->step(0.05);
        mKinect.update();
        mTreeLayer.update();
        mIntroLight.update();
        mCloseSwarm.update();
        mFarSwarm.update();
        mField.update();
//        i++;
//        if (i > max_ticks)
//        {
//            mAccumlator = mDT * -10;
//            mCurrentTime = getElapsedSeconds();
//        }
    }
}

void bobbevyApp::handleOSC()
{
    while (mListener.hasWaitingMessages()) 
    {
        osc::Message message;
        mListener.getNextMessage(&message);
        
        if ((message.getAddress().compare("/bobbevy/key") == 0) && (message.getArgType(0) == osc::TYPE_STRING) && (message.getArgAsString(0).size() > 0))
        {
            int code = message.getArgAsString(0).c_str()[0];
            map<int,int>::iterator iter = mMessageMap.find( code );
            if( iter != mMessageMap.end() )
            {
                KeyEvent ev(iter->second, iter->second, 0, 0);
                keyDown(ev);
            }
        } else {        
            if ((message.getAddress().find("/multi/") != std::string::npos) && 
                (message.getNumArgs() == 2) &&
                (message.getArgType(0) == osc::TYPE_FLOAT) &&
                (message.getArgType(1) == osc::TYPE_FLOAT))
            {
                char which = message.getAddress()[7];
                int index = boost::lexical_cast<int>(which);
                mKinect.updateFakeBlob(index-1, Vec2f(message.getArgAsFloat(0), 1.0f - message.getArgAsFloat(1)));
            } else {
                console() << "New message received" << std::endl;
                console() << "Address: " << message.getAddress() << std::endl;
                console() << "Num Arg: " << message.getNumArgs() << std::endl;
                for (int i = 0; i < message.getNumArgs(); i++) {
                    console() << "-- Argument " << i << std::endl;
                    console() << "---- type: " << message.getArgTypeName(i) << std::endl;
                    if (message.getArgType(i) == osc::TYPE_INT32){
                        try {
                            console() << "------ value: "<< message.getArgAsInt32(i) << std::endl;
                        }
                        catch (...) {
                            console() << "Exception reading argument as int32" << std::endl;
                        }
                        
                    }else if (message.getArgType(i) == osc::TYPE_FLOAT){
                        try {
                            console() << "------ value: " << message.getArgAsFloat(i) << std::endl;
                        }
                        catch (...) {
                            console() << "Exception reading argument as float" << std::endl;
                        }
                    }else if (message.getArgType(i) == osc::TYPE_STRING){
                        try {
                            console() << "------ value: " << message.getArgAsString(i).c_str() << std::endl;
                        }
                        catch (...) {
                            console() << "Exception reading argument as string" << std::endl;
                        }
                        
                    }
                }
            }
        }
    }
}

void bobbevyApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
	
	mTreeLayer.draw();
	mIntroLight.draw();

	if (mSceneState.mBlackoutAmount > 0.0)
	{
        gl::enableAlphaBlending();
        ColorA b = mBlackoutColor;
        b.a = mSceneState.mBlackoutAmount;
		gl::color( b );
        gl::drawSolidRect(getWindowBounds());
		gl::color( cinder::ColorA(1, 1, 1, 1) );
        gl::disableAlphaBlending();
	}    
    
	if (mDebugDraw)
		mKinect.draw();
    
    mFarSwarm.draw();
	mCloseSwarm.draw(); 
    mField.draw();

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
        gl::enableAlphaBlending();
		gl::setMatricesWindowPersp( getWindowWidth(), getWindowHeight());
		std::string s = boost::lexical_cast<std::string>(getAverageFps());
		gl::drawString(s, Vec2f(100,100));
        gl::disableAlphaBlending();
	}
}

CINDER_APP_BASIC( bobbevyApp, RendererGl )
