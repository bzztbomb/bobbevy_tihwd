/*
 *  bbTreeLayer.h
 *  bobbevy
 *
 *  Created by Brian Richardson on 6/30/12.
 *  Copyright 2012 Knowhere Studios Inc. All rights reserved.
 *
 */

#ifndef BBTREELAYER_H_
#define BBTREELAYER_H_

#include "sceneLayer.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/TriMesh.h"
#include "cinder/Camera.h"
#include "bbParticles.h"

class TreeLayer : public SceneLayer
{
public:
	TreeLayer();

    void setBlurred();
    void setLeaves(bool l);    
	void resetParams();
    void setSwarms(SkeletonParticles* swarm0, SkeletonParticles* swarm1);
    
	// SceneLayer
    virtual void setup(SceneState* manager);
	virtual void keyDown( cinder::app::KeyEvent event );
	virtual void update();
	virtual void draw();	
	virtual void setEnabled(bool e);	
private:
	SceneState* mManager;	
	// Trees
	cinder::gl::Texture texSun;
	cinder::gl::Texture texGround;
    cinder::gl::Texture texClip;
	cinder::gl::Texture texTree;
	cinder::gl::Texture texTreeWithLeaves;    
	cinder::gl::Texture texOverlay;
	bool mTreesEnabled;
    bool mWithLeaves;
	int mNumTrees;
	float mTreeRadius;
	float mTreeSizeVariance;
	cinder::Vec3f mTreePan;
	cinder::Anim<cinder::Vec3f> mTreePanSpeed;
	cinder::CameraPersp mTreeCam;
	cinder::TriMesh mGroundMesh;
	cinder::TriMesh mTreeMesh;
	bool mUpdateTrees;
	cinder::Vec3f travelBounds;
	cinder::Color mGroundColor;
	cinder::Color mSunColor;
	// Zoom to black
	bool mZoomToBlack;
	cinder::Vec3f mZoomTarget;
	cinder::gl::Texture texBlack;
    // Fade in
    
    cinder::gl::GlslProg mTreeShader;    
    cinder::Anim<float> mBlurAmount;
    cinder::Anim<float> mFadeAmount;
    cinder::Anim<float> mWarpAmount;
    cinder::Anim<float> mAlphaAmount;
    float mTimeMult;
    float mTime;
    float mYMult;
    float mFadeTransTime;
    float mZoomOffset;
    
    
    cinder::Anim<cinder::Color> mFogColor;
    cinder::Anim<float> mFogDistance;
    cinder::Anim<float> mFogHeight;
    
    static const int NUM_SWARMS;
    SkeletonParticles* mSwarm[2];
	
	void drawTrees();
	void initGroundMesh();
	void initTreeMesh();
	void addTree(const cinder::Vec3f& treePos, const cinder::Vec2f& treeScale, const cinder::Vec2f& texOffset, bool flipX);	
	void toggleZoomToBlack();
    void allocFBO();
};


#endif