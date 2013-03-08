/*
 *  sceneLayer.h
 *  bobbevy
 *
 *  Created by Brian Richardson on 6/30/12.
 *  Copyright 2012 Knowhere Studios Inc. All rights reserved.
 *
 */

#ifndef __SCENELAYER_H_
#define __SCENELAYER_H_

#include "cinder/params/Params.h"
#include "cinder/Timeline.h"
#include "cinder/app/AppBasic.h"
#include "cinder/Color.h"
#include "bbKinectWrapper.h"
#include "QTimeline.h"

struct SceneState
{
	cinder::params::InterfaceGl mParams;
	cinder::TimelineRef	mTimeline;
  cinder::ColorA mBlackoutColor;
	cinder::Anim<float> mBlackoutAmount;
	KinectWrapper* mKinect;        
};

class SceneLayer : public QTimelineModule
{
public:
	SceneLayer(const std::string& type) :
    QTimelineModule(type),
    mEnabled(false)
  {    
  }
	
	virtual void setup(SceneState* sceneState) {}
	virtual void tick() {}
	virtual void draw() {}
	virtual void keyDown( cinder::app::KeyEvent event ) {}
	virtual void setEnabled(bool e) { mEnabled = e; }
	virtual bool getEnabled() { return mEnabled; }

  virtual void render() { draw(); }
  virtual void activeChanged(bool active)
  {
    if (!active)
      setEnabled(active);
  }
protected:
	bool mEnabled;
};

#endif