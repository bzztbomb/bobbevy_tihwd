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


Vec3f dumbRand()
{
    Vec3f r = randVec3f();
    r.z = 0;
    r.normalize();
    return r;
}

//
// SkeletonParticles
//
SkeletonParticles::SkeletonParticles() :
	mManager(NULL),
	mTargetPoint(300.0f, 300.0f, 0.0f),
	mMaxVel(0.8f),
	mAfterTargetAccel(2.0f),
	mSwarm(true),
    mUserToken(KinectWrapper::utClosest),
    mColor(1,1,1),
    mDrag(0.9f),
    mTargetDrag(1.0f),
    mDistanceThresh(3.0f),
    mDropping(false),
    mDropAccel(0.0f, 0.5f, 0.0f),
    mNumParticles(200),
    mNumPositions(100),
    mBounds(100,100,600,600)
{
	Vec3f zero(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < mNumParticles; i++)
	{
		mParticlePos.push_back(zero);
		mParticleVel.push_back(zero);
		mReachedTarget.push_back(false);
		mWhichNode.push_back(i%mNumPositions);
		mRandOffset.push_back(dumbRand());
		mRandOffset[i].z = 0;
		mRandOffset[i].normalize();
	}
    for (int i = 0; i < mNumPositions; i++)
    {
        mNodePos.push_back(Vec3f(randFloat(), randFloat(), 0.0f)); 
    }
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
    gl::Texture::Format hiQFormat;
//	hiQFormat.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
//	hiQFormat.setMagFilter(GL_LINEAR_MIPMAP_LINEAR);
    texYellow = gl::Texture(loadImage(loadResource ("particle_yellow.png")), hiQFormat);
    texBrown = gl::Texture(loadImage(loadResource ("particle_brown.png")), hiQFormat);
    
	mManager->mParams.addSeparator();
	mManager->mParams.addParam(mName + ": Max Particle Vel", &mMaxVel, "step=0.1");
	mManager->mParams.addParam(mName + ": Drag", &mDrag, "step=0.1");
	mManager->mParams.addParam(mName + ": TargetDrag", &mTargetDrag, "step=0.1");
	mManager->mParams.addParam(mName + ": DistanceThresh", &mDistanceThresh);
	mManager->mParams.addParam(mName + ": Color", &mColor);
	mManager->mParams.addParam(mName + ": DropAccel", &mDropAccel);
	mManager->mParams.addParam(mName + ": NumParticles", &mNumParticles);

	for (int i = 0; i < NUM_SKELETON_PARTICLES; i++)
	{
		resetParticle(i);
	}		
}

void SkeletonParticles::keyDown( KeyEvent event )
{
	if (!mEnabled)
		return;
    switch( event.getChar() )
	{
		case KeyEvent::KEY_m:
            mDropping = true;
            break;
    }
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
    mWhichNode[index] = randInt(mNumPositions);
}

void SkeletonParticles::updateSwarm()
{
    Rand r;

	for (int i = 0; i < NUM_SKELETON_PARTICLES; i++)
	{
		Vec3f targetPos = mNodePos[mWhichNode[i]]; 
        targetPos.x *= mBounds.x2 - mBounds.x1;
        targetPos.x += mBounds.x1;
        targetPos.y *= mBounds.y2 - mBounds.y1;
        targetPos.y += mBounds.y1;
        Vec3f l = targetPos;
		Vec3f accel = targetPos - mParticlePos[i];
		accel.safeNormalize();
		mParticleVel[i] += accel*mMaxVel;
		mParticlePos[i] += mParticleVel[i];
		l -= mParticlePos[i];
		if (l.lengthSquared() < (mDistanceThresh*mDistanceThresh))
		{
            mParticleVel[i] *= mTargetDrag;
			mWhichNode[i] = randInt(mNumPositions);
		}
        mParticleVel[i] *= mDrag;
	}
}

void SkeletonParticles::updateDrop()
{
    bool particleIn = false;
	for (int i = 0; i < NUM_SKELETON_PARTICLES; i++)
	{
		mParticleVel[i] += mDropAccel;
		mParticlePos[i] += mParticleVel[i];
        mParticleVel[i] *= mDrag;
        particleIn |= (mParticlePos[i].y < getWindowHeight());            
	}
    if (!particleIn)
        setEnabled(false);
}

void SkeletonParticles::update()
{
	if (!mEnabled)
		return;
	
	// Calc target point, left and right vectors
	Blob* user = mManager->mKinect->getUser(mUserToken);
	if (user != NULL)
	{
        mBounds = user->mBounds;
	} 
    if (!mDropping)
        updateSwarm();
    else
        updateDrop();
}

void SkeletonParticles::draw()
{
	if (!mEnabled)
		return;
	gl::setMatricesWindowPersp( getWindowWidth(), getWindowHeight());
	gl::enableAlphaBlending();
    Vec3f bbUp(0.0f, -1.0f, 0.0f); 
    Vec3f bbRight(-1.0f, 0.0f, 0.0f);

    Rand r;
    for (int i = 0; i < 2; i++)
    {
        int startIndex = 0;
        if (i==0)
        {
            startIndex = 0;
            texBrown.enableAndBind();
        } else {
            texYellow.enableAndBind();
            startIndex = 1;
        }

        for (int i = startIndex; i < NUM_SKELETON_PARTICLES; i+=2)
        {
            gl::color(mColor * r.nextFloat(0.7, 1.0));
            Vec2f velNorm(mParticleVel[i].x, mParticleVel[i].y);
            velNorm.normalize();
            velNorm *= -1.0f;
            Vec2f polar = toPolar(velNorm);
            gl::drawBillboard(mParticlePos[i], Vec2f(16.0f, 16.0f), toDegrees(polar.y), bbRight, bbUp);
        }
        if (i==0)
            texBrown.unbind();
        else
            texYellow.unbind();
    }
	gl::disableAlphaBlending();
}

void SkeletonParticles::setEnabled(bool e)
{
    SceneLayer::setEnabled(e);
    if (mEnabled)
    {
        for (int i = 0; i < NUM_SKELETON_PARTICLES; i++)
        {
            resetParticle(i);
        }
        mDropping = false;
    }
}