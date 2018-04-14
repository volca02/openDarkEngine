/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2009 openDarkEngine team
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *	  $Id$
 *
 *****************************************************************************/

#include "OpdeException.h"
#include "DrawCommon.h"
#include "DrawService.h"
#include <OgreVector2.h>

using Ogre::Vector2;

namespace Opde {
//------------------------------------------------------
DrawSourceBase::DrawSourceBase(ID srcID, const Ogre::MaterialPtr& mat, const Ogre::TexturePtr& tex) : mMaterial(mat), mTexture(tex), mSourceID(srcID) {
    mPixelSize.width = tex->getWidth();
    mPixelSize.height = tex->getHeight();
};

//------------------------------------------------------
DrawSourceBase::DrawSourceBase() : mMaterial(), mTexture(), mSourceID(0) {
    mMaterial.reset();
    mTexture.reset();

    mPixelSize.width = 0;
    mPixelSize.height = 0;
};

//------------------------------------------------------
bool QuadLess::operator()(const DrawQuad* a, const DrawQuad* b) const {
    return a->depth < b->depth;
};

//------------------------------------------------------
DrawSource::DrawSource(DrawService *owner) :
    DrawSourceBase(),
    mAtlassed(false),
    mImageLoaded(false),
    mOwner(owner),
    mPixmap(NULL)
{
    mImage = new Ogre::Image();
}

//------------------------------------------------------
DrawSource::DrawSource(DrawService *owner, ID id, const Ogre::MaterialPtr& mat, const Ogre::TexturePtr& tex) :
    DrawSourceBase(id, mat, tex),
    mAtlassed(false),
    mImageLoaded(false),
    mOwner(owner),
    mPixmap(NULL) {
    mImage = new Ogre::Image();
}

//------------------------------------------------------
DrawSource::DrawSource(DrawService *owner, const Ogre::String& name, const Ogre::String& group, const Ogre::MaterialPtr& extMaterial) :
    DrawSourceBase(),
    mAtlassed(false),
    mImageLoaded(false),
    mOwner(owner),
    mPixmap(NULL) {

    mImage = new Ogre::Image();

    mPixelSize.width  = 0; // needs to be filled on loadimage
    mPixelSize.height = 0;
    mSize = Ogre::Vector2(1.0f, 1.0f);
    mDisplacement = Ogre::Vector2(0, 0);

    loadImage(name, group);

    // if there was an external material specified then assign now
    if (extMaterial)
        mMaterial = extMaterial;
}

//------------------------------------------------------
DrawSource::~DrawSource() {
    // TODO: mOwner->unregisterDrawSourceByPtr(this);
    delete mImage;
    delete[] mPixmap;
}

//------------------------------------------------------
Vector2 DrawSource::transform(const Vector2& input) {
    Vector2 tran(input);

    tran *= mSize;
    tran += mDisplacement;

    return tran;
};

//------------------------------------------------------
Ogre::Real DrawSource::transformX(Ogre::Real x) {
    return x*mSize.x + mDisplacement.x;
}

//------------------------------------------------------
Ogre::Real DrawSource::transformY(Ogre::Real y) {
    return y*mSize.y + mDisplacement.y;
}

//------------------------------------------------------
void DrawSource::atlas(const Ogre::MaterialPtr& mat, size_t x, size_t y, size_t width, size_t height) {
    mMaterial = mat;
    mDisplacement = Ogre::Vector2((Ogre::Real)x / width, (Ogre::Real)y / height);
    mSize = Ogre::Vector2((Ogre::Real)mPixelSize.width / width, (Ogre::Real)mPixelSize.height / height);
    mAtlassed = true;
}

//------------------------------------------------------
void DrawSource::updatePixelSizeFromImage() {
    mPixelSize.width  = mImage->getWidth();
    mPixelSize.height = mImage->getHeight();
}

//------------------------------------------------------
void DrawSource::fillTexCoords(DrawRect<Ogre::Real>& tc) {
    tc.left     = mDisplacement.x;
    tc.right    = mDisplacement.x + mSize.x;
    tc.top      = mDisplacement.y;
    tc.bottom   = mDisplacement.y + mSize.y;
}

//------------------------------------------------------
void DrawSource::loadImage(const Ogre::String& name, const Ogre::String& group) {
    // If the image is loaded already, we have an error
    if (mImageLoaded)
        OPDE_EXCEPT("Image already loaded in this DrawSource", "DrawSource::loadImage");

    mImage->load(name, group);
    updatePixelSizeFromImage();

    mImageLoaded = true;
}

//------------------------------------------------------
bool operator<(const DrawSheetPtr& a, const DrawSheetPtr& b) {
    return a.get() < b.get();
}
};
