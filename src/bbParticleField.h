//
//  bbParticleField.h
//  bobbevy
//
//  Created by Brian Richardson on 7/4/12.
//  Copyright (c) 2012 Knowhere Studios Inc. All rights reserved.
//

#ifndef bobbevy_bbParticleField_h
#define bobbevy_bbParticleField_h

#include "sceneLayer.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Rand.h"
#include "bbKinectWrapper.h"

class ParticleField : public SceneLayer
{
public:
	ParticleField();
  
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
  std::vector<cinder::Vec3f> mParticleGoal;
  cinder::Color mColor;
  cinder::Rand mRand;
  
  cinder::gl::Texture texBrown;
  cinder::gl::Texture texYellow;
  
  float mGoalVel;
  float mAvoidVel;
  float mGlobalDecay;
  float mTargetThreshold;
  float mTargetDecay;
  
  bool mDropping;
  cinder::Vec3f mDropAccel;
  
  void initField();
  cinder::Vec3f randScreenVec();
  void updateDrop();
};


#endif
