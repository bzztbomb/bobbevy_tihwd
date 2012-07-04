/*
 *  bbParticles.h
 *  bobbevy
 *
 *  Created by Brian Richardson on 6/30/12.
 *  Copyright 2012 Knowhere Studios Inc. All rights reserved.
 *
 */

#ifndef BBPARTICLES_H_
#define BBPARTICLES_H_
 
#include "sceneLayer.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include "cinder/gl/GlslProg.h"
#include "bbKinectWrapper.h"

#define NUM_SKELETON_PARTICLES 200

class SkeletonParticles : public SceneLayer
{
public:
	SkeletonParticles();

    void setName(const std::string& name);
    void followUser(KinectWrapper::UserToken ut);
    
	// SceneLayer
    virtual void setup(SceneState* manager);
	virtual void keyDown( cinder::app::KeyEvent event );
	virtual void update();
	virtual void draw();
private:
    std::string mName;
	SceneState* mManager;
    // User
    KinectWrapper::UserToken mUserToken;
    
	cinder::Vec3f mTargetPoint;
	cinder::Vec3f mDirVectors[2];
	std::vector<cinder::Vec3f> mParticlePos;
	std::vector<cinder::Vec3f> mParticleVel;
	std::vector<bool> mReachedTarget;
	float mMaxVel;
	float mAfterTargetAccel;
	bool mSwarm;
	// Swarm
	std::vector<int> mWhichNode;
	std::vector<cinder::Vec3f> mRandOffset;
    std::vector<float> mAge;
    float mAgeMax;
	cinder::Vec3f mRandTarg[6];
	cinder::Vec3f mNodePos[6];
    
    cinder::Color mColor;
	
	void updateSwarm();
	void updateHose();
	void resetParticle(int index);
};

#endif