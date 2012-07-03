/*
 *  bbTreeLayer.cpp
 *  bobbevy
 *
 *  Created by Brian Richardson on 6/30/12.
 *  Copyright 2012 Knowhere Studios Inc. All rights reserved.
 *
 */

#include "bbTreeLayer.h"
#include "cinder/ImageIo.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

//
// TreeLayer
//
// BTRTODO: FIXME
#define WIDTH 800
#define HEIGHT 600

TreeLayer::TreeLayer() :
	mTreeCam(WIDTH, HEIGHT, 60.0f, 1.0f, 1000.0f ),
	mGroundColor(44.0f / 255.0f, 32.0f / 255.0f, 30.0f / 255.0f),
	mSunColor(255.0f / 255.0f, 225.0f / 255.0f, 225.0f / 255.0f),
	mUpdateTrees(false),
	mTreesEnabled(true),
	mNumTrees(50),
	mTreeRadius(1.0f),
	mTreeSizeVariance(2.0f),
	mZoomToBlack(false)
{
	resetParams();
}

void TreeLayer::setup(SceneState* manager)
{
	mManager = manager;
	manager->mParams.addParam("UpdateTrees", &mUpdateTrees);
	manager->mParams.addParam("Num Trees", &mNumTrees, "min=0.0 max=1000.0 step=5");
	manager->mParams.addParam("TreePanSpeedX", &mTreePanSpeed.value().x, "min=-10.0 max=10.0 step=0.0005 keyIncr=x keyDecr=X");
	manager->mParams.addParam("TreePanSpeedY", &mTreePanSpeed.value().y, "min=-10.0 max=10.0 step=0.0002 keyIncr=y keyDecr=Y");
	manager->mParams.addParam("TreePanSpeedZ", &mTreePanSpeed.value().z, "min=-10.0 max=10.0 step=0.0005 keyIncr=z keyDecr=Z");
	manager->mParams.addParam("TreeRadius", &mTreeRadius, "min=0.0 max=100 step=0.1");
	manager->mParams.addParam("TreeSizeVariance", &mTreeSizeVariance, "min=0.0f max=4.0f step=0.1 keyIncr=v keyDecr=V");
	manager->mParams.addParam("GroundColor", &mGroundColor);
	manager->mParams.addParam("SunColor", &mSunColor);
	manager->mParams.addParam("ZoomTarget", &mZoomTarget);
	
	mTreeCam.lookAt(Vec3f(0,0,0), mTreeCam.getViewDirection(), -mTreeCam.getWorldUp());
	
	// Tree engine
	gl::Texture::Format hiQFormat;
	hiQFormat.enableMipmapping();
	hiQFormat.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
	hiQFormat.setMagFilter(GL_LINEAR_MIPMAP_LINEAR);
	
	texTree = gl::Texture(loadImage(loadAsset ("trees.png")), hiQFormat);
	texSun = gl::Texture(loadImage(loadAsset("sun.png")), hiQFormat);
	texOverlay = gl::Texture(loadImage(loadAsset("overlay.png")), hiQFormat);
	texBlack = gl::Texture(loadImage(loadAsset("zoomToBlack.png")), hiQFormat);
	
	initGroundMesh();
	initTreeMesh();	
}

void TreeLayer::setEnabled(bool e)
{
	SceneLayer::setEnabled(e);
	resetParams();
}

void TreeLayer::keyDown( cinder::app::KeyEvent event )
{
	if (!mEnabled)
		return;
	
	float fastTween = 1.0f;
	float slowTween = 10.0f;
	
	switch (event.getCode())
	{
		case KeyEvent::KEY_1:
			mManager->mTimeline->apply(&mTreePanSpeed, Vec3f(-0.001f, 0.0f, 0.0f), slowTween);
			break;
		case KeyEvent::KEY_2:
			mManager->mTimeline->apply(&mTreePanSpeed, Vec3f(-0.05f, 0.0f, 0.0f), fastTween);
			break;
		case KeyEvent::KEY_3:
			mManager->mTimeline->apply(&mTreePanSpeed, Vec3f(-0.05f, 0.0f, -0.03f), fastTween);
			break;
		case KeyEvent::KEY_4:
			mManager->mTimeline->apply(&mTreePanSpeed, Vec3f(-0.05f, -0.04f, -0.03f), fastTween);
			mManager->mTimeline->apply(&mManager->mBlackoutAmount, 1.0f, 5.0f);
			break;
		case KeyEvent::KEY_5:
			mManager->mTimeline->apply(&mTreePanSpeed, Vec3f(-0.05f, -0.002f, -0.03f), fastTween);
			break;
		case KeyEvent::KEY_9:
			mTreePanSpeed = Vec3f(0,0,0);
			break;
		case KeyEvent::KEY_0:
			resetParams();
			break;
		case KeyEvent::KEY_8:
			toggleZoomToBlack();
			break;
	}
}

void TreeLayer::toggleZoomToBlack()
{
	mZoomToBlack = !mZoomToBlack;
	if (mZoomToBlack)
	{
		mZoomTarget = Vec3f(0, 0, -10.0f);
		mTreePanSpeed = mZoomTarget / (9.0f*30.0f);
	}
}

void TreeLayer::update()
{
	if (!mEnabled)
		return;
	mTreePan += mTreePanSpeed;
	
	while (mTreePan.z < -travelBounds.z)
		mTreePan.z += travelBounds.z;
	while (mTreePan.x < -travelBounds.x)
		mTreePan.x += travelBounds.x;
	while (mTreePan.x > travelBounds.x)
		mTreePan.x -= travelBounds.x;
	
	if (mZoomToBlack)
	{
		if (mZoomTarget.z == -1.1f)
		{
//			setEnabled(false);
		}
		mZoomTarget -= mTreePanSpeed;
		if (mZoomTarget.z > -1.1f)
			mZoomTarget.z = -1.1f;			
	}
	
	if (mUpdateTrees)
	{
		mUpdateTrees = false;
		initTreeMesh();
		initGroundMesh();
	}	
}

void TreeLayer::draw()
{
	if (!mEnabled)
		return;
	
	// Set up draw states
	gl::disableDepthWrite();
	gl::disableDepthRead();
	gl::color( cinder::ColorA(1, 1, 1, 1) );
	
	// Draw sun
	gl::color(mSunColor);
	gl::setMatricesWindowPersp( getWindowWidth(), getWindowHeight());
	gl::draw(texSun, getWindowBounds());
	gl::color( cinder::ColorA(1, 1, 1, 1) );
	
	// Set camera up
	mTreeCam.lookAt(mTreePan, mTreePan + mTreeCam.getViewDirection(), mTreeCam.getWorldUp());
	gl::setMatrices(mTreeCam);
	
	// Draw ground
	gl::enableDepthWrite();
	gl::enableDepthRead();
	
	glDisable(GL_TEXTURE);
	glDisable(GL_TEXTURE_2D);
	gl::draw(mGroundMesh);
	glEnable(GL_TEXTURE);
	glEnable(GL_TEXTURE_2D);
	
	gl::disableDepthWrite();
	gl::enableDepthRead();
	gl::enableAlphaBlending();
	
	// Enable our tree texture
	texTree.enableAndBind();
	glTranslatef(0, 0, -travelBounds.z);
	gl::draw(mTreeMesh);
	glTranslatef(0, 0, travelBounds.z);
	gl::draw(mTreeMesh);
	texTree.unbind();
	
	if (mZoomToBlack)
	{
		texBlack.enableAndBind();
		Vec3f bbRight, bbUp;
		mTreeCam.getBillboardVectors(&bbRight, &bbUp);
		gl::drawBillboard(mTreePan + mZoomTarget, Vec2f(20, 20), 0, bbRight, bbUp); 
		texBlack.unbind();
	}
	
	// Draw overlay
	gl::disableDepthRead();
	gl::setMatricesWindowPersp( getWindowWidth(), getWindowHeight());
	gl::draw(texOverlay, getWindowBounds());	
}

void TreeLayer::initGroundMesh()
{
	mGroundMesh.clear();
	// Ground mesh
	float minv = -10000.0f;
	float maxv = 10000.0f;
	float yVal = 0;
	mGroundMesh.appendVertex(Vec3f(minv, yVal, minv ));
	Color groundColor(mGroundColor);
	mGroundMesh.appendColorRGB(groundColor);
	mGroundMesh.appendVertex( Vec3f(minv, yVal, maxv) );
	mGroundMesh.appendColorRGB(groundColor);
	mGroundMesh.appendVertex( Vec3f(maxv, yVal, maxv) );
	mGroundMesh.appendColorRGB(groundColor);
	mGroundMesh.appendVertex( Vec3f(maxv, yVal, minv));
	mGroundMesh.appendColorRGB(groundColor);
	
	// get the index of the vertex. not necessary with this example, but good practice
	int vIdx0 = mGroundMesh.getNumVertices() - 4;
	int vIdx1 = mGroundMesh.getNumVertices() - 3;
	int vIdx2 = mGroundMesh.getNumVertices() - 2;
	int vIdx3 = mGroundMesh.getNumVertices() - 1;
	
	// now create the triangles from the vertices
	mGroundMesh.appendTriangle( vIdx0, vIdx1, vIdx2 );
	mGroundMesh.appendTriangle( vIdx0, vIdx2, vIdx3 );
}

void TreeLayer::initTreeMesh()
{
	float scaleFactors[3] = { 1070.0f / 2311.0f, 954.0f / 2307.0f, 845.0f / 2132.0f };
	mTreeMesh.clear();
	
	Rand randit(23);
	for (int j = mNumTrees / 2; j >= 0; j--)
	{
		Vec3f rowStart = -Vec3f(mNumTrees*mTreeRadius / 2.0f, 0.0f, mTreeRadius * j * 2);
		Vec3f rowCurrent = rowStart;
		gl::color( cinder::ColorA(1, 1, 1, 1) );
		for (int i = 0; i < mNumTrees; i++)
		{
			int whichTree = randit.nextInt(3);
			Vec2f r = randit.nextVec2f() * (mTreeRadius / 4.0f);
			Vec3f treePos = rowCurrent + Vec3f(r.x, 0.0f, r.y);
			Vec2f scale = Vec2f(scaleFactors[whichTree], 1.0f);
			Vec2f treeScale = scale * (1 + randit.nextFloat(mTreeSizeVariance));
			treePos.y -= treeScale.y * 0.5;
			float minBound = -mNumTrees*mTreeRadius / 2.0f;
			if (treePos.x - (treeScale.x*scale.x) < minBound)
			{
				treePos.x = minBound + treeScale.x*scale.x;
			}
			float maxBound = mNumTrees*mTreeRadius / 2.0f;
			if (treePos.x + (treeScale.x*scale.x) > maxBound)
			{
				treePos.x = maxBound - treeScale.x*scale.x;
			}
			treePos.z = max(treePos.z, -mNumTrees*mTreeRadius);
			treePos.z = min(treePos.z, 0.0f);
			Vec2f texOffset(whichTree / 3.0f, 0.0f);
			bool flip = randit.nextBool();
			addTree(treePos, treeScale, texOffset, flip);
			addTree(treePos - Vec3f(mNumTrees*mTreeRadius, 0.0f, 0.0f), treeScale, texOffset, flip);
			addTree(treePos + Vec3f(mNumTrees*mTreeRadius, 0.0f, 0.0f), treeScale, texOffset, flip);
			
			rowCurrent += Vec3f(mTreeRadius, 0.0f, 0.0f);
		}
	}
	Vec3f bbsize = mTreeMesh.calcBoundingBox().getSize();
	travelBounds = Vec3f(mNumTrees*mTreeRadius, 0.0f, bbsize.z);
}

void TreeLayer::addTree(const Vec3f& treePos, const Vec2f& treeScale, const Vec2f& texOffset, bool flipX)
{
	GLfloat texCoords[8] = { 0, 1, 0, 0, 1.0f / 3.0f, 0, 1.0f / 3.0f, 1 };
	GLfloat texCoordsFlipped[8] = { 1.0f / 3.0f, 1, 1.0f / 3.0f, 0, 0, 0, 0, 1 };
	
	Vec3f bbRight(-1, 0, 0);
	Vec3f bbUp(0,0,-1);
	mTreeCam.getBillboardVectors(&bbRight, &bbUp);
	
	Vec3f verts[4];
	verts[0] = treePos + bbRight * (-0.5f * treeScale.x) + bbUp * (-0.5f  * treeScale.y);
	verts[1] = treePos + bbRight * (-0.5f * treeScale.x) + bbUp * (0.5f  * treeScale.y);
	verts[2] = treePos + bbRight * (0.5f * treeScale.x) + bbUp * (0.5f  * treeScale.y);
	verts[3] = treePos + bbRight * (0.5f * treeScale.x) + bbUp * (-0.5f  * treeScale.y);
	
	Color treeColor(1, 1, 1);
	for (int i = 0; i < 4; i++)
	{
		mTreeMesh.appendVertex(verts[i]);
		mTreeMesh.appendColorRGB(treeColor);
		Vec2f tc = Vec2f(flipX ? texCoordsFlipped[i*2] : texCoords[i*2], flipX ? texCoordsFlipped[i*2+1] : texCoords[i*2+1]) + texOffset;
		mTreeMesh.appendTexCoord(tc);
	}
	
	// get the index of the vertex. not necessary with this example, but good practice
	int vIdx0 = mTreeMesh.getNumVertices() - 4;
	int vIdx1 = mTreeMesh.getNumVertices() - 3;
	int vIdx2 = mTreeMesh.getNumVertices() - 2;
	int vIdx3 = mTreeMesh.getNumVertices() - 1;
	
	// now create the triangles from the vertices
	mTreeMesh.appendTriangle( vIdx0, vIdx1, vIdx2 );
	mTreeMesh.appendTriangle( vIdx0, vIdx2, vIdx3 );
}

void TreeLayer::resetParams()
{
	mTreePan = Vec3f(0.0f, -1.1f, 0.0f);
	mTreePanSpeed = Vec3f(-0.001f, 0.0f, 0.0f);
	mZoomToBlack = false;
}