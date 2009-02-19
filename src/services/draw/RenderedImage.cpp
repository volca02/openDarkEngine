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

#include "RenderedImage.h"
#include "DrawService.h"
#include "DrawBuffer.h"


using namespace Ogre;

namespace Opde {

	/*----------------------------------------------------*/
	/*-------------------- RenderedImage -----------------*/
	/*----------------------------------------------------*/
	RenderedImage::RenderedImage(DrawService* owner, DrawOperation::ID id, DrawSource* ds) :
			DrawOperation(owner, id), mDrawSource(ds) {
		mDrawQuad.texCoords.topleft = ds->transform(Vector2(0,0));
		mDrawQuad.texCoords.topright = ds->transform(Vector2(1.0f,0));
		mDrawQuad.texCoords.bottomleft = ds->transform(Vector2(0,1.0f));
		mDrawQuad.texCoords.bottomright = ds->transform(Vector2(1.0f,1.0f));

		mDrawQuad.colors.topleft = ColourValue(1.0f, 1.0f, 1.0f);
		mDrawQuad.colors.topright = ColourValue(1.0f, 1.0f, 1.0f);
		mDrawQuad.colors.bottomleft = ColourValue(1.0f, 1.0f, 1.0f);
		mDrawQuad.colors.bottomright = ColourValue(1.0f, 1.0f, 1.0f);
		
		mInClip = true;

		// to be sure we are up-to-date
		_markDirty();
	}

	//------------------------------------------------------
	void RenderedImage::visitDrawBuffer(DrawBuffer* db) {
		// are we in the clip area?
		if (mInClip)
			db->_queueDrawQuad(&mDrawQuad);
	}

	//------------------------------------------------------
	DrawSourceBase* RenderedImage::getDrawSourceBase() {
		return mDrawSource;
	}
	
	//------------------------------------------------------
	void RenderedImage::_rebuild() {
		const PixelSize& ps = mDrawSource->getPixelSize();
		mDrawQuad.positions.topleft     = mOwner->convertToScreenSpace(mPosition.first, mPosition.second, mZOrder);
		mDrawQuad.positions.topright    = mOwner->convertToScreenSpace(mPosition.first + ps.width, mPosition.second, mZOrder);
		mDrawQuad.positions.bottomleft  = mOwner->convertToScreenSpace(mPosition.first, mPosition.second + ps.height, mZOrder);
		mDrawQuad.positions.bottomright = mOwner->convertToScreenSpace(mPosition.first + ps.width, mPosition.second + ps.height, mZOrder);

		mInClip = mClipRect.clip(mDrawQuad);
		_markDirty();
	}

}
