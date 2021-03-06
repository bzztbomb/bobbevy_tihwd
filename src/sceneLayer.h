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
#include "bbDepthProcessor.h"
#include "QTimeline.h"

struct SceneState
{
	cinder::params::InterfaceGl mParams;
	DepthProcessor* mKinect;
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
  virtual void stateChanged(bool active)
  {
    if (!active)
      setEnabled(active);
  }
  
  // QTimelineModule
  virtual void init()
  {
    registerParam("enabled");    
  }
  
  virtual void update()
  {
    bool newEnabled = getParamValue("enabled") > 0.5f;
    if (newEnabled != getEnabled())
      setEnabled(newEnabled);    
  }
protected:
	bool mEnabled;
};

#endif