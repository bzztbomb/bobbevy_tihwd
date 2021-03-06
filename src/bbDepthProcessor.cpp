/*
 *  bbDepthProcessor.cpp
 *  bobbevy
 *
 *  Created by Brian Richardson on 6/30/12.
 *  Copyright 2012 Knowhere Studios Inc. All rights reserved.
 *
 */

#include "bbDepthProcessor.h"
#include <memory.h>
#include <algorithm>
#include <boost/filesystem.hpp>
#include "cinder/qtime/QuickTime.h"
#include "cinder/qtime/MovieWriter.h"
#include "cinder/ImageIo.h"
#include "Kinect.h"

using namespace ci;
using namespace ci::app;
using namespace std;

int DepthProcessor::smMAX_BLOBS = 3;
Vec2i DepthProcessor::smSize(640, 480);

bool cmpX(const cinder::Vec2f& a, const cinder::Vec2f& b)
{
  return a.x < b.x;
}

class KinectDepthSource : public DepthSource
{
public:
  KinectDepthSource(params::InterfaceGl& params)
  : mTilt(0)
  , mEnableIR(false)
  {
    params.addParam( "Tilt", &mTilt, "min=-31 max=32");
    params.addParam( "Toggle IR", &mEnableIR);
  }
  
  virtual void init()
  {
    console() << "There are " << Kinect::getNumDevices() << " Kinects connected." << std::endl;
    if (Kinect::getNumDevices() > 0)
    {
      mKinect = Kinect( Kinect::Device() ); // the default Device implies the first Kinect
      mTilt = mKinect.getTilt();
    }
  }
  
  virtual bool newData()
  {
    return mKinect.mObj ? mKinect.checkNewDepthFrame() : false;
  }
  
  virtual cinder::ImageSourceRef getDepthImage()
  {
    return mKinect.mObj ? mKinect.getDepthImage() : ImageSourceRef();
  }
  
  virtual cinder::ImageSourceRef getVideoImage()
  {
    return mKinect.mObj ? mKinect.getVideoImage() : ImageSourceRef();
  }
  
  virtual void update()
  {
    if (!mKinect.mObj)
      return;
    if (mKinect.getTilt() != mTilt)
      mKinect.setTilt(mTilt);
    if (mEnableIR != mKinect.isVideoInfrared())
      mKinect.setVideoInfrared(mEnableIR);
  }
private:
	cinder::Kinect	mKinect;
  int32_t mTilt;
  bool mEnableIR;
};

class FakeDepthSource : public DepthSource
{
public:
  FakeDepthSource()
  : mFakeSurface(640, 480, false, SurfaceChannelOrder::RGB)
  , mInitialFrames(0)
  {
  }
  
  void updateFakeBlob(int index, const Vec2f& pos)
  {
    BlobLock b(mBlobMutex);
    if (index < NUM_FAKE_BLOB_PTS)
      mFakeBlobs[index] = pos;
    mFakeBlobs[index].x *= 640.0f;
    mFakeBlobs[index].y *= 480.0f;
    mFakeDataAvail = true;
  }
  
  virtual void init()
  {
    BlobLock b(mBlobMutex);
    mInitialFrames = 0;
    for (int i = 0; i < NUM_FAKE_BLOB_PTS; i++)
      mFakeBlobs[i] = Vec2f();
  }
  
  virtual void update()
  {
    if (mInitialFrames++ > 60)
      return;
    BlobLock b(mBlobMutex);
    mFakeDataAvail = true;
  }

  virtual bool newData()
  {
    BlobLock b(mBlobMutex);
    bool ret = mFakeDataAvail;
    mFakeDataAvail = false;
    return ret;
  }

  virtual cinder::ImageSourceRef getDepthImage()
  {
    BlobLock b(mBlobMutex);
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
    return mFakeSurface;
  }
  
  virtual cinder::ImageSourceRef getVideoImage()
  {
    return mFakeSurface;
  }
private:
  // Lack of Kinect interface
  cinder::Vec2f mFakeBlobs[NUM_FAKE_BLOB_PTS];
  bool mFakeDataAvail;
  cinder::Surface8u mFakeSurface;
  int mInitialFrames;
  std::recursive_mutex mBlobMutex;
  typedef std::lock_guard<std::recursive_mutex> BlobLock;
};

class VideoDepthSource : public DepthSource
{
public:
  virtual ~VideoDepthSource() { };
  
  virtual void init()
  {
    try
    {
      std::string base("/Users/bzztbomb/projects/bobbevy/assets/0000000000_");
      
      mDepthMovie = qtime::MovieSurface::create(base + "depth.mov");
      if (!mDepthMovie)
        return;
      mColorMovie = qtime::MovieSurface::create(base + "color.mov");
      mDepthMovie->setLoop();
      mColorMovie->setLoop();      
      mDepthMovie->play();
      mColorMovie->play();
    }
    catch (const cinder::qtime::QuickTimeExc& e)
    {
      mDepthMovie = NULL;
      mColorMovie = NULL;
    }
  }
  
  virtual void stop()
  {
    mDepthMovie->stop();
    mColorMovie->stop();
    mDepthMovie = NULL;
    mColorMovie = NULL;
  }
  
  // Update maybe temp?
  virtual void update()
  {
  }
  
  // Is new depth data available?
  virtual bool newData()
  {
    return mDepthMovie ? mDepthMovie->checkNewFrame() : false;
  }
  
  // These two should only get called when newData = true
  virtual cinder::ImageSourceRef getDepthImage()
  {
    return mDepthMovie->getSurface();
  }
  
  virtual cinder::ImageSourceRef getVideoImage()
  {
    return mColorMovie->getSurface();
  }
private:
  qtime::MovieSurfaceRef mDepthMovie;
  qtime::MovieSurfaceRef mColorMovie;
};


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

DepthProcessor::DepthProcessor() :
  mRecordRequested(false),
  mRecord(false),
  mLastGray(480, 640, CV_32F),
  mInitInitial(0),
  mInitFrames(30),
  mDepthLowPass(240),
  mDepthType(dsKinect),
  mCurrentDepthType(dsKinect),
  mIndexFG(0),
  mDepthSurfaces(3),
  mColorSurfaces(3),
  mContourSurfaces(3),
  mStopProcessing(false),
  mContourDirty(false)
{
  
}

DepthProcessor::~DepthProcessor()
{
  mStopProcessing = true;
  mProcessingThread.join();
}


void DepthProcessor::setup(params::InterfaceGl& params)
{
	mStepFrom = 1;
	mAreaThreshold = 2000.0f;
  mDrawTex = dtDepth;

  std::vector<std::string> enumSource;
  enumSource.push_back("None");
  enumSource.push_back("Kinect");
  enumSource.push_back("Fake");
  enumSource.push_back("Recorded");
  params.addParam( "Depth Source", enumSource, (int*) &mDepthType);
	params.addParam( "Init frame amount", &mInitFrames, "min=0 max=300" );
	params.addParam( "Step from", &mStepFrom, "min=1 max=255" );
  params.addParam( "Depth LowPass filter", &mDepthLowPass, "min=0 max=255");
	params.addParam( "CV area threshold", &mAreaThreshold, "min=1");
  std::vector<std::string> enumDraw;
  enumDraw.push_back("Depth");
  enumDraw.push_back("Color");
  enumDraw.push_back("Contour");
  enumDraw.push_back("Background");
  params.addParam( "Texture", enumDraw, &mDrawTex);
  params.addParam( "Record Depth Data", &mRecordRequested);

  mKinectDepthSource = std::make_shared<KinectDepthSource>(params);
  mDepthSource = mKinectDepthSource;
  mFakeDepthSource = std::make_shared<FakeDepthSource>();
  mVideoDepthSource = std::make_shared<VideoDepthSource>();
  
  mProcessingThread = std::thread(std::bind(&DepthProcessor::threadFunc, this));
}

void DepthProcessor::enableRecordIfNeeded()
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

void DepthProcessor::resetBackground()
{
  mInitInitial = 0;
}

void DepthProcessor::keyDown( KeyEvent event )
{
	switch( event.getCode() ){
		case KeyEvent::KEY_a:
      resetBackground();
			break;
		default:
			break;
	}
}

void DepthProcessor::update()
{
  Surface d;
  while (mDepthSurfaces.isNotEmpty())
    mDepthSurfaces.popBack(&d);
  if (d)
    mDepthTexture = d;
  
  mContourDirty = mContourSurfaces.isNotEmpty();
  while (mContourSurfaces.isNotEmpty())
    mContourSurfaces.popBack(&mContourMat);
  
  Surface c;
  while (mColorSurfaces.isNotEmpty())
    mColorSurfaces.popBack(&c);
  if (c)
    mColorTexture = c;
}

cinder::gl::Texture DepthProcessor::getContourTexture()
{
  if (mContourDirty)
    mContourTexture = fromOcv(mContourMat);
  return mContourTexture;
}

void DepthProcessor::threadFunc()
{
  ci::ThreadSetup threadSetup; // instantiate this if you're talking to Cinder from a secondary thread

  mDepthSource->init();
  
  while (!mStopProcessing)
  {
    if (mDepthType != mCurrentDepthType)
    {
      if (mDepthSource)
        mDepthSource->stop();
      switch (mDepthType)
      {
        case dsKinect :
          mDepthSource = mKinectDepthSource;
          break;
        case dsFake :
          mDepthSource = mFakeDepthSource;
          break;
        case dsRecorded :
          mDepthSource = mVideoDepthSource;
          break;
        default :
          mDepthSource = NULL;
          break;
      }
      if (mDepthSource)
        mDepthSource->init();
      mCurrentDepthType = mDepthType;
      resetBackground();
    }

    if (!mDepthSource)
      return;
    
    enableRecordIfNeeded();
    
    mDepthSource->update();
    
    findBlobs();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

bool DepthProcessor::getDepthData()
{
  if ((!mDepthSource) || (!mDepthSource->newData()))
    return false;
  
  ImageSourceRef d = mDepthSource->getDepthImage();
  if (mRecord)
    mDepthWriter.addFrame(d);
  if (mDrawTex == dtDepth)
    mDepthSurfaces.pushFront(d);

  bool showingColor = mDrawTex == dtColor;
  if (showingColor || mRecord)
  {
    ImageSourceRef c = mDepthSource->getVideoImage();
    if (c && mRecord)
      mColorWriter.addFrame(c);
    if (c && showingColor)
      mColorSurfaces.pushFront(c);
  }
  
  return true;
}

void DepthProcessor::findBlobs()
{
  if (!getDepthData())
    return;
	
	Surface8u to8(mDepthSource->getDepthImage());
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
  
  auto& blobs = mBlobs[1 - mIndexFG];
  
	blobs.clear();
	float largest = mAreaThreshold;
  
  mContourSurfaces.pushFront(thresh.clone());
  
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
      blobs.push_back(b);
      push_heap(blobs.begin(), blobs.end(), SortDescendingArea());
      if (blobs.size() > DepthProcessor::smMAX_BLOBS)
      {
        blobs.erase(blobs.end()-1);
        largest = blobs.rbegin()->mContourArea;
      }
    }
  }
  
	for (BlobVector::iterator i = blobs.begin(); i != blobs.end(); i++)
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
      if (thresh.at<uint8_t>(cv::Point(sample.x, sample.y)) > 0)
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
      if (thresh.at<uint8_t>(cv::Point(sample.x, sample.y)) > 0)
      {
        float val = *to8.getDataRed(Vec2f(sample.x, sample.y));
        if (val < mDepthLowPass)
          i->mZDist = max(i->mZDist, val);
      }
      sample += step;
		}
	}
	sort(blobs.begin(), blobs.end(), SortDescendingZ());
  {
    BlobLock b(mBlobMutex);
    mIndexFG = 1 - mIndexFG;
  }
}

void DepthProcessor::draw()
{  
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
      getContourTexture();
      if (mContourTexture)
        gl::draw(mContourTexture, getWindowBounds());
      break;
    case dtBackground :
      {
        if (mInitInitial >= mInitFrames)
        {
          cinder::gl::Texture tex(fromOcv(mInitial));
          gl::draw(tex, getWindowBounds());
        }
      }
      break;
  }
	
	if (true)
	{
    BlobLock b(mBlobMutex);
    Vec2f ws((float) getWindowWidth() / 640.0f, (float) getWindowHeight() / 480.0f);
    
		// draw the contours
		int c = 0;
		for (BlobVector::iterator i = mBlobs[mIndexFG].begin(); i != mBlobs[mIndexFG].end(); i++)
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

BlobRef DepthProcessor::getUser(UserToken which)
{
  BlobLock b(mBlobMutex);
  auto& blobs = mBlobs[mIndexFG];
  switch (which)
  {
    case utClosest:
      if (blobs.size() > 0)
        return std::make_shared<Blob>(*blobs.rbegin());
      break;
    case utFurthest :
      if (blobs.size() > 0)
        return std::make_shared<Blob>(*blobs.begin());
      break;
    default:
      return NULL;
  }
  return NULL;
}

std::vector<Blob> DepthProcessor::getUsers()
{
  BlobLock b(mBlobMutex);
  return mBlobs[mIndexFG];
}

void DepthProcessor::updateFakeBlob(int index, const Vec2f& pos)
{
  if (mFakeDepthSource)
    mFakeDepthSource->updateFakeBlob(index, pos);
}
