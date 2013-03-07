//
//  bbParticleField.cpp
//  bobbevy
//
//  Created by Brian Richardson on 7/4/12.
//  Copyright (c) 2012 Knowhere Studios Inc. All rights reserved.
//

#include "bbParticleField.h"
#include "cinder/Rand.h"
#include "CinderOpenCV.h"

using namespace ci;
using namespace ci::app;
using namespace std;

ParticleField::ParticleField() :
mSceneState(NULL),
mNumParticles(200),
mColor(1,1,1),
mGoalVel(0.65f),
mAvoidVel(10.0f),
mGlobalDecay(0.90f),
mTargetThreshold(10.0f),
mTargetDecay(0.3f),
mDropping(false),
mDropAccel(0.0f, 1.5f, 0.0f)
{
}

// SceneLayer
void ParticleField::setup(SceneState* manager)
{
  mSceneState = manager;
	mSceneState->mParams.addSeparator();
	mSceneState->mParams.addParam("Num Particles", &mNumParticles);
  mSceneState->mParams.addParam("FieldColor", &mColor);
	mSceneState->mParams.addParam("Goal vel", &mGoalVel, "step=0.01");
	mSceneState->mParams.addParam("AvoidVel", &mAvoidVel, "step=0.01");
	mSceneState->mParams.addParam("GlobalDecay", &mGlobalDecay, "step=0.01");
	mSceneState->mParams.addParam("TargetThresh", &mTargetThreshold);
	mSceneState->mParams.addParam("TargetDecay", &mTargetDecay, "step=0.01");
  
  gl::Texture::Format hiQFormat;
  //	hiQFormat.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
  //	hiQFormat.setMagFilter(GL_LINEAR_MIPMAP_LINEAR);
  texYellow = gl::Texture(loadImage(loadResource ("particle_yellow.png")), hiQFormat);
  texBrown = gl::Texture(loadImage(loadResource ("particle_brown.png")), hiQFormat);
  
  initField();
}

Vec3f ParticleField::randScreenVec()
{
  Vec3f res(mRand.nextFloat() * getWindowWidth(), mRand.nextFloat() * getWindowHeight(), 0.0f);
  return res;
}

void ParticleField::initField()
{
  Rand r;
  mParticlePos.clear();
  mParticleGoal.clear();
  mParticleVel.clear();
  Vec3f zero(0.0f, 0.0f, 0.0f);
  for (int i = 0; i < mNumParticles; i++)
  {
    //        mParticlePos.push_back(randScreenVec());
    mParticlePos.push_back(Vec3f(mRand.nextFloat() * getWindowWidth(), 0.0f, 0.0f));
    mParticleGoal.push_back(randScreenVec());
    mParticleVel.push_back(zero);
  }
}

void ParticleField::setEnabled(bool e)
{
  SceneLayer::setEnabled(e);
  mDropping = false;
  initField();
}

void ParticleField::keyDown( cinder::app::KeyEvent event )
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

void ParticleField::tick()
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
  float xs = (float) getWindowWidth() / 640.0f;
  float ys = (float) getWindowHeight() / 480.0f;
  
	for (int i = 0; i < mParticlePos.size(); i++)
	{
    mParticleVel[i] *= mGlobalDecay;
    // Seek towards goal
    {
      Vec3f diff = mParticleGoal[i] - mParticlePos[i];
      if (diff.lengthSquared() < mTargetThreshold*mTargetThreshold)
      {
        mParticleGoal[i] = randScreenVec();
        mParticleVel[i] *= mTargetDecay;
      } else {
        diff.normalize();
        diff *= mGoalVel;
        mParticleVel[i] += diff;
      }
    }
    // Away from blobs!
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
              Vec3f diff = Vec3f(blobs[j].mCentroid.x,blobs[j].mCentroid.y, 0)  - mParticlePos[i];
              diff.normalize();
              diff *= -mAvoidVel;
              mParticleVel[i] += diff;
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
    if ((mParticlePos[i].x > getWindowWidth() || mParticlePos[i].x < 0) ||
        mParticlePos[i].y > getWindowHeight() || mParticlePos[i].y < 0)
    {
      mParticlePos[i] = randScreenVec();
      mParticleGoal[i] = randScreenVec();
      mParticleVel[i] = Vec3f(0,0,0);
      for (int j = 0; j < blobs.size(); j++)
      {
        if (blobs[j].mBounds.contains(Vec2f(mParticlePos[i].x, mParticlePos[i].y)))
        {
          mParticlePos[i] -= Vec3f(blobs[j].mCentroid.x, blobs[j].mCentroid.y, 0.0f);
        }
      }
    }
  }
}

void ParticleField::updateDrop()
{
  bool particleIn = false;
	for (int i = 0; i < mParticlePos.size(); i++)
	{
		mParticleVel[i] += mDropAccel;
		mParticlePos[i] += mParticleVel[i];
    mParticleVel[i] *= mGlobalDecay;
    particleIn |= (mParticlePos[i].y < getWindowHeight());
	}
  if (!particleIn)
    setEnabled(false);
}

void ParticleField::draw()
{
	if (!mEnabled)
		return;
	gl::setMatricesWindowPersp( getWindowWidth(), getWindowHeight());
	gl::enableAlphaBlending();
    
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
      gl::drawBillboard(mParticlePos[i], Vec2f(16.0f, 16.0f), toDegrees(polar.y), bbRight, bbUp);
    }
    if (i==0)
      texBrown.unbind();
    else
      texYellow.unbind();
  }
	gl::disableAlphaBlending();
}
