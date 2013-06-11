/*
 *  bbKinectWrapper.cpp
 *  bobbevy
 *
 *  Created by Brian Richardson on 6/30/12.
 *  Copyright 2012 Knowhere Studios Inc. All rights reserved.
 *
 */

#include "bbKinectWrapper.h"
#include <memory.h>
#include <algorithm>
#include <boost/filesystem.hpp>
#include "cinder/qtime/MovieWriter.h"
#include "cinder/ImageIo.h"

using namespace ci;
using namespace ci::app;
using namespace std;

int KinectWrapper::smMAX_BLOBS = 3;
Vec2i KinectWrapper::smSize(640, 480);

struct SortDescendingArea
{
  bool operator()(const Blob& t1, const Blob& t2) const
  {
    return t1.mContourArea < t2.mContourArea;
  }
};
struct SortDescendingZ
{
  bool operator()(const Blob& t1, const Blob& t2) const
  {
    return t1.mZDist < t2.mZDist;
  }
};

KinectWrapper::KinectWrapper() :
  mFakeSurface(640, 480, false, SurfaceChannelOrder::RGB),
  mRecordRequested(false),
  mRecord(false),
  mLastGray(480, 640, CV_32F),
  mInitInitial(0),
  mInitFrames(30),
  mDepthLowPass(240)
{
  
}

void KinectWrapper::setup(params::InterfaceGl& params)
{
	console() << "There are " << Kinect::getNumDevices() << " Kinects connected." << std::endl;
	mKinectEnabled = (Kinect::getNumDevices() > 0);
  if (mKinectEnabled)
  {
    mKinect = Kinect( Kinect::Device() ); // the default Device implies the first Kinect
    mTilt = mKinect.getTilt();
  }
	mEnabled = true;
  mBlobsEnabled = true;
  
	mStepFrom = 1;
	mAreaThreshold = 2000.0f;
  mDrawTex = dtDepth;
  mEnableIR = false;

	params.addParam( "Init frame amount", &mInitFrames, "min=1 max=300" );
	params.addParam( "Step from", &mStepFrom, "min=1 max=255" );
  params.addParam( "Depth LowPass filter", &mDepthLowPass, "min=0 max=255");
	params.addParam( "CV area threshold", &mAreaThreshold, "min=1");
  std::vector<std::string> enumDraw;
  enumDraw.push_back("Depth");
  enumDraw.push_back("Color");
  enumDraw.push_back("Contour");
  enumDraw.push_back("Background");
  params.addParam( "Texture", enumDraw, &mDrawTex);
  params.addParam( "Toggle IR", &mEnableIR);
  params.addParam( "KinectEnabled", &mEnabled);
  params.addParam( "BlobsEnabled", &mBlobsEnabled);
  params.addParam( "Record Kinect Data", &mRecordRequested);
  params.addParam( "Tilt", &mTilt, "min=-31 max=32");
}

void KinectWrapper::enableRecordIfNeeded()
{
  if (mRecord == mRecordRequested)
    return;
  if (mRecord)
  {
    // We are recording and should stop
    mDepthWriter.finish();
    mColorWriter.finish();
  } else {
    // We should start recording
    fs::path depth_path;
    fs::path color_path;
    fs::path bg_path;
    for (int i = 0; ; i++)
    {
      char buffer[32];
      sprintf(buffer, "../%.10d_depth.mov", i);
      depth_path = getAppPath() / buffer;
      sprintf(buffer, "../%.10d_color.mov", i);
      color_path = getAppPath() / buffer;
      sprintf(buffer, "../%.10d_background.png", i);
      bg_path = getAppPath() / buffer;
      if ((!boost::filesystem::exists(depth_path)) &&
          (!boost::filesystem::exists(color_path)) &&
          (!boost::filesystem::exists(bg_path)))
        break;
    }
    qtime::MovieWriter::Format format('raw ', 1.0);
    mDepthWriter = qtime::MovieWriter(depth_path, 640, 480, format);
    mColorWriter = qtime::MovieWriter(color_path, 640, 480, format);
    Surface8u init(fromOcv(mInitial));
    cinder::writeImage(bg_path, init);
  }
  mRecord = mRecordRequested;
}

void KinectWrapper::resetBackground()
{
  mInitInitial = 0;
}

void KinectWrapper::keyDown( KeyEvent event )
{
	if (!mEnabled)
		return;
  
	switch( event.getCode() ){
		case KeyEvent::KEY_a:
      resetBackground();
			break;
    case KeyEvent::KEY_7:
      mKinectEnabled = false;
      break;
    case KeyEvent::KEY_l:
      if (Kinect::getNumDevices() > 0)
        mKinectEnabled = true;
      break;
		default:
			break;
	}
}

void KinectWrapper::update()
{
	if (!mEnabled)
		return;
  
  if (mKinectEnabled)
  {
    if (mKinect.getTilt() != mTilt)
      mKinect.setTilt(mTilt);
  }
  
  enableRecordIfNeeded();
  
  if ((mKinectEnabled) && (mEnableIR != mKinect.isVideoInfrared()))
    mKinect.setVideoInfrared(mEnableIR);
  
  findBlobs();
}

bool cmpX(const cinder::Vec2f& a, const cinder::Vec2f& b)
{
  return a.x < b.x;
}

bool KinectWrapper::getDepthData()
{
  if (mKinectEnabled)
  {
    bool newDepth = false;
    if( mKinect.checkNewDepthFrame() )
    {
      newDepth = true;
      ImageSourceRef d = mKinect.getDepthImage();
      mDepthTexture = d;
      if (mRecord)
        mDepthWriter.addFrame(d);
    }
    
    if( mKinect.checkNewVideoFrame() )
    {
      ImageSourceRef c = mKinect.getVideoImage();
      mColorTexture = c;
      if (mRecord)
        mColorWriter.addFrame(c);
    }
    
    return newDepth;
  } else {
    if (!mFakeDataAvail)
      return false;
    mFakeDataAvail = false;
    
    memset(mFakeSurface.getData(), 0, mFakeSurface.getRowBytes() * mFakeSurface.getHeight());
    
    cinder::Vec2f fakeBlobs[NUM_FAKE_BLOB_PTS];
    memcpy(&fakeBlobs[0], &mFakeBlobs[0], sizeof(fakeBlobs));
    
    sort(fakeBlobs, fakeBlobs+NUM_FAKE_BLOB_PTS, cmpX);
    
    for (int index = 0; index < NUM_FAKE_BLOB_PTS; index+=2)
    {
      Vec2f minV(min(fakeBlobs[index].x, fakeBlobs[index+1].x),
                 min(fakeBlobs[index].y, fakeBlobs[index+1].y));
      Vec2f maxV(max(fakeBlobs[index].x, fakeBlobs[index+1].x),
                 max(fakeBlobs[index].y, fakeBlobs[index+1].y));
      uint8_t* d = mFakeSurface.getData(minV);
      int value = index==0 ? 192 : 128;
      for (int i = minV.y; i < maxV.y; i++)
      {
        memset(d, value, (maxV.x-minV.x)*mFakeSurface.getPixelInc());
        d += mFakeSurface.getRowBytes();
      }
    }
    mDepthTexture = mFakeSurface;
    return true;
  }
}

void KinectWrapper::findBlobs()
{
  bool newDepth = getDepthData();
	
	if ((!mDepthTexture) || (!newDepth) || (!mBlobsEnabled))
		return;
	
	Surface8u to8;
  if (mKinectEnabled)
    to8 = mKinect.getDepthImage();
  else
    to8 = mFakeSurface;
	cv::Mat input( toOcv( to8));
	
	cv::Mat gray;
	cv::Mat thresh;
	
  cv::cvtColor( input, gray, CV_RGB2GRAY );
  
  cv::threshold(gray, gray, mDepthLowPass, 0, CV_THRESH_TOZERO_INV);
  
  if (mInitInitial < mInitFrames)
  {
    if (mInitInitial == 0)
      mInitial = gray.clone();
    else
      mInitial = cv::max(gray, mInitial);
    mInitInitial++;
    return;
  }
  
  gray -= mInitial;

  cv::accumulateWeighted(gray, mLastGray, 0.9);
  mLastGray.convertTo(gray, CV_8U);
  
  cv::threshold( gray, thresh, mStepFrom, 255, CV_THRESH_BINARY );
  
	mBlobs.clear();
	float largest = mAreaThreshold;
  
  mContourMat = thresh.clone();
  mContourTexture = fromOcv(mContourMat);
  
  ContourVector vec;
  cv::findContours( thresh, vec, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );
  
  for( ContourVector::iterator iter = vec.begin(); iter != vec.end(); ++iter )
  {
    float a = cv::contourArea(*iter);
    if (a > largest)
    {
      Blob b;
      b.mContourArea = a;
      b.mContourPoints.resize(iter->size());
      copy(iter->begin(), iter->end(), b.mContourPoints.begin());
      mBlobs.push_back(b);
      push_heap(mBlobs.begin(), mBlobs.end(), SortDescendingArea());
      if (mBlobs.size() > KinectWrapper::smMAX_BLOBS)
      {
        mBlobs.erase(mBlobs.end()-1);
        largest = mBlobs.rbegin()->mContourArea;
      }
    }
  }
  
	for (BlobVector::iterator i = mBlobs.begin(); i != mBlobs.end(); i++)
	{
		i->mCentroid.x = i->mCentroid.y = 0.0f;
    float mag = 10000.0f;
		i->mLeftMost.x = mag;
		i->mRightMost.x = -mag;
		i->mTopMost.y = mag;
		i->mBottomMost.y = -mag;
    i->mBounds.x1 = i->mBounds.y1 = mag;
    i->mBounds.x2 = i->mBounds.y2 = -mag;
		for (vector<cv::Point>::iterator pt = i->mContourPoints.begin(); pt != i->mContourPoints.end(); ++pt)
		{
			i->mCentroid.x += pt->x;
			i->mCentroid.y += pt->y;
      i->mBounds.include(Vec2f(pt->x, pt->y));
			if (i->mLeftMost.x > pt->x)
			{
				i->mLeftMost.x = pt->x;
				i->mLeftMost.y = pt->y;
			}
			if (i->mRightMost.x < pt->x)
			{
				i->mRightMost.x = pt->x;
				i->mRightMost.y = pt->y;
			}
			if (i->mTopMost.y > pt->y)
			{
				i->mTopMost.x = pt->x;
				i->mTopMost.y = pt->y;
			}
			if (i->mBottomMost.y < pt->y)
			{
				i->mBottomMost.x = pt->x;
				i->mBottomMost.y = pt->y;
			}
		}
		float sz = i->mContourPoints.size();
		if (sz > 0.0f)
		{
			i->mCentroid.x /= sz;
			i->mCentroid.y /= sz;
		}     
		i->mZDist = *to8.getDataRed(i->mCentroid);
		// Loop through and do z calc
		float steps = 10.0f;
		Vec3f step = (i->mBottomMost - i->mTopMost) / steps;
		Vec3f sample = i->mTopMost;
		for (int x = 0; x < steps; x++)
		{
      if (mContourMat.at<uint8_t>(cv::Point(sample.x, sample.y)) > 0)
      {
        float val = *to8.getDataRed(Vec2f(sample.x, sample.y));
        if (val < mDepthLowPass) 
          i->mZDist = max(i->mZDist, val);
      }
      sample += step; 
		}
		step = (i->mRightMost - i->mLeftMost) / steps;
		sample = i->mLeftMost;
		for (int x = 0; x < steps; x++)
		{
      if (mContourMat.at<uint8_t>(cv::Point(sample.x, sample.y)) > 0)
      {
        float val = *to8.getDataRed(Vec2f(sample.x, sample.y));
        if (val < mDepthLowPass)
          i->mZDist = max(i->mZDist, val);
      }
      sample += step;
		}
	}
	sort(mBlobs.begin(), mBlobs.end(), SortDescendingZ());
}

void KinectWrapper::draw()
{
	if (!mEnabled)
		return;
  
  glDisable(GL_TEXTURE_2D);
	gl::color(Color(1.0f, 1.0f, 1.0f));
	gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
  switch (mDrawTex)
  {
    case dtDepth:
      if (mDepthTexture)
        gl::draw(mDepthTexture, getWindowBounds());
      break;
    case dtColor:
      if (mColorTexture)
        gl::draw(mColorTexture, getWindowBounds());
      break;
    case dtContour :
      if (mContourTexture)
        gl::draw(mContourTexture, getWindowBounds());
      break;
    case dtBackground :
      {
        cinder::gl::Texture tex(fromOcv(mInitial));
        gl::draw(tex, getWindowBounds());
      }
      break;
  }
	
	if (true)
	{
    Vec2f ws((float) getWindowWidth() / 640.0f, (float) getWindowHeight() / 480.0f);
    
		// draw the contours
		int c = 0;
		for (BlobVector::iterator i = mBlobs.begin(); i != mBlobs.end(); i++)
		{
			glBegin(GL_LINE_LOOP);
			for( vector<cv::Point>::iterator pt = i->mContourPoints.begin(); pt != i->mContourPoints.end(); ++pt )
			{
				switch (c)
				{
					case 0:
						gl::color( Color( 1.0f, 0.0f, 0.0f ) );
						break;
					case 1:
						gl::color( Color( 0.0f, 1.0f, 0.0f ) );
						break;
					case 2:
						gl::color( Color( 0.0f, 0.0f, 1.0f ) );
						break;
				}
				
				gl::vertex( Vec2f(pt->x * ws.x, pt->y * ws.y) );
			}
			glEnd();
      
			gl::drawSolidCircle(i->mCentroid * ws, 10);
      Rectf scaledBounds(i->mBounds);
      scaledBounds.x1 *= ws.x;
      scaledBounds.y1 *= ws.y;
      scaledBounds.x2 *= ws.x;
      scaledBounds.y2 *= ws.y;
      gl::drawStrokedRect(scaledBounds);
			c++;
		}
	}
  
  gl::color(Color(1,1,1));
}

Blob* KinectWrapper::getUser(UserToken which)
{
  switch (which)
  {
    case utClosest:
      if (mBlobs.size() > 0)
        return &(*mBlobs.rbegin());
      break;
    case utFurthest :
      if (mBlobs.size() > 0)
        return &(*mBlobs.begin());
      break;
    default:
      return NULL;
  }
  return NULL;
}

std::vector<Blob> KinectWrapper::getUsers()
{
  return mBlobs;
}

void KinectWrapper::updateFakeBlob(int index, const Vec2f& pos)
{
  if (index < 4)
    mFakeBlobs[index] = pos;
  mFakeBlobs[index].x *= 640.0f;
  mFakeBlobs[index].y *= 480.0f;
  mFakeDataAvail = true;
}
