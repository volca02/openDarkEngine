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


#include "DrawCommon.h"
#include <OgreVector2.h>

using Ogre::Vector2;

namespace Opde {
	//------------------------------------------------------
	DrawSourceBase::DrawSourceBase(const Ogre::MaterialPtr& mat, const Ogre::TexturePtr& tex) : mMaterial(mat), mTexture(tex) {
		mPixelSize.width = tex->getWidth();
		mPixelSize.height = tex->getHeight();
	};
	
	//------------------------------------------------------	
	DrawSourceBase::DrawSourceBase() : mMaterial(), mTexture() {
		mPixelSize.width = 0;
		mPixelSize.height = 0;
	};

	//------------------------------------------------------	
	bool QuadLess::operator()(const DrawQuad* a, const DrawQuad* b) const {
		return a->positions.topleft.z < b->positions.topleft.z;
	};

	//------------------------------------------------------	
	DrawSource::DrawSource(ID id, const Ogre::MaterialPtr& mat, const Ogre::TexturePtr& tex) : DrawSourceBase(mat, tex), mAtlassed(false) {
		setSourceID(id);
	}

	//------------------------------------------------------
	DrawSource::DrawSource() : mAtlassed(false) {
		mMaterial.setNull();
		mTexture.setNull();
		mPixelSize.width  = 0; // needs to be filled on loadimage
		mPixelSize.height = 0;
		mSize = Ogre::Vector2(1.0f, 1.0f);
		mDisplacement = Ogre::Vector2(0, 0);
	}
	
	//------------------------------------------------------	
	Vector2 DrawSource::transform(const Vector2& input) {
		Vector2 tran(input);

		tran *= mSize;
		tran += mDisplacement;

		return tran;
	};
		
	//------------------------------------------------------
	void DrawSource::atlas(const Ogre::MaterialPtr& mat, size_t x, size_t y, size_t width, size_t height) {
		mMaterial = mat;
		mDisplacement = Ogre::Vector2((Ogre::Real)x / width, (Ogre::Real)y / height);
		mSize = Ogre::Vector2((Ogre::Real)mPixelSize.width / width, (Ogre::Real)mPixelSize.height / height);
		mAtlassed = true;
	}

	//------------------------------------------------------	
	void DrawSource::updatePixelSizeFromImage() {
		mPixelSize.width  = mImage.getWidth();
		mPixelSize.height = mImage.getHeight();
	}
	
	//------------------------------------------------------
	void DrawSource::fillTexCoords(DrawRect<Ogre::Vector2>& tc) {
		tc.topleft     = mDisplacement;
		tc.topright    = mDisplacement + Vector2(mSize.x, 0);
		tc.bottomleft  = mDisplacement + Vector2(0, mSize.y);
		tc.bottomright = mDisplacement + Vector2(mSize.x, mSize.y);
	}
	
	//------------------------------------------------------
	void DrawSource::loadImage(const Ogre::String& name, const Ogre::String& group) {
		mImage.load(name, group);
		updatePixelSizeFromImage();
	}
};
