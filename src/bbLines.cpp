//
//  bbLines.cpp
//  bobbevy
//
//  Created by Brian Richardson on 4/19/13.
//
//

#include "bbLines.h"
#include "LiveAssetManager.h"
#include "bbDepthProcessor.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace gl;

LineLayer::LineLayer() :
  SceneLayer("LineLayer"),
  mFilterAmount(0.25f),
  mTimeMult(0.33f),
  mRays(64.0f),
  mPeriodDivider(1.3f),
  mInnerRadius(0.004f)
{
  
}


// QTimelineModule
void LineLayer::init()
{
  SceneLayer::init();

  registerParam("filterAmount", &mFilterAmount, 0.0, 1.0f);
  registerParam("timeMult", &mTimeMult, 0.0, 2.0f);
  registerParam("Rays", &mRays, 1.0f, 200.0f);
  registerParam("PeriodDivider", &mPeriodDivider, 1.0f, 4.0f);
  registerParam("InnerRadius", &mInnerRadius, 0.0, 0.75f);
}

// SceneLayer
void LineLayer::setup(SceneState* sceneState)
{
  mSceneState = sceneState;
  LiveAssetManager::load("PassThruVert.glsl", "LinesFrag.glsl",
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
  Vec2i ws = app::getWindowSize();
  mPos[0] = Vec2f(ws.x * 0.25f, ws.y * 0.5f);
  mPos[1] = Vec2f(ws.x * 0.75f, ws.y * 0.5f);
}

void LineLayer::tick()
{
  BlobRef closest = mSceneState->mKinect->getUser(DepthProcessor::utClosest);
  if (!closest)
    return;
  BlobRef far = mSceneState->mKinect->getUser(DepthProcessor::utFurthest);

  Vec2f pos[2];
  pos[0] = closest->mBounds.getCenter();
  pos[1] = far->mBounds.getCenter();

  float closeDist2 = (pos[0] - mPos[0]).lengthSquared();
  float farDist2 = (pos[1] - mPos[0]).lengthSquared();
  
  if (closeDist2 > farDist2)
  {
    Vec2f t = pos[1];
    pos[1] = pos[0];
    pos[0] = t;
  }

  mPos[0] = mPos[0].lerp(mFilterAmount, pos[0]);
  mPos[1] = mPos[1].lerp(mFilterAmount, pos[1]);
}

void LineLayer::draw()
{
  mShader.bind();
  Vec2i ws = app::getWindowSize();
  mShader.uniform("iResolution", Vec3f(ws.x, ws.y, 1));
  mShader.uniform("iGlobalTime", (float) getElapsedSeconds());
  
  float xs = getWindowWidth() / 640.0f; // need a KinectCoord space constant
  float ys = getWindowHeight() / 480.0f; // BTRTODO: ditto
  
  mShader.uniform("iMouse", Vec4f(mPos[0].x * xs, mPos[0].y * ys,
                                  mPos[1].x * xs, mPos[1].y * ys));
  mShader.uniform("iTimeMult", mTimeMult);
  mShader.uniform("iRays", mRays);
  float period = (M_PI * 2.0f) / (mRays / 2.0f);
  mShader.uniform("iPeriod", period);
  mShader.uniform("iHalf_period", period / mPeriodDivider);
  mShader.uniform("iInner_radius", mInnerRadius);
  
  gl::pushMatrices();
  gl::setMatricesWindow(app::getWindowSize(), false); //false: vertical flip
  gl::drawSolidRect(app::getWindowBounds());
  gl::popMatrices();
  mShader.unbind();
}