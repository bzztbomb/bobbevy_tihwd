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

std::set<SkeletonParticles*> SkeletonParticles::smCurrentSwarms;

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
SkeletonParticles::SkeletonParticles(const std::string& moduleName) :
  SceneLayer(moduleName),
  mManager(NULL),
  mTargetPoint(300.0f, 300.0f, 0.0f),
  mMaxVel(1.6f),
  mAfterTargetAccel(2.0f),
  mSwarm(true),
  mUserToken(KinectWrapper::utClosest),
  mColor(1,1,1),
  mDrag(0.9f),
  mTargetDrag(1.0f),
  mDistanceThresh(3.0f),
  mDropping(false),
  mDropAccel(0.0f, 1.0f, 0.0f),
  mNumParticles(200),
  mNumPositions(100),
  mBounds(100,100,600,600),
  mZValue(0.0f),
  mParticleSize(16.0f, 16.0f),
  mMoveSwarm(true),
  mCoordSpace(800.0f, 600.0f)
{
  smCurrentSwarms.insert(this);
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

SkeletonParticles::~SkeletonParticles()
{
  smCurrentSwarms.erase(this);
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
    case KeyEvent::KEY_j:
      moveSwarm(true);
      break;
  }
}

void SkeletonParticles::resetParticle(int index)
{
	Vec3f zero(0.0f, 0.0f, 0.0f);
	float radius = 200.0f;
	mParticlePos[index] = randVec3f() * radius;
	if (mSwarm)
		mParticlePos[index] += Vec3f(mCoordSpace.x + radius*2, mCoordSpace.y + radius*2, 0.0f);
	else
	{
		mParticlePos[index].x = mTargetPoint.x;
		mParticlePos[index].y = mCoordSpace.y-1;
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
    particleIn |= (mParticlePos[i].y < mCoordSpace.y);
	}
  if (!particleIn)
    setEnabled(false);
}

void SkeletonParticles::tick()
{
	if (!mEnabled)
		return;
  
  if (!mMoveSwarm)
  {
    updateWaiting();
    return;
  }
	
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
  //    gl::enableDepthRead();
  
  Vec3f bbUp(0.0f, -1.0f, 0.0f);
  Vec3f bbRight(-1.0f, 0.0f, 0.0f);
  
  Vec2f scaleFactor2f = ((Vec2f)getWindowSize()) / mCoordSpace;
  Vec3f scaleFactor3f(scaleFactor2f.x, scaleFactor2f.y, 1.0);
  
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
      velNorm.safeNormalize();
      velNorm *= -1.0f;
      Vec2f polar = toPolar(velNorm);
      Vec3f drawPos(mParticlePos[i].x, mParticlePos[i].y, mZValue);
      drawPos *= scaleFactor3f;
      gl::drawBillboard(drawPos,
                        mParticleSize, toDegrees(polar.y), bbRight, bbUp);
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
    mMoveSwarm = true;
  }
}

void SkeletonParticles::setZValue(float z)
{
  mZValue = z;
}

float initial_positions[NUM_SKELETON_PARTICLES][3] = {
  { -0.123214285714, -0.0285714285714, 0.0f },
  { -0.119047619048, -0.0219047619048, 0.0f },
  { -0.122619047619, -0.0228571428571, 0.0f },
  { -0.119047619048, -0.0361904761905, 0.0f },
  { -0.116071428571, -0.0285714285714, 0.0f },
  { -0.12380952381, -0.0333333333333, 0.0f },
  { -0.120833333333, -0.0542857142857, 0.0f },
  { -0.125, -0.0580952380952, 0.0f },
  { -0.120833333333, -0.0628571428571, 0.0f },
  { -0.117261904762, -0.0590476190476, 0.0f },
  { -0.114285714286, -0.0514285714286, 0.0f },
  { 0.140476190476, -0.0619047619048, 0.0f },
  { 0.136904761905, -0.0695238095238, 0.0f },
  { 0.133333333333, -0.0619047619048, 0.0f },
  { 0.139285714286, -0.0809523809524, 0.0f },
  { 0.134523809524, -0.0828571428571, 0.0f },
  { 0.131547619048, -0.0771428571429, 0.0f },
  { 0.156547619048, -0.14380952381, 0.0f },
  { 0.153571428571, -0.152380952381, 0.0f },
  { 0.150595238095, -0.146666666667, 0.0f },
  { 0.147619047619, -0.160952380952, 0.0f },
  { 0.145833333333, -0.164761904762, 0.0f },
  { 0.144047619048, -0.167619047619, 0.0f },
  { 0.144642857143, -0.16, 0.0f },
  { 0.116071428571, -0.137142857143, 0.0f },
  { 0.117261904762, -0.141904761905, 0.0f },
  { 0.113095238095, -0.138095238095, 0.0f },
  { 0.106547619048, -0.146666666667, 0.0f },
  { 0.103571428571, -0.140952380952, 0.0f },
  { 0.105952380952, -0.14, 0.0f },
  { 0.085119047619, -0.142857142857, 0.0f },
  { -0.0160714285714, -0.373333333333, 0.0f },
  { -0.0244047619048, -0.373333333333, 0.0f },
  { -0.0172619047619, -0.366666666667, 0.0f },
  { -0.0172619047619, -0.371428571429, 0.0f },
  { -0.0154761904762, -0.361904761905, 0.0f },
  { -0.0208333333333, -0.362857142857, 0.0f },
  { 0.00297619047619, -0.365714285714, 0.0f },
  { 0.00119047619048, -0.357142857143, 0.0f },
  { 0.00178571428571, -0.352380952381, 0.0f },
  { 0.00892857142857, -0.36380952381, 0.0f },
  { 0.00595238095238, -0.358095238095, 0.0f },
  { 0.00654761904762, -0.352380952381, 0.0f },
  { 0.027380952381, -0.36380952381, 0.0f },
  { 0.0255952380952, -0.360952380952, 0.0f },
  { 0.025, -0.355238095238, 0.0f },
  { 0.0345238095238, -0.36380952381, 0.0f },
  { 0.0333333333333, -0.36, 0.0f },
  { 0.0315476190476, -0.353333333333, 0.0f },
  { 0.0452380952381, -0.374285714286, 0.0f },
  { 0.0464285714286, -0.36380952381, 0.0f },
  { 0.0559523809524, -0.373333333333, 0.0f },
  { 0.0571428571429, -0.371428571429, 0.0f },
  { 0.0541666666667, -0.365714285714, 0.0f },
  { 0.0577380952381, -0.365714285714, 0.0f },
  { 0.052380952381, -0.359047619048, 0.0f },
  { 0.0434523809524, -0.368571428571, 0.0f },
  { 0.0565476190476, -0.377142857143, 0.0f },
  { -0.0964285714286, -0.326666666667, 0.0f },
  { -0.097619047619, -0.324761904762, 0.0f },
  { -0.102380952381, -0.317142857143, 0.0f },
  { -0.0952380952381, -0.318095238095, 0.0f },
  { -0.108928571429, -0.312380952381, 0.0f },
  { -0.123214285714, -0.30380952381, 0.0f },
  { -0.121428571429, -0.29619047619, 0.0f },
  { -0.116071428571, -0.285714285714, 0.0f },
  { -0.119642857143, -0.298095238095, 0.0f },
  { -0.125, -0.292380952381, 0.0f },
  { -0.116666666667, -0.275238095238, 0.0f },
  { -0.116071428571, -0.268571428571, 0.0f },
  { -0.117261904762, -0.270476190476, 0.0f },
  { -0.122023809524, -0.267619047619, 0.0f },
  { -0.127976190476, -0.209523809524, 0.0f },
  { -0.127380952381, -0.197142857143, 0.0f },
  { -0.130357142857, -0.188571428571, 0.0f },
  { -0.12619047619, -0.172380952381, 0.0f },
  { -0.128571428571, -0.169523809524, 0.0f },
  { -0.129761904762, -0.178095238095, 0.0f },
  { -0.129761904762, -0.171428571429, 0.0f },
  { -0.129761904762, -0.146666666667, 0.0f },
  { -0.129761904762, -0.154285714286, 0.0f },
  { -0.125595238095, -0.148571428571, 0.0f },
  { -0.129166666667, -0.133333333333, 0.0f },
  { -0.133333333333, -0.126666666667, 0.0f },
  { -0.127380952381, -0.126666666667, 0.0f },
  { -0.121428571429, -0.128571428571, 0.0f },
  { -0.119047619048, -0.0809523809524, 0.0f },
  { -0.120238095238, -0.0771428571429, 0.0f },
  { -0.111904761905, -0.0752380952381, 0.0f },
  { -0.116666666667, -0.0838095238095, 0.0f },
  { -0.114285714286, -0.0761904761905, 0.0f },
  { -0.0738095238095, -0.187619047619, 0.0f },
  { -0.0767857142857, -0.184761904762, 0.0f },
  { 0.0690476190476, -0.350476190476, 0.0f },
  { 0.0678571428571, -0.348571428571, 0.0f },
  { 0.0744047619048, -0.347619047619, 0.0f },
  { 0.0982142857143, -0.32380952381, 0.0f },
  { 0.0952380952381, -0.32, 0.0f },
  { 0.0982142857143, -0.319047619048, 0.0f },
  { 0.0940476190476, -0.314285714286, 0.0f },
  { 0.0797619047619, -0.338095238095, 0.0f },
  { 0.0833333333333, -0.329523809524, 0.0f },
  { 0.0779761904762, -0.33619047619, 0.0f },
  { 0.0785714285714, -0.330476190476, 0.0f },
  { 0.0815476190476, -0.339047619048, 0.0f },
  { 0.0946428571429, -0.304761904762, 0.0f },
  { 0.1, -0.3, 0.0f },
  { 0.108928571429, -0.3, 0.0f },
  { 0.104166666667, -0.299047619048, 0.0f },
  { 0.1, -0.295238095238, 0.0f },
  { 0.097619047619, -0.295238095238, 0.0f },
  { 0.102380952381, -0.29619047619, 0.0f },
  { 0.122619047619, -0.288571428571, 0.0f },
  { 0.12380952381, -0.28380952381, 0.0f },
  { 0.126785714286, -0.282857142857, 0.0f },
  { 0.123214285714, -0.278095238095, 0.0f },
  { 0.135119047619, -0.295238095238, 0.0f },
  { 0.1375, -0.287619047619, 0.0f },
  { 0.1375, -0.282857142857, 0.0f },
  { 0.135119047619, -0.173333333333, 0.0f },
  { 0.13630952381, -0.160952380952, 0.0f },
  { 0.139880952381, -0.154285714286, 0.0f },
  { 0.130357142857, -0.148571428571, 0.0f },
  { 0.136904761905, -0.144761904762, 0.0f },
  { 0.135714285714, -0.27619047619, 0.0f },
  { 0.151785714286, -0.270476190476, 0.0f },
  { 0.145833333333, -0.269523809524, 0.0f },
  { 0.147023809524, -0.265714285714, 0.0f },
  { 0.154761904762, -0.258095238095, 0.0f },
  { 0.139880952381, -0.26380952381, 0.0f },
  { 0.141071428571, -0.260952380952, 0.0f },
  { 0.144642857143, -0.255238095238, 0.0f },
  { 0.14880952381, -0.25619047619, 0.0f },
  { 0.0678571428571, -0.294285714286, 0.0f },
  { 0.0654761904762, -0.288571428571, 0.0f },
  { 0.0702380952381, -0.301904761905, 0.0f },
  { 0.0666666666667, -0.29619047619, 0.0f },
  { 0.0547619047619, -0.294285714286, 0.0f },
  { 0.0583333333333, -0.286666666667, 0.0f },
  { 0.0541666666667, -0.285714285714, 0.0f },
  { 0.0494047619048, -0.279047619048, 0.0f },
  { 0.0458333333333, -0.277142857143, 0.0f },
  { 0.05, -0.271428571429, 0.0f },
  { 0.0452380952381, -0.271428571429, 0.0f },
  { -0.0952380952381, -0.198095238095, 0.0f },
  { -0.0988095238095, -0.195238095238, 0.0f },
  { -0.0940476190476, -0.185714285714, 0.0f },
  { -0.0797619047619, -0.172380952381, 0.0f },
  { -0.0827380952381, -0.169523809524, 0.0f },
  { -0.0732142857143, -0.317142857143, 0.0f },
  { -0.0708333333333, -0.311428571429, 0.0f },
  { -0.0446428571429, -0.33619047619, 0.0f },
  { -0.0458333333333, -0.328571428571, 0.0f },
  { 0.089880952381, -0.308571428571, 0.0f },
  { 0.0994047619048, -0.200952380952, 0.0f },
  { 0.101785714286, -0.202857142857, 0.0f },
  { 0.104166666667, -0.20380952381, 0.0f },
  { 0.102976190476, -0.190476190476, 0.0f },
  { 0.101785714286, -0.195238095238, 0.0f },
  { 0.101785714286, -0.198095238095, 0.0f },
  { 0.106547619048, -0.199047619048, 0.0f },
  { 0.114285714286, -0.194285714286, 0.0f },
  { 0.150595238095, -0.138095238095, 0.0f },
  { 0.113095238095, -0.118095238095, 0.0f },
  { 0.109523809524, -0.114285714286, 0.0f },
  { 0.111904761905, -0.121904761905, 0.0f },
  { 0.0767857142857, -0.13619047619, 0.0f },
  { 0.0803571428571, -0.130476190476, 0.0f },
  { 0.0833333333333, -0.134285714286, 0.0f },
  { 0.0839285714286, -0.138095238095, 0.0f },
  { -0.0321428571429, -0.362857142857, 0.0f },
  { -0.039880952381, -0.357142857143, 0.0f },
  { -0.0369047619048, -0.36380952381, 0.0f },
  { -0.0321428571429, -0.354285714286, 0.0f },
  { -0.0357142857143, -0.353333333333, 0.0f },
  { -0.0458333333333, -0.32, 0.0f },
  { -0.0494047619048, -0.313333333333, 0.0f },
  { -0.0511904761905, -0.322857142857, 0.0f },
  { -0.0315476190476, -0.281904761905, 0.0f },
  { -0.0315476190476, -0.277142857143, 0.0f },
  { -0.0267857142857, -0.28, 0.0f },
  { -0.0761904761905, -0.246666666667, 0.0f },
  { -0.0809523809524, -0.246666666667, 0.0f },
  { -0.077380952381, -0.239047619048, 0.0f },
  { -0.0904761904762, -0.240952380952, 0.0f },
  { -0.0934523809524, -0.23619047619, 0.0f },
  { -0.0886904761905, -0.235238095238, 0.0f },
  { -0.0857142857143, -0.0695238095238, 0.0f },
  { -0.0827380952381, -0.0685714285714, 0.0f },
  { -0.0886904761905, -0.0609523809524, 0.0f },
  { -0.0886904761905, -0.0504761904762, 0.0f },
  { -0.0833333333333, -0.0552380952381, 0.0f },
  { -0.0857142857143, -0.0466666666667, 0.0f },
  { -0.0869047619048, -0.000952380952381, 0.0f },
  { -0.089880952381, 0.000952380952381, 0.0f },
  { -0.0839285714286, 0.00666666666667, 0.0f },
  { 0.12380952381, -0.271428571429, 0.0f },
  { 0.0988095238095, -0.290476190476, 0.0f },
  { 0.101785714286, -0.289523809524, 0.0f },
  { 0.0708333333333, -0.298095238095, 0.0f },
};

void SkeletonParticles::moveSwarm(bool move)
{
  mMoveSwarm = move;
  if (!mMoveSwarm)
  {
    float offset = (mUserToken == KinectWrapper::utClosest) ? (mCoordSpace.x / 7.5f) : -(mCoordSpace.x / 6.0f);
    Vec3f center((mCoordSpace.x / 2.0f) + offset, mCoordSpace.y / 2.0f, 0.0f);
    for (int i = 0; i < NUM_SKELETON_PARTICLES; i++)
    {
      mParticlePos[i].x = initial_positions[i][0];
      mParticlePos[i].y = initial_positions[i][1];
      mParticlePos[i].z = initial_positions[i][2];
      if (mParticlePos[i].x > 0)
        mParticleVel[i] = Vec3f(1.0f, 0.0f, 0.0f);
      else
        mParticleVel[i] = Vec3f(-1.0f, 0.0f, 0.0f);
      mParticlePos[i].x *= mCoordSpace.x;
      mParticlePos[i].y *= mCoordSpace.y;
      mParticlePos[i] += center;
    }
    mParticleSize = Vec2f(12.0f, 12.0f);
  } else {
    mParticleSize = Vec2f(16.0f, 16.0f);
  }
}

void SkeletonParticles::mouseDown(cinder::app::MouseEvent event)
{
#if 0
  static int idx = 0;
  if (idx < NUM_SKELETON_PARTICLES)
  {
    if (idx == 0)
    {
      printf("window: %d, %d\n", getWindowWidth(), getWindowHeight());
    }
    mParticlePos[idx] = Vec3f(event.getX(), event.getY(), 0.0f);
    printf("%d, %f, %f, %f\n", idx, mParticlePos[idx].x, mParticlePos[idx].y, mParticlePos[idx].z);
    idx++;
  }
#endif
}

void SkeletonParticles::updateWaiting()
{
  float offset = (mUserToken == KinectWrapper::utClosest) ? (mCoordSpace.x / 7.5f) : -(mCoordSpace.x / 6.0f);
  Vec3f center((mCoordSpace.x / 2.0f) + offset, mCoordSpace.y / 2.0f, 0.0f);
  
 	for (int i = 0; i < NUM_SKELETON_PARTICLES; i++)
	{
		mParticlePos[i] += mParticleVel[i];
    Vec3f targetPos = Vec3f(initial_positions[i][0], initial_positions[i][1], 0.0f);
    targetPos.x *= mCoordSpace.x;
    targetPos.y *= mCoordSpace.y;
    targetPos += center;
    Vec3f diff = targetPos - mParticlePos[i];
    mParticleVel[i] += dumbRand() * 1.0f;
    mParticleVel[i] += diff * 0.30;
    mParticleVel[i] *= mDrag;
  }
}

void SkeletonParticles::init()
{
  SceneLayer::init();
  
  registerParam("move");
  registerParam("drop");
}

void SkeletonParticles::update()
{
  if (getParamValue("enabled") > 0.5f && !mEnabled)
    setEnabled(getParamValue("enabled") > 0.5f);
  mDropping = getParamValue("drop") > 0.5f;
  bool newMove = getParamValue("move") > 0.5f;
  if (newMove != mMoveSwarm)
  {
    moveSwarm(newMove);
  }
}
