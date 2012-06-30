#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Surface.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/params/Params.h"

#include "CinderOpenCV.h"

#include "Kinect.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class KinectWrapper
{
public:
	Kinect			mKinect;
	gl::Texture		mColorTexture, mDepthTexture;

    typedef vector< vector<cv::Point> > ContourVector;
    ContourVector mContours;
	vector<float> mContourAreas;
	vector<cv::Point> mLargest;
	
    int mStepSize;
    int mBlurAmount;
	cv::Mat mInitial;
	bool mInitInitial;
	
	gl::Texture	mContourTexture;
	cv::Mat mContourMat;
		
	void setup(params::InterfaceGl& params);
	void update();
	void keyDown( KeyEvent event );
	void draw();
};

class bobbevyApp : public AppBasic {
public:
	void prepareSettings( Settings* settings );
	void setup();
	void keyDown( KeyEvent event );
	void mouseDown( MouseEvent event );	
	void update();
	void draw();	
private:
	KinectWrapper mKinect;
	params::InterfaceGl mParams;
};

void bobbevyApp::prepareSettings( Settings* settings )
{
	settings->setWindowSize( 640, 480 );
}

void bobbevyApp::setup()
{
	mParams = params::InterfaceGl("bobbevy", Vec2i(225, 200));	
	mKinect.setup(mParams);
}

void bobbevyApp::keyDown( KeyEvent event )
{
	mKinect.keyDown(event);
}

void bobbevyApp::mouseDown( MouseEvent event )
{
}

void bobbevyApp::update()
{
	mKinect.update();
}

void bobbevyApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
	mKinect.draw();
	
	// draw interface
	params::InterfaceGl::draw();	
}


void KinectWrapper::setup(params::InterfaceGl& params)
{
	console() << "There are " << Kinect::getNumDevices() << " Kinects connected." << std::endl;	
    mStepSize = 50;
    mBlurAmount = 10;
	mInitInitial = true;	
	mKinect = Kinect( Kinect::Device() ); // the default Device implies the first Kinect connected	
    
	params.addSeparator("CV Params");
	params.addParam( "Threshold Step Size", &mStepSize, "min=1 max=255" );
    params.addParam( "CV Blur amount", &mBlurAmount, "min=3 max=55" );	
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

	mContourMat = gray.clone();
	mContourTexture = fromOcv(mContourMat);
	mContours.clear();
	mContourAreas.clear();
	float largest = -1.0f;
	for( int t = 2; t < 255; t += mStepSize )
	{
		ContourVector vec;
		cv::threshold( gray, thresh, t, 255, CV_THRESH_BINARY );
//		cv::adaptiveThreshold(gray, thresh, 255, cv::ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 3, t);
		cv::findContours( thresh, vec, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );
		// put into mContours
//		mContours.insert( mContours.end(), vec.begin(), vec.end() );

		for( ContourVector::iterator iter = vec.begin(); iter != vec.end(); ++iter )
		{		
			float a = cv::contourArea(*iter);
			if (a > largest)
			{
				mLargest.clear();
				mLargest.resize(iter->size());
				copy(iter->begin(), iter->end(), mLargest.begin());
				largest = a;
			}
		}
	}
}

void KinectWrapper::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
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
#if 0
		for( ContourVector::iterator iter = mContours.begin(); iter != mContours.end(); ++iter )
		{
			glBegin( GL_LINE_LOOP );
			
			for( vector<cv::Point>::iterator pt = iter->begin(); pt != iter->end(); ++pt )
			{
				gl::color( Color( 1.0f, 1.0f, 1.0f ) );
				gl::vertex( fromOcv( *pt ) );
			}
			
			glEnd();
		}
#endif		
		glBegin(GL_LINE_LOOP);
		for( vector<cv::Point>::iterator pt = mLargest.begin(); pt != mLargest.end(); ++pt )
		{
			gl::color( Color( 1.0f, 0.0f, 0.0f ) );
			gl::vertex( fromOcv( *pt ) );
		}	
		glEnd();
		
		
		gl::popMatrices();	
	}
}


CINDER_APP_BASIC( bobbevyApp, RendererGl )
