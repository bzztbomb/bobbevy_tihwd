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

class KinectWrapper
{
public:
	void setup(cinder::params::InterfaceGl& params);
	void update();
	void keyDown( cinder::app::KeyEvent event );
	void draw();
public:
	// Kinect interface
	cinder::Kinect	mKinect;
	cinder::gl::Texture		mColorTexture, mDepthTexture;

	// Image processing
	int mStepFrom;
    int mStepSize;
    int mBlurAmount;
	cv::Mat mInitial;
	bool mInitInitial;	

	// Debug
	cinder::gl::Texture	mContourTexture;
	cv::Mat mContourMat;
	
	// Blob detection and "user tracking"
	static int smMAX_BLOBS;	
	struct Blob {
		float mContourArea;
		std::vector<cv::Point> mContourPoints;
		cv::Point mCentroid;
	};
	struct SortDescendingArea
	{
		bool operator()(const Blob& t1, const Blob& t2) const
		{ 
			return t1.mContourArea < t2.mContourArea;
		}
	};
	typedef std::vector<Blob> BlobVector;
	BlobVector mBlobs;
	float mAreaThreshold;
	typedef std::vector< std::vector<cv::Point> > ContourVector;
	
	void findBlobs();
	void processBlobs();
};

#endif