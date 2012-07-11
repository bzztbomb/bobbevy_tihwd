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
    void drop();
    
	// SceneLayer
    virtual void setup(SceneState* manager);
	virtual void keyDown( cinder::app::KeyEvent event );
	virtual void update();
	virtual void draw();
    virtual void setEnabled(bool e);
private:
    static const int smNumOffsets;
    std::string mName;
	SceneState* mManager;
    // User
    KinectWrapper::UserToken mUserToken;
    
    cinder::gl::Texture texBrown;
    cinder::gl::Texture texYellow;

    int mNumParticles;
    int mNumPositions;
	cinder::Vec3f mTargetPoint;
	std::vector<cinder::Vec3f> mParticlePos;
	std::vector<cinder::Vec3f> mParticleVel;
	std::vector<bool> mReachedTarget;
	float mMaxVel;
	float mAfterTargetAccel;
	bool mSwarm;
	// Swarm
	std::vector<int> mWhichNode;
	std::vector<cinder::Vec3f> mRandOffset;
    std::vector<cinder::Vec3f> mNodePos;
    cinder::Rectf mBounds;
    float mDrag;
    float mTargetDrag;
    float mDistanceThresh;
    bool mDropping;
    cinder::Vec3f mDropAccel;
    
    cinder::Color mColor;
	
	void updateSwarm();
    void updateDrop();
	void updateHose();
	void resetParticle(int index);
};

#endif