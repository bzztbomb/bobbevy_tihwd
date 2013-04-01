//
//  blackoutLayer.h
//  bobbevy
//
//  Created by Brian Richardson on 3/31/13.
//
//

#ifndef __bobbevy__blackoutLayer__
#define __bobbevy__blackoutLayer__

#include "sceneLayer.h"
#include "cinder/Color.h"

class BlackoutLayer : public SceneLayer
{
public:
	BlackoutLayer();
  
  // SceneLayer
  virtual void draw();
  
  // QTimelineModule
  virtual void init();
protected:
  float mBlackoutAmount;
  cinder::ColorA mBlackoutColor;
};

#endif /* defined(__bobbevy__blackoutLayer__) */
