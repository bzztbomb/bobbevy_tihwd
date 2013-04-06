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
#include "fullscreen_pass.h"
#include "LiveAssetManager.h"

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

const float zoomHackZ = -49.0166;

TreeLayer::TreeLayer() :
  SceneLayer("TreeLayer"),
  mTreeCam(WIDTH, HEIGHT, 60.0f, 1.0f, 1000.0f ),
  mGroundColor(44.0f / 255.0f, 32.0f / 255.0f, 30.0f / 255.0f),
  mSunColor(255.0f / 255.0f, 225.0f / 255.0f, 225.0f / 255.0f),
  mUpdateTrees(false),
  mTreesEnabled(true),
  mNumTrees(50),
  mTreeRadius(1.0f),
  mTreeSizeVariance(2.0f),
  mZoomToBlack(0.0f),
  mFogDistance(40.0f),
  mZoomOffset(0.45f),
  mOldResetZ(false)
{
  mFogColor = Color(30.0f / 255.0f, 10.0f / 255.0f, 10.0f / 255.0f);
	resetParams();
}

void TreeLayer::setup(SceneState* manager)
{
	mManager = manager;
  manager->mParams.addSeparator();
	manager->mParams.addParam("UpdateTrees", &mUpdateTrees);
	manager->mParams.addParam("Num Trees", &mNumTrees, "min=0.0 max=1000.0 step=5");
	manager->mParams.addParam("TreeRadius", &mTreeRadius, "min=0.0 max=100 step=0.1");
	manager->mParams.addParam("TreeSizeVariance", &mTreeSizeVariance, "min=0.0f max=4.0f step=0.1");
	manager->mParams.addParam("GroundColor", &mGroundColor);
	manager->mParams.addParam("SunColor", &mSunColor);
  manager->mParams.addParam("FogColor", &mFogColor.value());
  manager->mParams.addParam("ZoomOffset", &mZoomOffset, "step=0.1");
  
	mTreeCam.lookAt(Vec3f(0,0,0), mTreeCam.getViewDirection(), -mTreeCam.getWorldUp());
	
	// Tree engine
	gl::Texture::Format hiQFormat;
	hiQFormat.enableMipmapping();
	hiQFormat.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
	hiQFormat.setMagFilter(GL_LINEAR_MIPMAP_LINEAR);
  
	mWithLeaves = true;
	texTree = gl::Texture(loadImage(loadResource ("trees.png")), hiQFormat);
	texTreeWithLeaves = gl::Texture(loadImage(loadResource ("trees-with-leaves-aligned-scaled.png")), hiQFormat);
	texTreeBlurred = gl::Texture(loadImage(loadResource ("trees-blurred.png")), hiQFormat);
	texTreeWithLeavesBlurred = gl::Texture(loadImage(loadResource ("trees-with-leaves-aligned-blurred.png")), hiQFormat);

  texClip = gl::Texture(loadImage(loadResource("blackout.png")), hiQFormat);
	texSun = gl::Texture(loadImage(loadResource("sun.png")), hiQFormat);
  texGround = gl::Texture(loadImage(loadResource("groundTexture.png")), hiQFormat);
	texOverlay = gl::Texture(loadImage(loadResource("overlay.png")), hiQFormat);
	texBlack = gl::Texture(loadImage(loadResource("zoomToBlack.png")), hiQFormat);
  
  mTreeShader = gl::GlslProg(loadResource("TreeVert.glsl"), loadResource("TreeFrag.glsl"));
  LiveAssetManager::load("TreeVert.glsl", "TreeFrag.glsl",
                         [this](ci::DataSourceRef vert,ci::DataSourceRef frag)
                         {
                           try
                           {
                             mTreeShader = gl::GlslProg(vert, frag);
                           }
                           catch (...)
                           {
                             
                           }
                         });
  
  mTreeShadow = gl::GlslProg(loadResource("TreeVert.glsl"), loadResource("TreeBlack.glsl"));
  
  mRayShader = gl::GlslProg(loadResource("PassThruVert.glsl"), loadResource("SunRayFrag.glsl"));
  LiveAssetManager::load("PassThruVert.glsl", "SunRayFrag.glsl",
                         [this](ci::DataSourceRef vert,ci::DataSourceRef frag)
                         {
                           try
                           {
                             mRayShader = gl::GlslProg(vert, frag);
                           }
                           catch (...)
                           {
                             
                           }
                         });


  int width = getWindowWidth();
  int height = getWindowHeight();
  gl::Fbo::Format format;
  mLightRays = gl::Fbo(width, height, format);
  
	initGroundMesh();
	initTreeMesh();
}

void TreeLayer::setEnabled(bool e)
{
	SceneLayer::setEnabled(e);
	resetParams();
}

void TreeLayer::tick()
{
	if (!mEnabled)
		return;

  renderLightRays();
  
  if (mZoomToBlack == 0.0f)
	{
    mTreePan += mTreePanSpeed;
    
    while (mTreePan.z < -travelBounds.z)
      mTreePan.z += travelBounds.z;
    while (mTreePan.x < -travelBounds.x)
      mTreePan.x += travelBounds.x;
    while (mTreePan.x > travelBounds.x)
      mTreePan.x -= travelBounds.x;
  }
  
	if (mZoomToBlack > 0.0f)
	{
    mTreePan.z = mZoomToBlack * zoomHackZ;
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
  
  drawScene(getWindowBounds(), mTreeShader);
  
  gl::enableAdditiveBlending();
  mRayShader.bind();
  mRayShader.uniform("tex0", 0);
  mRayShader.uniform("tex1", 1);
  mRayShader.uniform("tex2", 2);
  mRayShader.uniform("ssLightPos", Vec2f(0.5f, 0.5f));
  shader::fullscreenQuadPass({{0, mLightRays}});
  mRayShader.unbind();
  gl::disableAlphaBlending();
}

void TreeLayer::drawScene(const cinder::Area& renderArea, cinder::gl::GlslProg& shader)
{
  
  // Set up draw states
  gl::disableDepthWrite();
  gl::disableDepthRead();
  gl::color( cinder::ColorA(1, 1, 1, 1) );
  gl::setMatricesWindowPersp(renderArea.x2, renderArea.y2);
  
  // Draw sun
  
  // Set camera up
  mTreeCam.lookAt(mTreePan, mTreePan + mTreeCam.getViewDirection(), mTreeCam.getWorldUp());
  gl::setMatrices(mTreeCam);
  
  Vec3f tl, tr, bl, br;
  mTreeCam.getFarClipCoordinates(&tl, &tr, &bl, &br);
  
  
  Vec3f bbRight, bbUp;
  mTreeCam.getBillboardVectors(&bbRight, &bbUp);
  
  shader.bind();
  shader.uniform("farClip", mFogDistance);
  shader.uniform("fogColor", ColorA(mFogColor));
  shader.uniform("tex", 0);
  shader.uniform("texBlurred", 1);
  
  texClip.bind();
  gl::drawBillboard(mTreePan + Vec3f(0.0f, 0.0f, -(mTreeCam.getFarClip()-1.0f)), Vec2f( mTreeCam.getFarClip()*10, mTreeCam.getFarClip()*10), 0, bbRight, bbUp);
  texClip.unbind();
  
  // Draw sun
  shader.unbind();
  gl::enableAlphaBlending();
  texSun.enableAndBind();
  gl::color(mSunColor);
  gl::drawBillboard(mTreePan + Vec3f(0.0f, 0.0f, -(mTreeCam.getFarClip()-1.0f)), Vec2f( mTreeCam.getFarClip(), mTreeCam.getFarClip()), 0, bbRight, bbUp);
  gl::disableAlphaBlending();
  texSun.unbind();
  shader.bind();
  
  gl::enableDepthWrite();
  gl::enableDepthRead();
  
  // Draw ground
  gl::color(mGroundColor);
  texGround.enableAndBind();
  gl::draw(mGroundMesh);
  texGround.unbind();
  
  gl::enableAlphaTest(0.0f);
  gl::enableAlphaBlending();
  
  gl::color(Color::white());
  // Enable our tree texture
  if (!mWithLeaves)
  {
    texTree.enableAndBind();
    texTreeBlurred.bind(1);
  } else {
    texTreeWithLeaves.enableAndBind();
    texTreeWithLeavesBlurred.bind(1);
  }
  
  glTranslatef(0, 0, -travelBounds.z);
  gl::draw(mTreeMesh);
  glTranslatef(0, 0, travelBounds.z);
  gl::draw(mTreeMesh);
  
  if (!mWithLeaves)
  {
    texTree.unbind();
    texTreeBlurred.unbind();
  } else {
    texTreeWithLeaves.unbind();
    texTreeWithLeavesBlurred.unbind();
  }
  
  if (mZoomToBlack > 0.0f)
  {
    texBlack.enableAndBind();
    Vec3f bbRight, bbUp;
    mTreeCam.getBillboardVectors(&bbRight, &bbUp);
    Vec2f scale = Vec2f(1070.0f / 2311.0f, 1.0f);
    scale *= 2.0f;
    Vec3f middlePos(mTreePan.x,
                    -scale.y * 0.5f,
                    zoomHackZ - 2.0f);
    gl::drawBillboard(middlePos + Vec3f(mZoomOffset, 0.0f, 0.0f), scale, 0.0f, bbRight, bbUp);
    gl::drawBillboard(middlePos - Vec3f(mZoomOffset, 0.0f, 0.0f), scale, 0.0f, bbRight, bbUp);
    texBlack.unbind();
  }
  
  shader.unbind();
  
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
	mGroundMesh.appendColorRgb(groundColor);
  mGroundMesh.appendTexCoord(Vec2f(0.0f, 0.0f));
  
	mGroundMesh.appendVertex( Vec3f(minv, yVal, maxv) );
	mGroundMesh.appendColorRgb(groundColor);
  mGroundMesh.appendTexCoord(Vec2f(0.0f, 0.0f));
  
	mGroundMesh.appendVertex( Vec3f(maxv, yVal, maxv) );
	mGroundMesh.appendColorRgb(groundColor);
  mGroundMesh.appendTexCoord(Vec2f(0.0f, 0.0f));
  
	mGroundMesh.appendVertex( Vec3f(maxv, yVal, minv));
	mGroundMesh.appendColorRgb(groundColor);
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
	travelBounds = Vec3f(mNumTrees*mTreeRadius, 0.0f, bbsize.z + mTreeRadius*2.0f);
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
		mTreeMesh.appendColorRgb(treeColor);
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
  mWithLeaves = false;
  mFogDistance = 40.0f;
}

void TreeLayer::setLeaves(bool l)
{
  mWithLeaves = l;
  if (mWithLeaves)
    mTreePanSpeed = Vec3f(0,0,0);
}

void TreeLayer::init()
{
  SceneLayer::init();
  
  registerParam("leaves");
  registerParam("panSpeed.x", &mTreePanSpeedTimeline.x, -1.0f, 1.0f);
  registerParam("panSpeed.y", &mTreePanSpeedTimeline.y, -1.0f, 1.0f);
  registerParam("panSpeed.z", &mTreePanSpeedTimeline.z, -10.0f, 1.0f);
  registerParam("fogDistance", &mFogDistance.value(), 0, 40);
  registerParam("resetZ");
  registerParam("zoomToBlack");
}

void TreeLayer::update()
{
  SceneLayer::update();
  
  bool newLeaves = getParamValue("leaves") > 0.5f;
  if (getLeaves() != newLeaves)
  {
    setLeaves(newLeaves);
  }
  bool newResetZ = getParamValue("resetZ") > 0.5f;
  if ((newResetZ != mOldResetZ) && (newResetZ))
  {
    mTreePan.z = 0;
  }
  mOldResetZ = newResetZ;
  mZoomToBlack = getParamValue("zoomToBlack");
  if (mZoomToBlack == 0.0f)
    mTreePanSpeed = mTreePanSpeedTimeline;
}

void TreeLayer::renderLightRays()
{
  gl::SaveFramebufferBinding bindingSaver;
  gl::pushMatrices();
  Area savedVP = gl::getViewport();
  
  mLightRays.bindFramebuffer();
  // clear out the window with black
  gl::clear( Color( 0, 0, 0 ) );
  
  gl::setViewport(mLightRays.getBounds());
  gl::setMatricesWindow(mLightRays.getBounds().getWidth(), mLightRays.getBounds().getHeight());
  
  drawScene(mLightRays.getBounds(), mTreeShadow);
  
  gl::popMatrices();
  gl::setViewport(savedVP);
}