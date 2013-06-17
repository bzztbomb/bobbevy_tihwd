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

class DepthSource
{
public:
  virtual ~DepthSource() { };
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

class DepthProcessor
{
public:
  static cinder::Vec2i smSize;
public:
  DepthProcessor();
  
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
  void setEnabled(bool val) { mEnabled = val; }
  bool getEnabled() const { return mEnabled; }
  
  Blob* getUser(UserToken which);
  std::vector<Blob> getUsers();
  void updateFakeBlob(int index, const cinder::Vec2f& pos);
  
  cv::Mat* getContourMat() { return &mContourMat; }
  cinder::gl::Texture getContourTexture() { return mContourTexture; }
protected:
  DepthSourceRef mDepthSource;
  DepthSourceRef mKinectDepthSource;
  
	// Kinect interface
  bool mEnabled;
	cinder::gl::Texture		mColorTexture, mDepthTexture;
  
  // Lack of Kinect interface
  cinder::Vec2f mFakeBlobs[NUM_FAKE_BLOB_PTS];
  bool mFakeDataAvail;
  cinder::Surface8u mFakeSurface;
  
  
	cv::Mat mInitial;
  cv::Mat mLastGray; // Accumlated grayscal image
	int mInitInitial;
  int mInitFrames;

	// Image processing
	int mStepFrom;
  int mDepthLowPass;

  //
  bool mBlobsEnabled;
  
	// Debug
	cinder::gl::Texture	mContourTexture;
	cv::Mat mContourMat;
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
  BlobVector mBlobs;
  float mAreaThreshold;
  typedef std::vector< std::vector<cv::Point> > ContourVector;
  
  void findBlobs();
  bool getDepthData();
private:
  void enableRecordIfNeeded();
};
      
#endif