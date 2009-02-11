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
#include "FontDrawSource.h"

namespace Opde {
	/*----------------------------------------------------*/
	/*------------------- FontDrawSource -----------------*/
	/*----------------------------------------------------*/
	FontDrawSource::FontDrawSource(TextureAtlas* container, const std::string& name) : mContainer(container), mName(name), mMaxHeight(0), mMaxWidth(0) {
		//
	}

	//------------------------------------------------------
	void FontDrawSource::addGlyph(FontCharType chr, const PixelSize& dimensions, Ogre::PixelFormat pf, void* data, RGBQuad* pallette) {
		// Register the new glyph.
		if (dimensions.height > mMaxHeight)
			mMaxHeight = dimensions.height;

		if (dimensions.width > mMaxWidth)
			mMaxWidth = dimensions.width;

		DrawSourcePtr ds = new DrawSource();

		// TODO: Fill the image, dimensions, atlas ref. etc.

		mRepresentedGlyphs.insert(std::make_pair(chr, ds));
	}

	//------------------------------------------------------
	DrawSourcePtr FontDrawSource::getGlyph(FontCharType chr) {
		GlyphMap::iterator it = mRepresentedGlyphs.find(chr);

		if (it!=mRepresentedGlyphs.end())
			return it->second;

		return NULL;
	}

	//------------------------------------------------------
	void FontDrawSource::bake() {
		GlyphMap::iterator it = mRepresentedGlyphs.begin();
		while (it != mRepresentedGlyphs.end()) {
			DrawSourcePtr& ds = (it++)->second;

			// atlas
			mContainer->_addDrawSource(ds);
		}
	}

};
