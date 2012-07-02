/*
 *  bbIntroLight.cpp
 *  bobbevy
 *
 *  Created by Brian Richardson on 6/30/12.
 *  Copyright 2012 Knowhere Studios Inc. All rights reserved.
 *
 */

#include "bbIntroLight.h"

using namespace ci;
using namespace ci::app;
using namespace std;

//
// IntroLight
//
IntroLight::IntroLight() :
	mManager(NULL),
	mLightColor(150.0f / 255.0, 130.0f / 255.0f, 130.0f / 255.0f, 1.0f),
	mLightSize(0.0f),
	mLightSizeOffset(0.0),
	mSourcePoint(400, 0, 0),
	mRevealOffset(0.0f),
	mTargetLightSize(100.0f),
	mState(lsStart)
{
}

void IntroLight::setup(SceneState* manager)
{
	mManager = manager;
	mManager->mParams.addParam("LightColor", &mLightColor);
	mManager->mParams.addParam("LightPos", &mSourcePoint);
	mManager->mParams.addParam("LightSize", &mLightSize());
	mShader = gl::GlslProg( loadAsset( "introLightVert.glsl" ), loadAsset( "introLightFrag.glsl" ) );
	mManager->mTimeline->apply(&mLightSizeOffset, 4.0f, 2.0f).pingPong(true).infinite(true);
}

void IntroLight::reveal()
{
	mState = lsReveal;
	mRevealOffset = 0.0f;
	mManager->mTimeline->apply(&mRevealOffset, (float) getWindowWidth(), 5.0f);
	// BTRTODO: FIX THIS
	//mManager->mTreeLayer.resetParams();
}

void IntroLight::keyDown( KeyEvent event )
{
	if (!mEnabled)
		return;
	
	switch (event.getCode())
	{
		case KeyEvent::KEY_r:
			reveal();
			break;
		case KeyEvent::KEY_k:
			if (mLightSize == 0.0f)
			{
				mManager->mTimeline->apply(&mLightSize, mTargetLightSize, 3.0f);
			} else {
				mLightSize = 0.0f;
			}
			break;
	}
}

float lerp(float a, float b, float t)
{
	return a + t*(b-a);
}

void IntroLight::update()
{
	Blob* user = mManager->mKinect->getFurtherUser();
	if (user != NULL)
	{
		mSourcePoint.x = lerp(mSourcePoint.x, user->mCentroid.x, 0.1);
		mSourcePoint.y = lerp(mSourcePoint.y, user->mCentroid.y, 0.1);
	} else {
		mSourcePoint.x = getWindowWidth() + 1000;
	}
	if (mState == lsReveal)
	{
		if (mRevealOffset == getWindowWidth())
			mEnabled = false;
	}
}

void IntroLight::draw()
{
	if (!mEnabled)
		return;
	gl::setMatricesWindowPersp( getWindowWidth(), getWindowHeight());
	float lightSize = (mLightSize > 0.0f) ? max(mLightSize + mLightSizeOffset, 0.0f) : 0.0f;
	gl::color(cinder::ColorA(0, 0, 0, 1));
	gl::enableAlphaBlending();
	if (mState == lsStart)
	{
		gl::drawSolidRect(getWindowBounds());
		mShader.bind();
		mShader.uniform("lightColor", mLightColor);
		Rectf t(mSourcePoint.x - lightSize, 0, mSourcePoint.x + lightSize, getWindowHeight());
		gl::drawSolidRect(t);
		mShader.unbind();
	} else {
		float xLeft = mSourcePoint.x-mRevealOffset;
		float xRight = mSourcePoint.x+mRevealOffset;
		gl::drawSolidRect(Rectf(0, 0, xLeft, getWindowHeight()));
		gl::drawSolidRect(Rectf(xRight, 0, getWindowWidth(), getWindowHeight()));
		
		mShader.bind();
		mShader.uniform("lightColor", mLightColor);
		gl::drawSolidRect(Rectf(xLeft - lightSize, 0, xLeft + lightSize, getWindowHeight()));
		gl::drawSolidRect(Rectf(xRight - lightSize, 0, xRight + lightSize, getWindowHeight()));
		mShader.unbind();
	}
}
