//
//  bbParticleField.cpp
//  bobbevy
//
//  Created by Brian Richardson on 7/4/12.
//  Copyright (c) 2012 Knowhere Studios Inc. All rights reserved.
//

#include "bbParticleField.h"
#include "cinder/Rand.h"
#include "CinderOpenCV.h"

using namespace ci;
using namespace ci::app;
using namespace std;

ParticleField::ParticleField() :
    mSceneState(NULL),
    mNumParticles(200),
    mColor(1,1,1),
    mGoalVel(0.65f),
    mAvoidVel(2.0f),
    mGlobalDecay(0.90f),
    mTargetThreshold(10.0f),
    mTargetDecay(0.3f)
{
}

// SceneLayer
void ParticleField::setup(SceneState* manager)
{
    mSceneState = manager;
	mSceneState->mParams.addSeparator();
	mSceneState->mParams.addParam("Num Particles", &mNumParticles);
    mSceneState->mParams.addParam("FieldColor", &mColor);
	mSceneState->mParams.addParam("Goal vel", &mGoalVel, "step=0.01");
	mSceneState->mParams.addParam("AvoidVel", &mAvoidVel, "step=0.01");
	mSceneState->mParams.addParam("GlobalDecay", &mGlobalDecay, "step=0.01");
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
    if (!mEnabled)
		return;

    if (mNumParticles != mParticlePos.size())
        initField();    
    
    Rand r;
    std::vector<Blob> blobs = mSceneState->mKinect->getUsers();
    cv::Mat* contour = mSceneState->mKinect->getContourMat();

    // BTRTODO: FIX THIS COORDINATE HACK!
    float xs = (float) getWindowWidth() / 640.0f;
    float ys = (float) getWindowHeight() / 480.0f;
    
	for (int i = 0; i < mParticlePos.size(); i++)
	{
        mParticleVel[i] *= mGlobalDecay;
        // Seek towards goal
        {
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
        // Away from blobs!
        {
            if (contour->data != NULL)
            {
                int x = constrain(mParticlePos[i].x / xs, 0.0f, 639.0f);
                int y = constrain(mParticlePos[i].y / ys, 0.0f, 479.0f);
                if (contour->at<uint8_t>(cv::Point(x, y)) > 0) 
                {
                    for (int j = 0; j < blobs.size(); j++)
                    {
                        if (blobs[j].mBounds.contains(Vec2f(mParticlePos[i].x, mParticlePos[i].y)))
                        {
                            Vec3f diff = Vec3f(blobs[j].mCentroid.x,blobs[j].mCentroid.y, 0)  - mParticlePos[i];
                            diff.normalize();
                            diff *= -mAvoidVel;
                            mParticleVel[i] += diff;
                        }
                    }
                }
            }
        }
    }
    // Update pos
    for (int i = 0; i < mParticlePos.size(); i++)
	{
        mParticlePos[i] += mParticleVel[i];
        if ((mParticlePos[i].x > getWindowWidth() || mParticlePos[i].x < 0) || 
            mParticlePos[i].y > getWindowHeight() || mParticlePos[i].y < 0)
        {
            mParticlePos[i] = randScreenVec();
            mParticleGoal[i] = randScreenVec();
            mParticleVel[i] = Vec3f(0,0,0); 
            for (int j = 0; j < blobs.size(); j++)
            {
                if (blobs[j].mBounds.contains(Vec2f(mParticlePos[i].x, mParticlePos[i].y)))
                {
                    mParticlePos[i] -= Vec3f(blobs[j].mCentroid.x, blobs[j].mCentroid.y, 0.0f);
                }
            }
        }
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
