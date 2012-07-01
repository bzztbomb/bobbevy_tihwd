/*
 *  bbIntroLight.h
 *  bobbevy
 *
 *  Created by Brian Richardson on 6/30/12.
 *  Copyright 2012 Knowhere Studios Inc. All rights reserved.
 *
 */

#ifndef BBINTROLIGHT_H_
#define BBINTROLIGHT_H_

#include "sceneLayer.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include "cinder/gl/GlslProg.h"

class IntroLight : public SceneLayer
{
public:
	IntroLight();
	virtual void setup(SceneState* manager);
	virtual void keyDown( cinder::app::KeyEvent event );
	virtual void update();
	virtual void draw();
	virtual void setEnabled(bool e) { mEnabled = e; mState = lsStart; mLightSize = 0.0f; }
	
	void reveal();
private:
	enum LightState {
		lsStart,
		lsReveal,
	};
	LightState mState;
	SceneState* mManager;
	cinder::gl::GlslProg	mShader;
	cinder::ColorA mLightColor;
	cinder::Anim<float> mLightSize;
	cinder::Anim<float> mLightSizeOffset;
	cinder::Vec3f mSourcePoint;
	cinder::Anim<float> mRevealOffset;
};

#endif