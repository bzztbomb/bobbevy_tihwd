// TODO:
// Get OSC working again

#include <queue>
#include <boost/thread.hpp>

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
#include "QTimeline.h"

#include "bbKinectWrapper.h"
#include "sceneLayer.h"
#include "bbTreeLayer.h"
#include "bbIntroLight.h"
#include "bbParticles.h"
#include "bbParticleField.h"
#include "BlackoutLayer.h"
#include "bbLines.h"

#include "LabMidi/LabMidiCommand.h"
#include "LabMidi/LabMidiIn.h"
#include "LabMidi/LabMidiOut.h"
#include "LabMidi/LabMidiUtil.h"

using namespace ci;
using namespace ci::app;
using namespace std;

static const int WIDTH = 800;
static const int HEIGHT = 600;

QTimelineWidget::QTimelineWidget( std::string name)
{
  mFont           = QTimeline::getPtr()->getFontSmall();
  mMenu           = new QTimelineMenu();
  
  setName( name );
}

void QTimelineMenuItem::init()
{
  mHeight = 0.0f;
  
  mFont   = QTimeline::getPtr()->getFontSmall();
}



struct WindowData
{
  WindowData()
  {
    mDisplayTimeline = true;
    mDisplayScene = true;
  }
  
  bool mDisplayTimeline;
  bool mDisplayScene;
};

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
  
  // Midi
  Lab::MidiIn*         midiIn;
  Lab::MidiOut*        midiOut;
  
  // Command queue
  std::queue<int> mCommandQueue;
  boost::mutex mCommandMutex;
  
  // Timeline
  QTimelineRef mTimeline;
  vector<QTimelineModuleRef> mModules;
  
  // Simulation
  double mCurrentTime;
  double mAccumlator;
  double mDT;
  
	// Layers
	SceneState mSceneState;
  
  void handleOSC();
  
  void createModuleCallback( QTimeline::CreateModuleCallbackArgs args );
  void deleteModuleCallback( QTimeline::DeleteModuleCallbackArgs args );
  
  void createNewWindow();
  
  static void midiCallback(void* userData, Lab::MidiCommand* m);
};

void bobbevyApp::prepareSettings( Settings* settings )
{
	settings->setFrameRate( 60 );
	settings->setWindowSize( WIDTH, HEIGHT );
	settings->setTitle( "bobbevy" );
}

extern void publish_via_bonjour();

void bobbevyApp::setup()
{
  getWindow()->setUserData( new WindowData );
  
  mTimeline = QTimelineRef( new QTimeline() );
  mTimeline->init();
  
  mTimeline->registerModule("TreeLayer", this,
                            &bobbevyApp::createModuleCallback,
                            &bobbevyApp::deleteModuleCallback);
  mTimeline->registerModule("CloseSwarm", this,
                            &bobbevyApp::createModuleCallback,
                            &bobbevyApp::deleteModuleCallback);
  mTimeline->registerModule("FarSwarm", this,
                            &bobbevyApp::createModuleCallback,
                            &bobbevyApp::deleteModuleCallback);
  mTimeline->registerModule("Field", this,
                            &bobbevyApp::createModuleCallback,
                            &bobbevyApp::deleteModuleCallback);
  mTimeline->registerModule("BlackoutLayer", this,
                            &bobbevyApp::createModuleCallback,
                            &bobbevyApp::deleteModuleCallback);
  mTimeline->registerModule("LineLayer", this,
                            &bobbevyApp::createModuleCallback,
                            &bobbevyApp::deleteModuleCallback);
  
	mSceneState.mParams = params::InterfaceGl("bobbevy", Vec2i(225, 200));
	mSceneState.mParams.addParam("DebugDraw", &mDebugDraw, "keyIncr=d");
	mSceneState.mParams.addParam("ShowParams", &mShowParams, "keyIncr=p");
	mSceneState.mParams.addParam("ShowFPS", &mShowFPS);
	  
	mDebugDraw = false;
	mShowFPS = false;
	mShowParams = false;
  
  mListener.setup(23232);
  publish_via_bonjour();

  midiIn = new Lab::MidiIn();
  midiOut = new Lab::MidiOut();
  midiIn->addCallback(bobbevyApp::midiCallback, this);
  midiIn->openPort(1);
  midiOut->openPort(1);
  
	mKinect.setup(mSceneState.mParams);
	
	mSceneState.mKinect = &mKinect;
	
	gl::Texture::Format hiQFormat;
	hiQFormat.enableMipmapping();
	hiQFormat.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
	hiQFormat.setMagFilter(GL_LINEAR_MIPMAP_LINEAR);
  
  mCurrentTime = getElapsedSeconds();
  mAccumlator = 0.0;
  mDT = 1.0/30.0;  
}

void bobbevyApp::keyDown( KeyEvent event )
{
  WindowData *wd = getWindow()->getUserData<WindowData>();

  float zoomInc = 0.25f;
  if (event.isMetaDown())
  {
    switch (event.getCode())
    {
      case KeyEvent::KEY_l:
        mTimeline->load("/Users/bzztbomb/projects/bobbevy/timeline.xml");
        break;
      case KeyEvent::KEY_s:
        mTimeline->save("/Users/bzztbomb/projects/bobbevy/timeline.xml");
        break;
      case KeyEvent::KEY_f:
        {
          FullScreenOptions fo;
          fo.secondaryDisplayBlanking(false);
          setFullScreen( !isFullScreen(), fo );
        }
        break;
      case KeyEvent::KEY_RETURN:
        mTimeline->playCue();
        break;
      case KeyEvent::KEY_w:
        createNewWindow();
        break;
      case KeyEvent::KEY_EQUALS:
        mTimeline->setZoom(mTimeline->getZoom() + zoomInc);
        break;
      case KeyEvent::KEY_KP_MINUS:
        mTimeline->setZoom(mTimeline->getZoom() - zoomInc);
        break;
      case KeyEvent::KEY_p:
        mShowParams = !mShowParams;
        break;
    }
  }
	switch( event.getCode() )
	{
		case KeyEvent::KEY_ESCAPE:
			this->quit();
			this->shutdown();
			break;
    case KeyEvent::KEY_F1:
      {
        wd->mDisplayTimeline = !wd->mDisplayTimeline;
      };
      break;
    case KeyEvent::KEY_F2:
      {
        wd->mDisplayScene = !wd->mDisplayScene;
      };
      break;
    case KeyEvent::KEY_F5:
      {
        mTimeline->play( !mTimeline->isPlaying(), QTimeline::FREE_RUN );
      };
      break;
	}
	mKinect.keyDown(event);

//  auto tracks = mTimeline->getTracks();
//  for (auto i = tracks.rbegin(); i != tracks.rend(); i++)
//  {
//    auto item = (*i)->getActiveItem();
//    if (item)
//    {
//      SceneLayer* sl = static_cast<SceneLayer*>(item->getTargetModule().get());
//      sl->keyDown(event);
//    }
//  }
}

void bobbevyApp::mouseDown( MouseEvent event )
{
//  mFarSwarm.mouseDown(event);
}

void bobbevyApp::update()
{
  handleOSC();
  {
    boost::mutex::scoped_lock lock(mCommandMutex);
    int mCue = -1;
    while (!mCommandQueue.empty())
    {
      mCue = mCommandQueue.front();
      mCommandQueue.pop();
    }
    if (mCue != -1)
    {
      if (mTimeline->isPlaying())
        mTimeline->play(false);
      mTimeline->playCue(mCue);
    }
  }

  mTimeline->update();
  double newTime = getElapsedSeconds();
  double frameTime = newTime - mCurrentTime;
  mCurrentTime = newTime;
  mAccumlator += frameTime;
  //    int i = 0;
  //    int max_ticks = 0;
  while (mAccumlator >= mDT)
  {
    mAccumlator -= mDT;
    
    mKinect.update();
    
    auto tracks = mTimeline->getTracks();
    for (auto i = tracks.rbegin(); i != tracks.rend(); i++)
    {
      auto item = (*i)->getActiveItem();
      if (item)
      {
        SceneLayer* sl = static_cast<SceneLayer*>(item->getTargetModule().get());
        sl->tick();
      }
    }
    
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
    
    if ((message.getAddress().find("/multi/") != std::string::npos) &&
        (message.getNumArgs() == 2) &&
        (message.getArgType(0) == osc::TYPE_FLOAT) &&
        (message.getArgType(1) == osc::TYPE_FLOAT))
    {
      char which = message.getAddress()[7];
      int index = boost::lexical_cast<int>(which);
      mKinect.updateFakeBlob(index-1, Vec2f(message.getArgAsFloat(0), message.getArgAsFloat(1)));
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

void bobbevyApp::midiCallback(void* userData, Lab::MidiCommand* m)
{
  if (!m)
    return;
  if (m->command == MIDI_NOTE_ON)
  {
    bobbevyApp* bb = static_cast<bobbevyApp*>(userData);
    int offPos = m->byte1 - 36;
    if ((offPos < 0) || (offPos > 15))
      return;
    
    // Push it to the command queue.
    {
      boost::mutex::scoped_lock lock(bb->mCommandMutex);
      bb->mCommandQueue.push(offPos);
    }
    
    std::cout << "NOTE: " << (int) m->byte1 << " " << Lab::noteName(m->byte1) << " POS: " << offPos << std::endl;
    int pos = (offPos + 1) * 2;
    for (int i = pos; i < 32; i++)
    {
      bb->midiOut->sendNoteOn(0, i, 0);
    }
    for (int i = 0; i < pos; i++)
    {
      bb->midiOut->sendNoteOn(0, i, 127);
    }
  }
}


void bobbevyApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );

  WindowData* wd = getWindow()->getUserData<WindowData>();
  
	if (mDebugDraw)
		mKinect.draw();
  
  if (wd->mDisplayScene)
  {
    auto tracks = mTimeline->getTracks();
    for (auto i = tracks.rbegin(); i != tracks.rend(); i++)
    {
      auto item = (*i)->getActiveItem();
      if (item)
        item->getTargetModule()->render();
    }
  }

  if (wd->mDisplayTimeline)
  {
    gl::enableAlphaBlending();
    mTimeline->render();
    gl::disableAlphaBlending();
  }
  
	// Params
	if (mShowParams)
	{
    //   showCursor();
		gl::setMatricesWindowPersp( getWindowWidth(), getWindowHeight());
		gl::disableDepthWrite();
		gl::disableDepthRead();
    mSceneState.mParams.draw();
//		params::InterfaceGl::draw();
    
	} else {
		// hideCursor();
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

void bobbevyApp::createModuleCallback( QTimeline::CreateModuleCallbackArgs args )
{
  QTimelineModuleRef  mod;
  QTimelineItemRef    item = args.itemRef;
  
  if( args.type == "TreeLayer" )
  {
    TreeLayer* tl = new TreeLayer();
    tl->setup(&mSceneState);
    mod = QTimelineModuleRef( tl );
  }
  if (args.type == "CloseSwarm")
  {
    SkeletonParticles* sp = new SkeletonParticles("CloseSwarm");
    sp->setName("CloseSwarm");
    sp->setup(&mSceneState);
    sp->followUser(KinectWrapper::utClosest);
    mod = QTimelineModuleRef(sp);
  }
  if (args.type == "FarSwarm")
  {
    SkeletonParticles* sp = new SkeletonParticles("FarSwarm");
    sp->setName("FarSwarm");
    sp->setup(&mSceneState);
    sp->followUser(KinectWrapper::utFurthest);
    mod = QTimelineModuleRef(sp);
  }
  if (args.type == "Field")
  {
    ParticleField* pf = new ParticleField();
    pf->setup(&mSceneState);
    mod = QTimelineModuleRef(pf);
  }
  if (args.type == "BlackoutLayer")
  {
    BlackoutLayer* bl = new BlackoutLayer();
    bl->setup(&mSceneState);
    mod = QTimelineModuleRef(bl);
  }
  if (args.type == "LineLayer")
  {
    LineLayer* ll = new LineLayer();
    ll->setup(&mSceneState);
    mod = QTimelineModuleRef(ll);
  }
  
  if ( !mod )
    return;
  
  mod->setItemRef(item);
  item->setTargetModule( mod );
  mod->init();
  mModules.push_back( mod );
}


void bobbevyApp::deleteModuleCallback( QTimeline::DeleteModuleCallbackArgs args )
{
  for( size_t k=0; k < mModules.size(); k++ )
    if ( mModules[k] == args.itemRef->getTargetModule() )
    {
      mModules.erase( mModules.begin() + k );
      return;
    }
}

void bobbevyApp::createNewWindow()
{
  app::WindowRef newWindow = createWindow( Window::Format().size( 400, 400 ) );
  WindowData* wd = new WindowData;
  wd->mDisplayTimeline = false;
  newWindow->setUserData( wd );
  
  // for demonstration purposes, we'll connect a lambda unique to this window which fires on close
  int uniqueId = getNumWindows();
  newWindow->getSignalClose().connect(
                                      [uniqueId,this] { this->console() << "You closed window #" << uniqueId << std::endl; }
                                      );
}

CINDER_APP_BASIC( bobbevyApp, RendererGl )
