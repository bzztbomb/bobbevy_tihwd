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
	cinder::Kinect	mKinect;
	cinder::gl::Texture		mColorTexture, mDepthTexture;
	
    typedef std::vector< std::vector<cv::Point> > ContourVector;
    ContourVector mContours;
	std::vector<float> mContourAreas;
	std::vector<cv::Point> mLargest;
	
    int mStepSize;
    int mBlurAmount;
	cv::Mat mInitial;
	bool mInitInitial;
	
	cinder::gl::Texture	mContourTexture;
	cv::Mat mContourMat;
	
	void setup(cinder::params::InterfaceGl& params);
	void update();
	void keyDown( cinder::app::KeyEvent event );
	void draw();
};

#endif