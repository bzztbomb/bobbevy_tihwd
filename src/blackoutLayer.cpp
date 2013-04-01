//
//  blackoutLayer.cpp
//  bobbevy
//
//  Created by Brian Richardson on 3/31/13.
//
//

#include "blackoutLayer.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

BlackoutLayer::BlackoutLayer() :
  SceneLayer("BlackoutLayer"),
  mBlackoutAmount(0.0f),
  mBlackoutColor(0.0f, 0.0f, 0.0f, 1.0f)
{
}

// SceneLayer
void BlackoutLayer::draw()
{
  if (mBlackoutAmount > 0.0)
	{
    gl::enableAlphaBlending();
    ColorA b = mBlackoutColor;
    b.a = mBlackoutAmount;
		gl::color( b );
    gl::drawSolidRect(getWindowBounds());
		gl::color( cinder::ColorA(1, 1, 1, 1) );
    gl::disableAlphaBlending();
	}
}

// QTimelineModule
void BlackoutLayer::init()
{
  registerParam("amount", &mBlackoutAmount);
  registerParam("red", &mBlackoutColor.r);
  registerParam("green", &mBlackoutColor.g);
  registerParam("blue", &mBlackoutColor.b);
}