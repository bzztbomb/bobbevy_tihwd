//
//  bbAntiParticleField.cpp
//  bobbevy
//
//  Created by Brian Richardson on 7/4/12.
//  Copyright (c) 2012 Knowhere Studios Inc. All rights reserved.
//

#include "bbAntiField.h"
#include "cinder/Rand.h"
#include "CinderOpenCV.h"

using namespace ci;
using namespace ci::app;
using namespace std;

AntiParticleField::AntiParticleField() :
SceneLayer("AntiField"),
mSceneState(NULL),
mNumParticles(200),
mColor(1,1,1),
mGlobalDecay(0.90f),
mDropping(false),
mDropAccel(0.0f, 1.5f, 0.0f),
mCoordSpace(640.0f, 480.0f),
mBaseAccelX(-1.0f),
mRandomAccelX(-2.0f),
mBaseAccelY(0.0f),
mRandomAccelY(1.0f),
mSlowDownAmt(0.5),
mSlowDownY(0.5)
{
}

// SceneLayer
void AntiParticleField::setup(SceneState* manager)
{
  mSceneState = manager;
	mSceneState->mParams.addSeparator();
	mSceneState->mParams.addParam("Num Particles", &mNumParticles);
  mSceneState->mParams.addParam("FieldColor", &mColor);
	mSceneState->mParams.addParam("GlobalDecay", &mGlobalDecay, "step=0.01");
  
  gl::Texture::Format hiQFormat;
  //	hiQFormat.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
  //	hiQFormat.setMagFilter(GL_LINEAR_MIPMAP_LINEAR);
  texYellow = gl::Texture(loadImage(loadResource ("particle_yellow.png")), hiQFormat);
  texBrown = gl::Texture(loadImage(loadResource ("particle_brown.png")), hiQFormat);
  
  initField();
}

Vec3f AntiParticleField::randScreenVec()
{
  Vec3f res(mRand.nextFloat() * mCoordSpace.x, mRand.nextFloat() * mCoordSpace.y, 0.0f);
  return res;
}

void AntiParticleField::initField()
{
  Rand r;
  mParticlePos.clear();
  mParticleAccel.clear();
  mParticleVel.clear();
  Vec3f zero(0.0f, 0.0f, 0.0f);
  for (int i = 0; i < mNumParticles; i++)
  {
    mParticlePos.push_back(Vec3f(mCoordSpace.x + 10.0f, mCoordSpace.y, 0));
    mParticleAccel.push_back(Vec3f::zero());
    mParticleVel.push_back(zero);
  }
}

void AntiParticleField::setEnabled(bool e)
{
  SceneLayer::setEnabled(e);
  mDropping = false;
  initField();
}

void AntiParticleField::keyDown( cinder::app::KeyEvent event )
{
	if (!mEnabled)
		return;
  switch( event.getChar() )
	{
		case KeyEvent::KEY_m:
      mDropping = true;
      break;
  }
}

void AntiParticleField::tick()
{
  if (!mEnabled)
		return;
  
  if (mNumParticles != mParticlePos.size())
    initField();
  
  if (mDropping)
  {
    updateDrop();
    return;
  }
  
  Rand r;
  std::vector<Blob> blobs = mSceneState->mKinect->getUsers();
  cv::Mat* contour = mSceneState->mKinect->getContourMat();
  
  // BTRTODO: FIX THIS COORDINATE HACK!
  float xs = 1.0f;
  float ys = 1.0f;
  
	for (int i = 0; i < mParticlePos.size(); i++)
	{
    // Acceleration
    mParticleVel[i] *= mGlobalDecay;
    mParticleVel[i] += mParticleAccel[i];
    // Blob interaction
    {
      if (contour->data != NULL)
      {
        int x = constrain(mParticlePos[i].x / xs, 0.0f, 639.0f);
        int y = constrain(mParticlePos[i].y / ys, 0.0f, 479.0f);
        if (contour->at<uint8_t>(cv::Point(x, y)) > 0)
        {
          for (int j = 0; j < blobs.size(); j++)
          {
            if (blobs[j].mBounds.contains(Vec2f(mParticlePos[i].x, mParticlePos[i].y)))
            {
              mParticleVel[i] *= mSlowDownAmt;
              mParticleVel[i].y += mSlowDownY;
            }
          }
        }
      }
    }
  }
  // Update pos
  for (int i = 0; i < mParticlePos.size(); i++)
	{
    mParticlePos[i] += mParticleVel[i];
    if ((mParticlePos[i].x > mCoordSpace.x || mParticlePos[i].x < 0) ||
        mParticlePos[i].y > mCoordSpace.y || mParticlePos[i].y < 0)
    {
      mParticlePos[i] = Vec3f(mCoordSpace.x-1.0f,
                                   mRand.nextFloat() * mCoordSpace.y, 0.0f);
      mParticleAccel[i] = Vec3f((mRand.nextFloat() * mRandomAccelX) + mBaseAccelX,
                                     (mRand.nextFloat() * mRandomAccelY) + mBaseAccelY, 0);
      mParticleVel[i] = Vec3f(0,0,0);
    }
  }
}

void AntiParticleField::updateDrop()
{
  bool particleIn = false;
	for (int i = 0; i < mParticlePos.size(); i++)
	{
		mParticleVel[i] += mDropAccel;
		mParticlePos[i] += mParticleVel[i];
    mParticleVel[i] *= mGlobalDecay;
    particleIn |= (mParticlePos[i].y < mCoordSpace.y);
	}
}

void AntiParticleField::draw()
{
	if (!mEnabled)
		return;
	gl::setMatricesWindowPersp( getWindowWidth(), getWindowHeight());
	gl::enableAlphaBlending();
  
  Vec3f scaleFactor(getWindowWidth() / mCoordSpace.x, getWindowHeight() / mCoordSpace.y, 1.0f);
  
  Vec3f bbUp(0.0f, -1.0f, 0.0f);
  Vec3f bbRight(-1.0f, 0.0f, 0.0f);
  Rand r;
  for (int i = 0; i < 2; i++)
  {
    int startIndex = 0;
    if (i==0)
    {
      startIndex = 0;
      texBrown.enableAndBind();
    } else {
      texYellow.enableAndBind();
      startIndex = 1;
    }
    
    for (int i = startIndex; i < mParticlePos.size(); i+=2)
    {
      gl::color(mColor * r.nextFloat(0.7, 1.0));
      Vec2f velNorm(mParticleVel[i].x, mParticleVel[i].y);
      velNorm.normalize();
      velNorm *= -1.0f;
      Vec2f polar = toPolar(velNorm);
      gl::drawBillboard(mParticlePos[i] * scaleFactor, Vec2f(16.0f, 16.0f), toDegrees(polar.y), bbRight, bbUp);
    }
    if (i==0)
      texBrown.unbind();
    else
      texYellow.unbind();
  }
	gl::disableAlphaBlending();
}

void AntiParticleField::init()
{
  SceneLayer::init();
  
  registerParam("drop");
  registerParam("BaseAccelX", &mBaseAccelX, -2.0f, 0.0f);
  registerParam("RandomAccelX", &mRandomAccelX, -2.0f, 0.0f);
  registerParam("BaseAccelY", &mBaseAccelY, 0.0f, 2.0f);
  registerParam("RandomAccelY", &mRandomAccelY, 0.0f, 2.0f);
  registerParam("SlowDownAmt", &mSlowDownAmt, 0.0f, 1.0f);
  registerParam("SlowDownY", &mSlowDownAmt, 0.0f, 2.0f);
}

void AntiParticleField::update()
{
  SceneLayer::update();
  mDropping = getParamValue("drop") > 0.5f;
}
