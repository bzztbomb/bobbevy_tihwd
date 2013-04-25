//
//  bbLines.h
//  bobbevy
//
//  Created by Brian Richardson on 4/19/13.
//
//

#ifndef __bobbevy__bbLines__
#define __bobbevy__bbLines__

#include "sceneLayer.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"

class LineLayer : public SceneLayer
{
public:
  LineLayer();

  // SceneLayer
  virtual void draw();
	virtual void setup(SceneState* sceneState);
  virtual void tick();
  
  // QTimelineModule
  virtual void init();
private:
  cinder::gl::GlslProg mShader;
  SceneState* mSceneState;
  cinder::Vec2f mPos[2];
  float mFilterAmount;
  float mTimeMult;
  float mRays;
  float mPeriodDivider;
  float mInnerRadius;
};



#endif /* defined(__bobbevy__bbLines__) */
