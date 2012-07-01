/*
 *  bbParticles.cpp
 *  bobbevy
 *
 *  Created by Brian Richardson on 6/30/12.
 *  Copyright 2012 Knowhere Studios Inc. All rights reserved.
 *
 */

#include "bbParticles.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

//
// SkeletonParticles
//
SkeletonParticles::SkeletonParticles() :
	mManager(NULL),
	mTargetPoint(300.0f, 300.0f, 0.0f),
	mMaxVel(15.0f),
	mAfterTargetAccel(2.0f),
	mSwarm(true)
{
	mDirVectors[0] = Vec3f(-1.0f, 0.0f, 0.0f);
	mDirVectors[1] = Vec3f(1.0f, 0.0f, 0.0f);
	Vec3f zero(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < NUM_SKELETON_PARTICLES; i++)
	{
		mParticlePos.push_back(zero);
		mParticleVel.push_back(zero);
		mReachedTarget.push_back(false);
		mWhichNode.push_back(i%6);
		mRandOffset.push_back(randVec3f());
		mRandOffset[i].z = 0;
		mRandOffset[i].normalize();
	}
	for (int i = 0; i < 6; i++)
		mRandTarg[i] = randVec3f();
}

void SkeletonParticles::setup(SceneState* manager)
{
	mManager = manager;
	mManager->mParams.addParam("Max Particle Vel", &mMaxVel);
	mManager->mParams.addParam("Swarm", &mSwarm);
	for (int i = 0; i < NUM_SKELETON_PARTICLES; i++)
	{
		resetParticle(i);
	}		
}

void SkeletonParticles::keyDown( KeyEvent event )
{
	if (!mEnabled)
		return;
}

void SkeletonParticles::resetParticle(int index)
{
	Vec3f zero(0.0f, 0.0f, 0.0f);
	float radius = 200.0f;
	mParticlePos[index] = randVec3f() * radius;
	mParticlePos[index] += Vec3f(getWindowWidth() + radius*2, getWindowHeight() + radius*2, 0.0f);
	mParticleVel[index] = zero;
	mReachedTarget[index] = false;
}

void SkeletonParticles::updateSwarm()
{
	for (int i = 0; i < NUM_SKELETON_PARTICLES; i++)
	{
		Vec3f targetPos = mNodePos[mWhichNode[i]] + mRandOffset[i] * mMaxVel*8;
		Vec3f accel = targetPos - mParticlePos[i];
		accel.safeNormalize();
		mParticleVel[i] += accel*mMaxVel*0.3f;
		mParticleVel[i].limit(mMaxVel);
		mParticlePos[i] += mParticleVel[i];
		Vec3f l = mNodePos[mWhichNode[i]] - mParticlePos[i];
		if (l.lengthSquared() < (mMaxVel*8)*(mMaxVel*8))
		{
			mWhichNode[i] = (mWhichNode[i]+1)%6;
		}
		// Check for off screen and respawn
		float x = mParticlePos[i].x;
		float y = mParticlePos[i].y;
		//		if (x < 0 || x > getWindowWidth() || y < 0 || y > getWindowHeight())
		//			resetParticle(i);		
	}
}

void SkeletonParticles::updateHose()
{
	// Calculate new particle velocities and positions
	for (int i = 0; i < NUM_SKELETON_PARTICLES; i++)
	{
		Vec3f targetPoint = mTargetPoint + mRandOffset[i] * mMaxVel;
		if (!mReachedTarget[i])
		{
			Vec3f len = targetPoint - mParticlePos[i];
			Vec3f accel = len;
			accel.safeNormalize();
			mParticleVel[i] += accel*mMaxVel*0.3f;
			mParticleVel[i].limit(mMaxVel);
		} else {
			int dir = i%2 ? 0 : 1;
			mParticleVel[i] += mDirVectors[dir]*mAfterTargetAccel;
			mParticleVel[i].limit(mMaxVel*2);
		}
		mParticlePos[i] += mParticleVel[i];
		if (!mReachedTarget[i])
		{
			Vec3f l = targetPoint - mParticlePos[i];
			mReachedTarget[i] = (l.lengthSquared() < (mMaxVel*8)*(mMaxVel*8));
		} else {
			// Check for off screen and respawn
			float x = mParticlePos[i].x;
			float y = mParticlePos[i].y;
			if (x < 0 || x > getWindowWidth() || y < 0 || y > getWindowHeight())
				resetParticle(i);
		}
	}	
}

void SkeletonParticles::update()
{
	if (!mEnabled)
		return;
	
	// Calc target point, left and right vectors
	Blob* user = mManager->mKinect->getClosestUser();
	if (user != NULL)
	{
		float r = sqrt(user->mContourArea);
		mTargetPoint.x =  user->mCentroid.x;
		mTargetPoint.y =  user->mCentroid.y;
		mTargetPoint.z = 0;
		mNodePos[0] = mTargetPoint;
		for (int i = 1; i < 6; i++)
			mNodePos[i] = mTargetPoint + (mRandTarg[i] * r);
		
		mDirVectors[0] = mTargetPoint;
		mDirVectors[1] = mTargetPoint;
		for (int i = 0; i < 2; i++)
		{
			mDirVectors[i] -= mTargetPoint;
			mDirVectors[i].normalize();
			//mDirVectors[i] *= mAfterTargetAccel;
		}
	}
	if (mSwarm)
		updateSwarm();
	else
		updateHose();
}

void SkeletonParticles::draw()
{
	if (!mEnabled)
		return;
	gl::setMatricesWindowPersp( getWindowWidth(), getWindowHeight());
	gl::disableAlphaBlending();
	for (int i = 0; i < NUM_SKELETON_PARTICLES; i++)
	{
		if (mReachedTarget[i])
			gl::color(cinder::ColorA(1, 0, 0, 1));
		else
			gl::color(cinder::ColorA(1, 1, 1, 1));
		gl::drawSolidRect(Rectf(mParticlePos[i].x-5, mParticlePos[i].y-5,
								mParticlePos[i].x+5, mParticlePos[i].y+5));
	}
	gl::enableAlphaBlending();
}
