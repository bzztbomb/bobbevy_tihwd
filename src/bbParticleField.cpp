//
//  bbParticleField.cpp
//  bobbevy
//
//  Created by Brian Richardson on 7/4/12.
//  Copyright (c) 2012 Knowhere Studios Inc. All rights reserved.
//

#include "bbParticleField.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

ParticleField::ParticleField() :
    mSceneState(NULL),
    mNumParticles(200),
    mColor(1,1,1),
    mGoalVel(0.01),
    mTargetThreshold(10),
    mTargetDecay(0.3)
{
}

// SceneLayer
void ParticleField::setup(SceneState* manager)
{
    mSceneState = manager;
	mSceneState->mParams.addSeparator();
	mSceneState->mParams.addParam("Num Particles", &mNumParticles);
	mSceneState->mParams.addParam("Goal vel", &mGoalVel, "step=0.01");
	mSceneState->mParams.addParam("TargetThresh", &mTargetThreshold);
	mSceneState->mParams.addParam("TargetDecay", &mTargetDecay, "step=0.01");
    
    initField();
}

Vec3f ParticleField::randScreenVec() 
{
    Vec3f res(mRand.nextFloat() * getWindowWidth(), mRand.nextFloat() * getWindowHeight(), 0.0f);
    return res;
}

void ParticleField::initField()
{
    Rand r;
    mParticlePos.clear();
    mParticleGoal.clear();
    mParticleVel.clear();
    Vec3f zero(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < mNumParticles; i++)
    {
        mParticlePos.push_back(randScreenVec());
        mParticleGoal.push_back(randScreenVec());
        mParticleVel.push_back(zero);
    }
}

void ParticleField::keyDown( cinder::app::KeyEvent event )
{
}

void ParticleField::update()
{
    if (mNumParticles != mParticlePos.size())
        initField();    
    
    Rand r;
    
	for (int i = 0; i < mParticlePos.size(); i++)
	{
        // Seek towards goal
        Vec3f diff = mParticleGoal[i] - mParticlePos[i];
        if (diff.lengthSquared() < mTargetThreshold*mTargetThreshold)
        {
            mParticleGoal[i] = randScreenVec();
            mParticleVel[i] *= mTargetDecay;
        } else {
            diff.normalize();
            diff *= mGoalVel;
            mParticleVel[i] += diff;
        }
    }
    // Update pos
    for (int i = 0; i < mParticlePos.size(); i++)
	{
        mParticlePos[i] += mParticleVel[i];
    }
}

void ParticleField::draw()
{
	if (!mEnabled)
		return;
	gl::setMatricesWindowPersp( getWindowWidth(), getWindowHeight());
	gl::disableAlphaBlending();
	glDisable(GL_TEXTURE_2D);
    Rand r;
	for (int i = 0; i < mParticlePos.size(); i++)
	{
        gl::color(mColor * r.nextFloat(0.7, 1.0));
		gl::drawSolidRect(Rectf(mParticlePos[i].x-2, mParticlePos[i].y-2,
								mParticlePos[i].x+2, mParticlePos[i].y+2));
	}
	gl::enableAlphaBlending();
}
