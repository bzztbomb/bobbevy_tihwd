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

using namespace ci;
using namespace ci::app;
using namespace std;

int KinectWrapper::smMAX_BLOBS = 3;

KinectWrapper::KinectWrapper() :
    mFakeSurface(640, 480, false, SurfaceChannelOrder::RGB)
{

}

void KinectWrapper::setup(params::InterfaceGl& params)
{
	console() << "There are " << Kinect::getNumDevices() << " Kinects connected." << std::endl;	
	mKinectEnabled = (Kinect::getNumDevices() > 0);
    if (mKinectEnabled)
        mKinect = Kinect( Kinect::Device() ); // the default Device implies the first Kinect         
	mEnabled = true;
    mBlobsEnabled = true;
		
    mStepSize = 255; // Just threshold from step from!
    mBlurAmount = 3;
	mStepFrom = 5;
	mAreaThreshold = 1000.0f;
	mInitInitial = true;	
    mDrawContour = false;
    mLowPass = 255;
    mDilate = false;
    
	params.addSeparator("CV Params");
	params.addParam( "Step from", &mStepFrom, "min=1 max=255" );
	params.addParam( "Threshold Step Size", &mStepSize, "min=1 max=255" );
    params.addParam( "LowPass filter", &mLowPass, "min=0 max=255");
    params.addParam( "CV Blur amount", &mBlurAmount, "min=3 max=55" );	
	params.addParam( "CV area threshold", &mAreaThreshold, "min=1");
    params.addParam( "Show contour", &mDrawContour);
    params.addParam( "Dilate", &mDilate);
    params.addParam( "KinectEnabled", &mEnabled);
    params.addParam( "BlobsEnabled", &mBlobsEnabled);
}

void KinectWrapper::keyDown( KeyEvent event )
{
	if (!mEnabled)
		return;

	switch( event.getCode() ){
		case KeyEvent::KEY_a:
			mInitInitial = true;
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

	findBlobs();
}

bool KinectWrapper::getDepthData()
{
    if (mKinectEnabled)
    {
        bool newDepth = false;
        if( mKinect.checkNewDepthFrame() )
        {
            newDepth = true;
            mDepthTexture = mKinect.getDepthImage();
        }
        
        if( mKinect.checkNewVideoFrame() )
            mColorTexture = mKinect.getVideoImage();
        return newDepth;
    } else {        
        if (!mFakeDataAvail)
            return false;
        mFakeDataAvail = false;
        
        memset(mFakeSurface.getData(), 0, mFakeSurface.getRowBytes() * mFakeSurface.getHeight());

        for (int index = 0; index < 4; index+=2)
        {            
            Vec2f minV(min(mFakeBlobs[index].x, mFakeBlobs[index+1].x), 
                       min(mFakeBlobs[index].y, mFakeBlobs[index+1].y));
            Vec2f maxV(max(mFakeBlobs[index].x, mFakeBlobs[index+1].x), 
                       max(mFakeBlobs[index].y, mFakeBlobs[index+1].y));       
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
    if (mDilate)
        cv::dilate(gray, gray, cv::Mat());
    if (mBlurAmount > 3)
        cv::blur( gray, gray, cv::Size( mBlurAmount, mBlurAmount ) );
    else
        cv::medianBlur(gray, gray, (mBlurAmount%2==0)?mBlurAmount+1:mBlurAmount);
    
    if (mInitInitial)
	{
		mInitial = gray.clone();
		mInitInitial = false;
	}		
	gray -= mInitial;
		
	mBlobs.clear();
	float largest = mAreaThreshold;
	for( int t = mStepFrom; t < 255; t += mStepSize )
	{
		ContourVector vec;
        if (mLowPass != 255)
        {
            cv::threshold( gray, thresh, mLowPass, mLowPass, CV_THRESH_TOZERO_INV );	
            cv::threshold( thresh, thresh, t, 255, CV_THRESH_BINARY );
        } else {
            cv::threshold( gray, thresh, t, 255, CV_THRESH_BINARY );
        }
        //if (mDrawContour)
        {
            mContourMat = thresh.clone();
            mContourTexture = fromOcv(mContourMat);
        }
        
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

    float xs = (float) getWindowWidth() / 640.0f;
    float ys = (float) getWindowHeight() / 480.0f;					
	
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
            i->mBounds.include(Vec2f(pt->x * xs, pt->y * ys));
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

    glDisable(GL_TEXTURE_2D);
	gl::color(Color(1.0f, 1.0f, 1.0f));
	gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
    if (mDrawContour)
    {
        if( mContourTexture )
            gl::draw( mContourTexture, getWindowBounds() );
    } else {
        if( mDepthTexture )
            gl::draw( mDepthTexture, getWindowBounds() );
    }
	
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
            gl::drawStrokedRect(i->mBounds);
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
