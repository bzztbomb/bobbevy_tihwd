//
//  fullscreen_pass.h
//  gawd_rays
//
//  Created by Brian Richardson on 1/17/13.
//
//

#ifndef gawd_rays_fullscreen_pass_h
#define gawd_rays_fullscreen_pass_h

#pragma once

// Grabbed from num3ric

#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include <map>
#include <boost/optional/optional.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/static_visitor.hpp>


using namespace ci;

namespace shader {
  
  namespace {
    class bind_visitor : public boost::static_visitor<> {
      int mUnit;
    public:
      bind_visitor(int unit) : mUnit(unit) { };
      void operator()(gl::Fbo& fbo) const {
        fbo.bindTexture(mUnit);
      }
      void operator()(gl::Texture& tex) const {
        glEnable(tex.getTarget());
        tex.bind(mUnit);
      }
    };
    
    class unbind_visitor : public boost::static_visitor<> {
      int mUnit;
    public:
      unbind_visitor(int unit) : mUnit(unit) { };
      void operator()(gl::Fbo& fbo) const {
        fbo.unbindTexture();
      }
      void operator()(gl::Texture& tex) const {
        tex.unbind(mUnit);
        tex.disable();
      }
    };
  } //anonymous 'private' namespace
  
  
  void fullscreenQuadPass(std::map<int, boost::variant<gl::Fbo&, gl::Texture&>> textureFboUnits,
                          boost::optional<gl::Fbo&> renderTarget = boost::optional<gl::Fbo&>())
  {
    if (textureFboUnits.empty())
      return;
    if (renderTarget) {
      renderTarget->bindFramebuffer();
    }
    for ( auto& entry : textureFboUnits ) {
      boost::apply_visitor(bind_visitor(entry.first), entry.second );
    }
//    gl::clear();
    gl::pushMatrices();
    if (renderTarget) {
      gl::setMatricesWindow(renderTarget->getSize(), false); //false: vertical flip
      gl::drawSolidRect( renderTarget->getBounds() );
    } else {
      gl::setMatricesWindow(app::getWindowSize(), false); //false: vertical flip
      gl::drawSolidRect(app::getWindowBounds());
    }
    gl::popMatrices();
    
    for ( auto& entry : textureFboUnits ) {
      boost::apply_visitor(unbind_visitor(entry.first), entry.second );
    }
    if (renderTarget) {
      renderTarget->unbindFramebuffer();
    }
  }
}

#endif
