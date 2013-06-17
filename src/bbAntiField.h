//
//  bbParticleField.h
//  bobbevy
//
//  Created by Brian Richardson on 7/4/12.
//  Copyright (c) 2012 Knowhere Studios Inc. All rights reserved.
//

#ifndef bobbevy_bbAntiParticleField_h
#define bobbevy_bbAntiParticleField_h

#include "sceneLayer.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Rand.h"
#include "bbDepthProcessor.h"

class AntiParticleField : public SceneLayer
{
public:
	AntiParticleField();
  
	// SceneLayer
  virtual void setup(SceneState* manager);
	virtual void keyDown( cinder::app::KeyEvent event );
	virtual void tick();
	virtual void draw();
  virtual void setEnabled(bool e);
  
  // TimelineModule
  virtual void init();
  virtual void update();
private:
	SceneState* mSceneState;
  int mNumParticles;
  std::vector<cinder::Vec3f> mParticlePos;
  std::vector<cinder::Vec3f> mParticleVel;
  std::vector<cinder::Vec3f> mParticleAccel;
  cinder::Color mColor;
  cinder::Rand mRand;
  cinder::Vec2f mCoordSpace;
  
  cinder::gl::Texture texBrown;
  cinder::gl::Texture texYellow;
  
  float mGlobalDecay;
  float mBaseAccelX;
  float mRandomAccelX;
  float mBaseAccelY;
  float mRandomAccelY;
  float mSlowDownAmt;
  float mSlowDownY;
  float mTimeMult;
  float mLfoAmp;
  float mVelMult;
  
  bool mDropping;
  cinder::Vec3f mDropAccel;
  
  void initField();
  cinder::Vec3f randScreenVec();
  void updateDrop();
};


#endif
