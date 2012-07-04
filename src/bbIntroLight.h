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

#include <list>
#include "sceneLayer.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include "cinder/gl/GlslProg.h"
#include "bbKinectWrapper.h"

class IntroLight : public SceneLayer
{
public:
	IntroLight();

    void reveal();
    void followUser(KinectWrapper::UserToken ut);
    
    // SceneLayer
	virtual void setup(SceneState* manager);
	virtual void keyDown( cinder::app::KeyEvent event );
	virtual void update();
	virtual void draw();
	virtual void setEnabled(bool e) { mEnabled = e; mState = lsStart; mLightSize = 0.0f; }
private:
    static const int smMedianFilterSize;
    
	enum LightState {
		lsStart,
		lsReveal,
	};
	LightState mState;
	SceneState* mManager;
	cinder::gl::GlslProg	mShader;
	cinder::ColorA mLightColor;
	cinder::Anim<float> mLightSize;
	float mTargetLightSize;
	cinder::Anim<float> mLightSizeOffset;
	cinder::Vec3f mSourcePoint;
    std::list<float> mPointFilter;
	cinder::Anim<float> mRevealOffset;
    KinectWrapper::UserToken mUserToken;
};

#endif