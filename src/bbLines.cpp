//
//  bbLines.cpp
//  bobbevy
//
//  Created by Brian Richardson on 4/19/13.
//
//

#include "bbLines.h"
#include "LiveAssetManager.h"
#include "bbKinectWrapper.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace gl;

LineLayer::LineLayer() :
  SceneLayer("LineLayer"),
  mFilterAmount(0.25f)
{
  
}


// QTimelineModule
void LineLayer::init()
{
  SceneLayer::init();

  registerParam("filterAmount", &mFilterAmount, 0.0, 1.0f);
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
  Blob* closest = mSceneState->mKinect->getUser(KinectWrapper::utClosest);
  if (!closest)
    return;
  Blob* far = mSceneState->mKinect->getUser(KinectWrapper::utFurthest);

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
  
  mShader.uniform("iMouse", Vec4f(mPos[0].x, mPos[0].y, mPos[1].x, mPos[1].y));
  
  gl::pushMatrices();
  gl::setMatricesWindow(app::getWindowSize(), false); //false: vertical flip
  gl::drawSolidRect(app::getWindowBounds());
  gl::popMatrices();
  mShader.unbind();
}