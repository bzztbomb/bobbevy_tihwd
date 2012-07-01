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

struct SceneState
{
	cinder::params::InterfaceGl mParams;
	cinder::TimelineRef	mTimeline;
	cinder::Anim<float> mBlackoutAmount;
};

class SceneLayer
{
public:
	SceneLayer() { mEnabled = false; }
	
	virtual void setup(SceneState* sceneState) {}
	virtual void update() {}
	virtual void draw() {}
	virtual void keyDown( cinder::app::KeyEvent event ) {}
	virtual void setEnabled(bool e) { mEnabled = e; }
	virtual bool getEnabled() { return mEnabled; }
protected:
	bool mEnabled;
};

#endif