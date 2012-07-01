/*
 *  bbKinectWrapper.cpp
 *  bobbevy
 *
 *  Created by Brian Richardson on 6/30/12.
 *  Copyright 2012 Knowhere Studios Inc. All rights reserved.
 *
 */

#include "bbKinectWrapper.h"

using namespace ci;
using namespace ci::app;
using namespace std;

int KinectWrapper::smMAX_BLOBS = 3;

void KinectWrapper::setup(params::InterfaceGl& params)
{
	console() << "There are " << Kinect::getNumDevices() << " Kinects connected." << std::endl;	
	mKinect = Kinect( Kinect::Device() ); // the default Device implies the first Kinect connected	

    mStepSize = 255; // Just threshold from step from!
    mBlurAmount = 20;
	mStepFrom = 2;
	mAreaThreshold = 1000.0f;
	mInitInitial = true;	
    
	params.addSeparator("CV Params");
	params.addParam( "Step from", &mStepFrom, "min=1 max=255" );
	params.addParam( "Threshold Step Size", &mStepSize, "min=1 max=255" );
    params.addParam( "CV Blur amount", &mBlurAmount, "min=3 max=55" );	
	params.addParam( "CV area threshold", &mAreaThreshold, "min=1");
}

void KinectWrapper::keyDown( KeyEvent event )
{
	switch( event.getChar() ){
		case 'm':
			mInitInitial = true;
			break;
		default:
			break;
	}	
}

void KinectWrapper::update()
{
	findBlobs();
}

void KinectWrapper::findBlobs()
{
	if( mKinect.checkNewDepthFrame() )
		mDepthTexture = mKinect.getDepthImage();
	
	if( mKinect.checkNewVideoFrame() )
		mColorTexture = mKinect.getVideoImage();
	
	if ((!mDepthTexture) || (!mColorTexture))
		return;
	
	Surface8u to8 = Surface8u(mKinect.getDepthImage());
	cv::Mat input( toOcv( to8)); 
	
	cv::Mat gray;
	cv::Mat thresh;
	
	cv::cvtColor( input, gray, CV_RGB2GRAY );
	cv::dilate(gray, gray, cv::Mat());
	cv::blur( gray, gray, cv::Size( mBlurAmount, mBlurAmount ) );
	
	if (mInitInitial)
	{
		mInitial = gray.clone();
		mInitInitial = false;
	}		
	gray -= mInitial;
	
	// Debug texture
	mContourMat = gray.clone();
	mContourTexture = fromOcv(mContourMat);
	
	mBlobs.clear();
	float largest = mAreaThreshold;
	for( int t = mStepFrom; t < 255; t += mStepSize )
	{
		ContourVector vec;
		cv::threshold( gray, thresh, t, 255, CV_THRESH_BINARY );
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
					mBlobs.erase(mBlobs.end());
					largest = mBlobs.rbegin()->mContourArea;
				}
			}
		}
	}
	
	for (BlobVector::iterator i = mBlobs.begin(); i != mBlobs.end(); i++)
	{
		i->mCentroid.x = i->mCentroid.y = 0.0f;
		for (vector<cv::Point>::iterator pt = i->mContourPoints.begin(); pt != i->mContourPoints.end(); ++pt)
		{
			i->mCentroid.x += pt->x;
			i->mCentroid.y += pt->y;			
		}
		float sz = i->mContourPoints.size();
		if (sz > 0.0f)
		{
			i->mCentroid.x /= sz;
			i->mCentroid.y /= sz;		
		}
		i->mZDist = *to8.getDataRed(fromOcv(i->mCentroid));
	}
	sort(mBlobs.begin(), mBlobs.end(), SortDescendingZ());
}

void KinectWrapper::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::color(Color(1.0f, 1.0f, 1.0f));
	gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
	if( mContourTexture )
		gl::draw( mContourTexture );
	if( mDepthTexture )
		gl::draw( mDepthTexture, Vec2i( 640, 0 ) );
	
	if (true)
	{
		gl::pushMatrices();
		gl::translate( Vec2f( getWindowWidth() - 640, getWindowHeight() - 480 ) * 0.5f );
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
				gl::vertex( fromOcv( *pt ) );
			}	
			glEnd();
			gl::drawSolidCircle(fromOcv(i->mCentroid), 10);
			c++;			
		}				
		gl::popMatrices();	
	}
}
