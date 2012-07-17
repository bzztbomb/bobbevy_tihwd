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
using namespace gl;

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
	mZoomToBlack(false),
    mFadeAmount(1.0f),
    mWarpAmount(0.0f),
    mAlphaAmount(1.0f),
    mTime(0.0f),
    mTimeMult(1.0f),
    mYMult(1.0f),
    mFadeTransTime(20.0f),
    mFogDistance(40.0f),
    mFogHeight(200.0f)
{
	resetParams();
}

void TreeLayer::setup(SceneState* manager)
{
	mManager = manager;
    manager->mParams.addSeparator();
	manager->mParams.addParam("UpdateTrees", &mUpdateTrees);
	manager->mParams.addParam("Num Trees", &mNumTrees, "min=0.0 max=1000.0 step=5");
	manager->mParams.addParam("TreePanSpeedX", &mTreePanSpeed.value().x, "min=-10.0 max=10.0 step=0.0005 keyIncr=x keyDecr=X");
	manager->mParams.addParam("TreePanSpeedY", &mTreePanSpeed.value().y, "min=-10.0 max=10.0 step=0.0002 keyIncr=y keyDecr=Y");
	manager->mParams.addParam("TreePanSpeedZ", &mTreePanSpeed.value().z, "min=-10.0 max=10.0 step=0.0005 keyIncr=z keyDecr=Z");
	manager->mParams.addParam("TreeRadius", &mTreeRadius, "min=0.0 max=100 step=0.1");
	manager->mParams.addParam("TreeSizeVariance", &mTreeSizeVariance, "min=0.0f max=4.0f step=0.1");
	manager->mParams.addParam("GroundColor", &mGroundColor);
	manager->mParams.addParam("SunColor", &mSunColor);
	manager->mParams.addParam("ZoomTarget", &mZoomTarget);
    manager->mParams.addParam("FadeAmount", &mFadeAmount.value(), "min=0.0 max=1.0 step=0.01");
    manager->mParams.addParam("WarpAmount", &mWarpAmount.value(), "min=0.0 max=100.0 step=0.001");
    manager->mParams.addParam("AlphaAmount", &mAlphaAmount.value(), "min=0.0 max=1.0 step=0.01");
    manager->mParams.addParam("TimeMult", &mTimeMult, "min=0.0 max=100.0 step=0.01");
    manager->mParams.addParam("yMult", &mYMult, "min=0.0 max=10.0 step=0.01");
    manager->mParams.addParam("FadeTransTime", &mFadeTransTime, "min=0");
    manager->mParams.addParam("FogColor", &mFogColor.value());
    manager->mParams.addParam("FogDistance", &mFogDistance.value(), "step=1.0");
    manager->mParams.addParam("FogHeight", &mFogHeight.value());
    
    
	mTreeCam.lookAt(Vec3f(0,0,0), mTreeCam.getViewDirection(), -mTreeCam.getWorldUp());
	
	// Tree engine
	gl::Texture::Format hiQFormat;
	hiQFormat.enableMipmapping();
	hiQFormat.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
	hiQFormat.setMagFilter(GL_LINEAR_MIPMAP_LINEAR);

	mWithLeaves = true;
	texTree = gl::Texture(loadImage(loadResource ("trees.png")), hiQFormat);
	texTreeWithLeaves = gl::Texture(loadImage(loadResource ("trees-with-leaves-aligned-scaled.png")), hiQFormat);
    texClip = gl::Texture(loadImage(loadResource("blackout.png")), hiQFormat);
	texSun = gl::Texture(loadImage(loadResource("sun.png")), hiQFormat);
    texGround = gl::Texture(loadImage(loadResource("groundTexture.png")), hiQFormat);                            
	texOverlay = gl::Texture(loadImage(loadResource("overlay.png")), hiQFormat);
	texBlack = gl::Texture(loadImage(loadResource("zoomToBlack.png")), hiQFormat);
    
    mTreeShader = gl::GlslProg(loadResource("TreeVert.glsl"), loadResource("TreeFrag.glsl"));
    
	initGroundMesh();
	initTreeMesh();	
    
    mTime = getElapsedSeconds();    
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
        case KeyEvent::KEY_6:
			mManager->mTimeline->apply(&mTreePanSpeed, Vec3f(0.0f, 0.0f, -9.5f), fastTween);
            break;
        case KeyEvent::KEY_7:
            setBlurred();
            break;
//        case KeyEvent::KEY_8:
//            mManager->mTimeline->apply(&mFadeAmount, 1.0f, mFadeTransTime);
//            mManager->mTimeline->apply(&mWarpAmount, 0.0f, mFadeTransTime);
//            mManager->mTimeline->apply(&mAlphaAmount, 1.0f, mFadeTransTime);
//            break;
		case KeyEvent::KEY_9:
			mTreePanSpeed = Vec3f(0,0,0);
			break;
		case KeyEvent::KEY_0:
			resetParams();
			break;
        case KeyEvent::KEY_a:
            mWithLeaves = !mWithLeaves;
            break;
		case KeyEvent::KEY_8:
			toggleZoomToBlack();
			break;
        case KeyEvent::KEY_w:
            mFogColor = Color(30.0 / 255.0, 10.0 / 255.0, 10.0 / 255.0);
            mFogDistance = 1;
            break;
        case KeyEvent::KEY_r:                        
            mManager->mTimeline->apply(&mFogDistance, 12.0f, slowTween);            
            break;
        case KeyEvent::KEY_u:
            mManager->mTimeline->apply(&mFogDistance, 40.0f, slowTween);                        
            break;
	}
}

void TreeLayer::toggleZoomToBlack()
{
	mZoomToBlack = !mZoomToBlack;
	if (mZoomToBlack)
	{
		mZoomTarget = Vec3f(0.0f, 0.0f, -10.0f);
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
        {
			mZoomTarget.z = -1.1f;			
            mTreePanSpeed = Vec3f(0.0f, 0.0f, 0.0f);
        }
	}
	
	if (mUpdateTrees)
	{
		mUpdateTrees = false;
		initTreeMesh();
		initGroundMesh();
	}	
    
    mTime = getElapsedSeconds();
//    float chunk = M_PI * 2.0f * 10.0f;
//    while (mTime > chunk)
//        mTime -= chunk;
}

void TreeLayer::draw()
{
	if (!mEnabled)
		return;
            
    // Set up draw states
    gl::disableDepthWrite();
    gl::disableDepthRead();
    gl::color( cinder::ColorA(1, 1, 1, 1) );
    Area renderArea = getWindowBounds();
    gl::setMatricesWindowPersp(renderArea.x2, renderArea.y2);
    
    // Draw sun
    
    // Set camera up
    mTreeCam.lookAt(mTreePan, mTreePan + mTreeCam.getViewDirection(), mTreeCam.getWorldUp());
    gl::setMatrices(mTreeCam);

    Vec3f tl, tr, bl, br;
    mTreeCam.getFarClipCoordinates(&tl, &tr, &bl, &br);

    
    Vec3f bbRight, bbUp;
    mTreeCam.getBillboardVectors(&bbRight, &bbUp);

    mTreeShader.bind();
    mTreeShader.uniform("farClip", mFogDistance);
    mTreeShader.uniform("fogColor", ColorA(mFogColor));
    mTreeShader.uniform("fogHeight", mFogHeight);
    
    texClip.bind();
    gl::drawBillboard(mTreePan + Vec3f(0.0f, 0.0f, -(mTreeCam.getFarClip()-1.0f)), Vec2f( mTreeCam.getFarClip()*10, mTreeCam.getFarClip()*10), 0, bbRight, bbUp);                
    texClip.unbind();

    // Draw sun
    mTreeShader.unbind();
    gl::enableAlphaBlending();
    texSun.enableAndBind();        
    gl::color(mSunColor);
    gl::drawBillboard(mTreePan + Vec3f(0.0f, 0.0f, -(mTreeCam.getFarClip()-1.0f)), Vec2f( mTreeCam.getFarClip(), mTreeCam.getFarClip()), 0, bbRight, bbUp);        
    gl::disableAlphaBlending();
    texSun.unbind();
    mTreeShader.bind();
    
    gl::enableDepthWrite();
    gl::enableDepthRead();

    // Draw ground
    gl::color(mGroundColor);
    texGround.enableAndBind();
    gl::draw(mGroundMesh);
    texGround.unbind();
    
    gl::enableAlphaTest(0.0f);
    gl::enableAlphaBlending();
    
    // Enable our tree texture
    if (!mWithLeaves)
        texTree.enableAndBind();
    else
        texTreeWithLeaves.enableAndBind();
    
    {
        glTranslatef(0, 0, -travelBounds.z);
        gl::draw(mTreeMesh);
        glTranslatef(0, 0, travelBounds.z);
        gl::draw(mTreeMesh);
    }
    if (!mWithLeaves)
        texTree.unbind();
    else
        texTreeWithLeaves.unbind();
    
    if (mZoomToBlack)
    {
        texBlack.enableAndBind();
        Vec3f bbRight, bbUp;
        mTreeCam.getBillboardVectors(&bbRight, &bbUp);
        gl::drawBillboard(mTreePan + mZoomTarget, Vec2f(2, 2), 0, bbRight, bbUp); 
        texBlack.unbind();
    }
    mTreeShader.unbind();
    
    gl::disableAlphaTest();
    gl::enableAlphaBlending();
    // Draw overlay
    gl::disableDepthRead();
    gl::disableDepthWrite();
    gl::setMatricesWindowPersp(renderArea.x2, renderArea.y2);
    gl::draw(texOverlay, renderArea);	
    gl::disableAlphaBlending();
}

void TreeLayer::initGroundMesh()
{
	mGroundMesh.clear();
	// Ground mesh
	float minv = -10000.0f;
	float maxv = 10000.0f;
	float yVal = 0;
	Color groundColor(mGroundColor);
    
	mGroundMesh.appendVertex(Vec3f(minv, yVal, minv ));
	mGroundMesh.appendColorRGB(groundColor);
    mGroundMesh.appendTexCoord(Vec2f(0.0f, 0.0f));
    
	mGroundMesh.appendVertex( Vec3f(minv, yVal, maxv) );    
	mGroundMesh.appendColorRGB(groundColor);
    mGroundMesh.appendTexCoord(Vec2f(0.0f, 0.0f));
    
	mGroundMesh.appendVertex( Vec3f(maxv, yVal, maxv) );    
	mGroundMesh.appendColorRGB(groundColor);
    mGroundMesh.appendTexCoord(Vec2f(0.0f, 0.0f));
    
	mGroundMesh.appendVertex( Vec3f(maxv, yVal, minv));
	mGroundMesh.appendColorRGB(groundColor);
    mGroundMesh.appendTexCoord(Vec2f(0.0f, 0.0f));
	
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
			Vec2f treeScale = scale * (1.0 + randit.nextFloat(mTreeSizeVariance));
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
	travelBounds = Vec3f(mNumTrees*mTreeRadius, 0.0f, bbsize.z + mTreeRadius);
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
    mFadeAmount = 1.0f;
    mWarpAmount = 0.0f;        
    mAlphaAmount = 0.15f;
    mWithLeaves = false;
}

void TreeLayer::setBlurred()
{
    mFadeAmount = 0.27f;
    mWarpAmount = 0.006f;  
    mAlphaAmount = 0.77f;
}

void TreeLayer::setLeaves(bool l)
{
    mWithLeaves = l;
    if (mWithLeaves)
        mTreePanSpeed = Vec3f(0,0,0);        
}