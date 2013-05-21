//
//  bbFeedback.h
//  bobbevy
//
//  Created by Brian Richardson on 5/9/13.
//
//

#ifndef __bobbevy__bbFeedback__
#define __bobbevy__bbFeedback__

#include "sceneLayer.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"

class FeedbackLayer : public SceneLayer
{
public:
	FeedbackLayer();
  
  // SceneLayer
  virtual void setup(SceneState* sceneState);
  virtual void draw();
  virtual void tick();
  
  // QTimelineModule
  virtual void init();
private:
  enum
  {
    NUM_FRAMES = 2
  };
  SceneState* mSceneState;
  int mPrevFrame;
  bool mFboInit;
  cinder::gl::Fbo mFrames[NUM_FRAMES];
  cinder::gl::GlslProg mShader;
  float mInOut;
  float mNoiseAmp;
  
  void initFbo();
};


#endif /* defined(__bobbevy__bbFeedback__) */
