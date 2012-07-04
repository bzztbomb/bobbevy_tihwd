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
	mEnabled = false;
	console() << "There are " << Kinect::getNumDevices() << " Kinects connected." << std::endl;	
	if (Kinect::getNumDevices() == 0)
		return;
	mEnabled = true;
	mKinect = Kinect( Kinect::Device() ); // the default Device implies the first Kinect connected	
	
	
    mStepSize = 255; // Just threshold from step from!
    mBlurAmount = 20;
	mStepFrom = 5;
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
	if (!mEnabled)
		return;

	switch( event.getChar() ){
		case ' ':
			mInitInitial = true;
			break;
		default:
			break;
	}	
}

void KinectWrapper::update()
{
	if (!mEnabled)
		return;

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
		i->mLeftMost.x = 10000;
		i->mRightMost.x = -10000;
		i->mTopMost.y = 10000;
		i->mBottomMost.y = -10000;
		for (vector<cv::Point>::iterator pt = i->mContourPoints.begin(); pt != i->mContourPoints.end(); ++pt)
		{			
			i->mCentroid.x += pt->x;
			i->mCentroid.y += pt->y;			
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
		i->mCentroid.x *= (float) getWindowWidth() / 640.0f;
		i->mCentroid.y *= (float) getWindowHeight() / 480.0f;
		// Loop through and do z calc
		float steps = 10.0f;
		Vec3f step = (i->mBottomMost - i->mTopMost) / steps;
		Vec3f sample = i->mTopMost;
		for (int x = 0; x < steps; x++)
		{
			i->mZDist = max(i->mZDist, (float) *to8.getDataRed(Vec2f(sample.x, sample.y)));
			sample += step;
		}
		step = (i->mRightMost - i->mLeftMost) / steps;
		sample = i->mLeftMost;
		for (int x = 0; x < steps; x++)
		{
			i->mZDist = max(i->mZDist, (float) *to8.getDataRed(Vec2f(sample.x, sample.y)));
			sample += step;
		}		
	}
	sort(mBlobs.begin(), mBlobs.end(), SortDescendingZ());
}

void KinectWrapper::draw()
{
	if (!mEnabled)
		return;

	gl::color(Color(1.0f, 1.0f, 1.0f));
	gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
//	if( mContourTexture )
//		gl::draw( mContourTexture, getWindowBounds() );
	if( mDepthTexture )
		gl::draw( mDepthTexture, getWindowBounds() );
	
	if (true)
	{
		float xs = (float) getWindowWidth() / 640.0f;
		float ys = (float) getWindowHeight() / 480.0f;				

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
				
				gl::vertex( Vec2f(pt->x * xs, pt->y * ys) );
			}	
			glEnd();
			gl::drawSolidCircle(i->mCentroid, 10);
			c++;			
		}				
	}
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
