/*
 *  bbDepthProcessor.h
 *  bobbevy
 *
 *  Created by Brian Richardson on 6/30/12.
 *  Copyright 2012 Knowhere Studios Inc. All rights reserved.
 *
 */
#ifndef __BBDepthProcessor_H_
#define __BBDepthProcessor_H_

#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "CinderOpenCV.h"
#include "cinder/app/AppBasic.h"
#include "cinder/params/Params.h"
#include "cinder/gl/Fbo.h"
#include "cinder/qtime/MovieWriter.h"
#include "cinder/Vector.h"
#include "cinder/ConcurrentCircularBuffer.h"

struct Blob {
	float mContourArea;
	std::vector<cv::Point> mContourPoints;
	cinder::Vec2f mCentroid;
	float mZDist;
	// Bounds
  cinder::Rectf mBounds;
	cinder::Vec3f mLeftMost;
	cinder::Vec3f mRightMost;
	cinder::Vec3f mTopMost;
	cinder::Vec3f mBottomMost;
};

typedef std::shared_ptr<Blob> BlobRef;

class DepthSource
{
public:
  virtual ~DepthSource() { };
  virtual void init() { }
  virtual void stop() { }
  // Update maybe temp?
  virtual void update() = 0;
  // Is new depth data available?
  virtual bool newData() = 0;
  // These two should only get called when newData = true
  virtual cinder::ImageSourceRef getDepthImage() = 0;
  virtual cinder::ImageSourceRef getVideoImage() = 0;
};

typedef std::shared_ptr<DepthSource> DepthSourceRef;

#define NUM_FAKE_BLOB_PTS 4

class FakeDepthSource;

class DepthProcessor
{
public:
  static cinder::Vec2i smSize;
public:
  DepthProcessor();
  virtual ~DepthProcessor();
  
  enum UserToken
  {
    utClosest,
    utFurthest
  };
  
	void setup(cinder::params::InterfaceGl& params);
	void update();
	void keyDown( cinder::app::KeyEvent event );
	void draw();
  
  void resetBackground();
  
  BlobRef getUser(UserToken which);
  std::vector<Blob> getUsers();
  void updateFakeBlob(int index, const cinder::Vec2f& pos);
  
  cv::Mat* getContourMat() { return &mContourMat; }
  cinder::gl::Texture getContourTexture();
protected:
  std::thread mProcessingThread;
  std::recursive_mutex mBlobMutex;
  typedef std::lock_guard<std::recursive_mutex> BlobLock;
  bool mStopProcessing;
  
  cinder::ConcurrentCircularBuffer<cinder::Surface> mDepthSurfaces;
  cinder::ConcurrentCircularBuffer<cinder::Surface> mColorSurfaces;
  cinder::ConcurrentCircularBuffer<cv::Mat> mContourSurfaces;
  
  enum DepthSourceType
  {
    dsNone = 0,
    dsKinect = 1,
    dsFake = 2,
    dsRecorded = 3
  };
  DepthSourceType mDepthType;
  DepthSourceType mCurrentDepthType;
  DepthSourceRef mDepthSource;
  DepthSourceRef mKinectDepthSource;
  DepthSourceRef mVideoDepthSource;
  std::shared_ptr<FakeDepthSource> mFakeDepthSource;
  
	cinder::gl::Texture		mColorTexture, mDepthTexture;
    
	cv::Mat mInitial;
  cv::Mat mLastGray; // Accumlated grayscal image
	int mInitInitial;
  int mInitFrames;

	// Image processing
	int mStepFrom;
  int mDepthLowPass;
  
	// Debug
	cinder::gl::Texture	mContourTexture;
	cv::Mat mContourMat;
  bool mContourDirty;
  
  enum DrawTex
  {
    dtDepth = 0,
    dtColor = 1,
    dtContour = 2,
    dtBackground = 3
  };
  int mDrawTex;
  bool mEnableIR;
  
  bool mRecordRequested;
  bool mRecord;
  cinder::qtime::MovieWriter mDepthWriter;
  cinder::qtime::MovieWriter mColorWriter;
  
	// Blob detection and "user tracking"
	static int smMAX_BLOBS;
  typedef std::vector<Blob> BlobVector;
  BlobVector mBlobs[2];
  int mIndexFG;
  float mAreaThreshold;
  typedef std::vector< std::vector<cv::Point> > ContourVector;
  
  void findBlobs();
  bool getDepthData();

  void enableRecordIfNeeded();
  
  void threadFunc();
};
      
#endif