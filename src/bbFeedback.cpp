//
//  bbFeedback.cpp
//  bobbevy
//
//  Created by Brian Richardson on 5/9/13.
//
//

#include "bbFeedback.h"
#include "cinder/gl/gl.h"
#include "bbKinectWrapper.h"
#include "LiveAssetManager.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace gl;

FeedbackLayer::FeedbackLayer()
: SceneLayer("FeedbackLayer")
, mPrevFrame(0)
, mFboInit(false)
, mNoiseAmp(0.0)
{
}

// SceneLayer
void FeedbackLayer::setup(SceneState* sceneState)
{
  mSceneState = sceneState;
  LiveAssetManager::load("PassThruVert.glsl", "FeedbackFrag.glsl",
                         [this](ci::DataSourceRef vert,ci::DataSourceRef frag)
                         {
                           try
                           {
                             mShader = gl::GlslProg(vert, frag);
                           }
                           catch (GlslProgCompileExc e)
                           {
                             printf("%s\n", e.what());
                           }
                         });
}

void FeedbackLayer::tick()
{
  auto a = gl::getViewport();
  gl::pushMatrices();
  initFbo();
  int prev = mPrevFrame;
  int curr = 1 - prev;
  {
    int w = mFrames[curr].getBounds().getWidth();
    int h = mFrames[curr].getBounds().getHeight();
    
    gl::SaveFramebufferBinding bindingSaver;
    mFrames[curr].bindFramebuffer();
    
    gl::setViewport(mFrames[curr].getBounds());
    gl::setMatricesWindow(w, h);
    gl::scale(1.0f, -1.0f);
    gl::translate(0.0f, -h);
    gl::clear(Color(0.0f, 0.0f, 0.0f));
    
    // Render something new
    Texture c = mSceneState->mKinect->getContourTexture();
    if (c)
    {
      mShader.bind();
      mShader.uniform("newContour", 0);
      mShader.uniform("oldFeedback", 1);
      mShader.uniform("iResolution", Vec3f(w, h, 1));
      mShader.uniform("iGlobalTime", (float) getElapsedSeconds());
      mShader.uniform("iInOut", mInOut > 0.5f ? -1.0f : 1.0f);
      mShader.uniform("iNoiseAmp", mNoiseAmp);
      
      c.enableAndBind();
      mFrames[prev].getTexture(0).bind(1);
      gl::drawSolidRect(Rectf(0,0,w,h));
      mFrames[prev].getTexture(0).unbind();
      c.unbind();
      c.disable();
      mShader.unbind();
    }
    
    mPrevFrame = 1 - mPrevFrame;
  }
  gl::popMatrices();
  gl::setViewport(a);
  gl::setMatricesWindow(getWindowSize());
}

void FeedbackLayer::draw()
{
  if (!mFboInit)
    return;
  gl::disableAlphaBlending();
  gl::setMatricesWindow(getWindowSize());
  Rectf dst(Vec2f::zero(), getWindowSize());
  gl::draw(mFrames[mPrevFrame].getTexture(0), dst);
}

// QTimelineModule
void FeedbackLayer::init()
{
  SceneLayer::init();
  registerParam("inOut", &mInOut);
  registerParam("NoiseAmp", &mNoiseAmp);  
}

// helpers
void FeedbackLayer::initFbo()
{
  if (mFboInit)
    return;
  mFboInit = true;
  gl::Fbo::Format format;
  // Kinect size
  for (int i = 0; i < NUM_FRAMES; i++)
  {
    mFrames[i] = gl::Fbo(KinectWrapper::smSize.x, KinectWrapper::smSize.y, format);
    gl::SaveFramebufferBinding saveBinding;
    mFrames[i].bindFramebuffer();
    gl::setViewport(mFrames[i].getBounds());
    gl::setMatricesWindow(mFrames[i].getWidth(), mFrames[i].getHeight());
    gl::clear(Color(1.0f, 0.0f, 0.0f));
  }
}