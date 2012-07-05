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
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/TriMesh.h"
#include "cinder/Camera.h"


class TreeLayer : public SceneLayer
{
public:
	TreeLayer();
	virtual void setup(SceneState* manager);
	virtual void keyDown( cinder::app::KeyEvent event );
	virtual void update();
	virtual void draw();	
	virtual void setEnabled(bool e);
	
	void resetParams();
private:
	SceneState* mManager;	
	// Trees
	cinder::gl::Texture texSun;
	cinder::gl::Texture texTree;
	cinder::gl::Texture texOverlay;
	bool mTreesEnabled;
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
    bool mFboActive;
    cinder::gl::Fbo mFbo;
    cinder::gl::GlslProg mFadeShader;
    cinder::Anim<float> mFadeAmount;
	
	void drawTrees();
	void initGroundMesh();
	void initTreeMesh();
	void addTree(const cinder::Vec3f& treePos, const cinder::Vec2f& treeScale, const cinder::Vec2f& texOffset, bool flipX);	
	void toggleZoomToBlack();
};


#endif