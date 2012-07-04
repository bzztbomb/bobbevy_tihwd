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
	mMaxVel(20.0f),
	mAfterTargetAccel(2.0f),
	mSwarm(true),
    mUserToken(KinectWrapper::utClosest),
    mColor(1,1,1),
    mAgeMax(60.0f)
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
        mAge.push_back(0.0f);
	}
	for (int i = 0; i < 6; i++)
		mRandTarg[i] = randVec3f() * randFloat(0.1, 0.9);
}

void SkeletonParticles::followUser(KinectWrapper::UserToken ut)
{
    mUserToken = ut;
}

void SkeletonParticles::setName(const std::string& name)
{
    mName = name;
}

void SkeletonParticles::setup(SceneState* manager)
{
	mManager = manager;
	mManager->mParams.addSeparator();
	mManager->mParams.addParam(mName + ": Max Particle Vel", &mMaxVel);
	mManager->mParams.addParam(mName + ": Max Age", &mAgeMax);
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
	if (mSwarm)	
		mParticlePos[index] += Vec3f(getWindowWidth() + radius*2, getWindowHeight() + radius*2, 0.0f);
	else
	{
		mParticlePos[index].x = mTargetPoint.x;
		mParticlePos[index].y = getWindowHeight()-1;
	}
	mParticleVel[index] = zero;
	mReachedTarget[index] = false;
}

void SkeletonParticles::updateSwarm()
{
    Rand r;
	for (int i = 0; i < NUM_SKELETON_PARTICLES; i++)
	{
        mAge[i] += r.nextFloat(0.5, 1.0f);
		Vec3f targetPos = mNodePos[mWhichNode[i]] + mRandOffset[i] * mMaxVel*8;
		Vec3f accel = targetPos - mParticlePos[i];
		accel.safeNormalize();
		mParticleVel[i] += accel*mMaxVel*0.3f;
		mParticleVel[i].limit(mMaxVel);
		mParticlePos[i] += mParticleVel[i];
		Vec3f l = mNodePos[mWhichNode[i]] - mParticlePos[i];
		if (mAge[i] > mAgeMax)
		{
            mAge[i] = 0;
			mWhichNode[i] = r.nextInt(6);
		}
	}
}

void SkeletonParticles::update()
{
	if (!mEnabled)
		return;
	
	// Calc target point, left and right vectors
	Blob* user = mManager->mKinect->getUser(mUserToken);
	if (user != NULL)
	{
		float r = sqrt(user->mContourArea) / 2.0f;
		mTargetPoint.x =  user->mCentroid.x;
		mTargetPoint.y =  user->mCentroid.y;
		mTargetPoint.z = 0;
		mNodePos[0] = mTargetPoint;
		for (int i = 1; i < 6; i++)
			mNodePos[i] = mTargetPoint + (mRandTarg[i] * r);
		
		mDirVectors[0] = user->mRightMost;
		mDirVectors[1] = user->mLeftMost;
		for (int i = 0; i < 2; i++)
		{
			mDirVectors[i] -= mTargetPoint;
			mDirVectors[i].normalize();
			mDirVectors[i] *= mAfterTargetAccel;
		}
	}
    updateSwarm();
}

void SkeletonParticles::draw()
{
	if (!mEnabled)
		return;
	gl::setMatricesWindowPersp( getWindowWidth(), getWindowHeight());
	gl::disableAlphaBlending();
	glDisable(GL_TEXTURE_2D);
    Rand r;
	for (int i = 0; i < NUM_SKELETON_PARTICLES; i++)
	{
        gl::color(mColor * r.nextFloat(0.7, 1.0));
		gl::drawSolidRect(Rectf(mParticlePos[i].x-2, mParticlePos[i].y-2,
								mParticlePos[i].x+2, mParticlePos[i].y+2));
	}
	gl::enableAlphaBlending();
}
