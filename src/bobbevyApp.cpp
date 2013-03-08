// TODO:
// Get OSC working again

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


using namespace ci;
using namespace ci::app;
using namespace std;

static const int WIDTH = 800;
static const int HEIGHT = 600;

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

class SceneLayerModule : public QTimelineModule
{
  
public:
  SceneLayerModule(const std::string& type, SceneLayer* layer) :
    QTimelineModule( type ),
    mLayer(layer)
  {
  }
  
  void init()
  {
  }
  
  void update()
  {
  }

  virtual void render()
  {
    mLayer->draw();
  }
  
  virtual void activeChanged(bool active)
  {
    mLayer->setEnabled(true);
  }
private:
  SceneLayer* mLayer;
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
  map<int, int> mMessageMap;
  
  QTimelineRef mTimeline;
  vector<QTimelineModuleRef> mModules;
  
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
  
  float mFadeInSlow;
  float mFadeInNormal;
  
  void handleOSC();
  void initMsgMap();
  
  void createModuleCallback( QTimeline::CreateModuleCallbackArgs args );
  void deleteModuleCallback( QTimeline::DeleteModuleCallbackArgs args );
  
  void createNewWindow();
};

void bobbevyApp::prepareSettings( Settings* settings )
{
	settings->setFrameRate( 60 );
	settings->setWindowSize( WIDTH, HEIGHT );
	settings->setTitle( "bobbevy" );
	settings->enableSecondaryDisplayBlanking(false);
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
  mTimeline->registerModule("IntroLight", this,
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
  
	mSceneState.mParams = params::InterfaceGl("bobbevy", Vec2i(225, 200));
	mSceneState.mParams.addParam("DebugDraw", &mDebugDraw, "keyIncr=d");
	mSceneState.mParams.addParam("ShowParams", &mShowParams, "keyIncr=p");
	mSceneState.mParams.addParam("ShowFPS", &mShowFPS);
	mSceneState.mParams.addParam("FadeInNormal", &mFadeInNormal);
	mSceneState.mParams.addParam("FadeInSlow", &mFadeInSlow);
	
	mSceneState.mTimeline = Timeline::create();
	mSceneState.mTimeline->setDefaultAutoRemove(true);
  
	mDebugDraw = false;
	mShowFPS = false;
	mShowParams = false;
  mSceneState.mBlackoutColor = ColorA(0.0f, 0.0f, 0.0f, 1.0f);
  
  mListener.setup(23232);
  publish_via_bonjour();
  
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
    }
  }
	switch( event.getCode() )
	{
		case KeyEvent::KEY_ESCAPE:
			this->quit();
			this->shutdown();
			break;
#if 0
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
      //		case KeyEvent::KEY_l:
      //			mIntroLight.setEnabled(!mIntroLight.getEnabled());
      //			break;
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
      mSceneState.mBlackoutColor = ColorA(1.0, 0.85f, 0.85f, 1.0f);
      break;
    case KeyEvent::KEY_d:
      mSceneState.mBlackoutColor = ColorA(0.0, 0.0f, 0.0f, 1.0f);
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
#endif
    case KeyEvent::KEY_h:
      mSceneState.mTimeline->clear();
      break;
    case KeyEvent::KEY_SPACE:
      mTimeline->play( !mTimeline->isPlaying(), QTimeline::FREE_RUN );
      break;
    case KeyEvent::KEY_RETURN:
      mTimeline->playCue();
      break;
    case KeyEvent::KEY_w:
      createNewWindow();
      break;
    case KeyEvent::KEY_F1:
      {
        WindowData* wd = getWindowIndex(0)->getUserData<WindowData>();
        wd->mDisplayTimeline = !wd->mDisplayTimeline;
      };
      break;
    case KeyEvent::KEY_F2:
      {
        WindowData* wd = getWindowIndex(0)->getUserData<WindowData>();
        wd->mDisplayScene = !wd->mDisplayScene;
      };
      break;
	}
	mKinect.keyDown(event);

#if 0
	mTreeLayer.keyDown(event);
	mIntroLight.keyDown(event);
	mCloseSwarm.keyDown(event);
  mFarSwarm.keyDown(event);
  mField.keyDown(event);
#endif
  auto tracks = mTimeline->getTracks();
  for (auto i = tracks.rbegin(); i != tracks.rend(); i++)
  {
    auto item = (*i)->getActiveItem();
    if (item)
    {
      SceneLayer* sl = static_cast<SceneLayer*>(item->getTargetModule().get());
      //sl->keyDown(event);
    }
  }
}

void bobbevyApp::mouseDown( MouseEvent event )
{
  mFarSwarm.mouseDown(event);
}

void bobbevyApp::update()
{
  handleOSC();
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
    
    mSceneState.mTimeline->step(0.05);
    mKinect.update();
    mTreeLayer.tick();
    mIntroLight.tick();
    mCloseSwarm.tick();
    mFarSwarm.tick();
    mField.tick();
    
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
    
    if ((message.getAddress().compare("/bobbevy/key") == 0) && (message.getArgType(0) == osc::TYPE_STRING) && (message.getArgAsString(0).size() > 0))
    {
      int code = message.getArgAsString(0).c_str()[0];
      map<int,int>::iterator iter = mMessageMap.find( code );
      if( iter != mMessageMap.end() )
      {
//        KeyEvent ev(getWindow(), iter->second, iter->second, 0, 0);
//        keyDown(ev);
      }
    } else {
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
}

void bobbevyApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );

  WindowData* wd = getWindow()->getUserData<WindowData>();
  
#if 0
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
  
  for( size_t k=0; k < mModules.size(); k ++ )
    if ( mModules[k]->isPlaying() )
      mModules[k]->render();  
#endif

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
#if 0
      auto mods = (*i)->getItems();
      for (auto module : mods)
      {
        QTimelineModuleRef mod = module->getTargetModule();
        if ((mod->getItemRef() != NULL) && (mod->isPlaying()))
          mod->render();
      }
#endif
    }
  }

  mFarSwarm.draw();
	mCloseSwarm.draw();

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
		params::InterfaceGl::draw();
    
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
  if (args.type == "IntroLight")
    mod = QTimelineModuleRef( new SceneLayerModule("IntroLight", &mIntroLight));
  if (args.type == "CloseSwarm")
    mod = QTimelineModuleRef( new SceneLayerModule("CloseSwarm", &mCloseSwarm));
  if (args.type == "FarSwarm")
    mod = QTimelineModuleRef( new SceneLayerModule("FarSwarm", &mFarSwarm));
  if (args.type == "Field")
    mod = QTimelineModuleRef( new SceneLayerModule("Field", &mField));
  
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
