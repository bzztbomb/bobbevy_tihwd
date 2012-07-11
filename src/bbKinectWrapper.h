/*
 *  bbKinectWrapper.h
 *  bobbevy
 *
 *  Created by Brian Richardson on 6/30/12.
 *  Copyright 2012 Knowhere Studios Inc. All rights reserved.
 *
 */
#ifndef __BBKINECTWRAPPER_H_
#define __BBKINECTWRAPPER_H_

#include "Kinect.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "CinderOpenCV.h"
#include "cinder/params/Params.h"
#include "cinder/app/AppBasic.h"

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

class KinectWrapper
{
public:
    enum UserToken
    {
        utClosest,
        utFurthest
    };

	void setup(cinder::params::InterfaceGl& params);
	void update();
	void keyDown( cinder::app::KeyEvent event );
	void draw();

    Blob* getUser(UserToken which);
    std::vector<Blob> getUsers();
    
    cv::Mat* getContourMat() { return &mContourMat; }
public:
	// Kinect interface
	bool mEnabled;
	cinder::Kinect	mKinect;
	cinder::gl::Texture		mColorTexture, mDepthTexture;    

	// Image processing
	int mStepFrom;
    int mStepSize;
    int mBlurAmount;
	cv::Mat mInitial;
	bool mInitInitial;	
    int mLowPass;
    bool mDilate;
    bool mBlobsEnabled;

	// Debug
	cinder::gl::Texture	mContourTexture;
	cv::Mat mContourMat;
    bool mDrawContour;
	
	// Blob detection and "user tracking"
	static int smMAX_BLOBS;	
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
	typedef std::vector<Blob> BlobVector;
	BlobVector mBlobs;
	float mAreaThreshold;
	typedef std::vector< std::vector<cv::Point> > ContourVector;
	
	void findBlobs();
};

#endif