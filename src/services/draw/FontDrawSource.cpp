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
#include "OpdeException.h"
#include "TextureAtlas.h"

namespace Opde {
	/*----------------------------------------------------*/
	/*------------------- FontDrawSource -----------------*/
	/*----------------------------------------------------*/
	FontDrawSource::FontDrawSource(TextureAtlas* container, const std::string& name) : mContainer(container), mName(name), mMaxHeight(0), mMaxWidth(0), mBuilt(false) {
		//
	}

	//------------------------------------------------------
	void FontDrawSource::addGlyph(FontCharType chr, const PixelSize& dimensions, DarkPixelFormat pf, size_t rowlen, void* data, const RGBAQuad* pallette) {
		assert(!mBuilt);

		// Register the new glyph.
		if (dimensions.height > mMaxHeight)
			mMaxHeight = dimensions.height;

		if (dimensions.width > mMaxWidth)
			mMaxWidth = dimensions.width;

		DrawSourcePtr ds = new DrawSource();

		// TODO: Fill the image, dimensions, atlas ref. etc.
		switch (pf) {
			case DPF_MONO : populateImageFromMono(ds, dimensions, rowlen, data, pallette); break;
			case DPF_8BIT : populateImageFrom8BitPal(ds, dimensions, rowlen, data, pallette); break;
			default:
				OPDE_EXCEPT("Invalid pixel format supplied", "FontDrawSource::addGlyph");
		}

		// copy the dimensions
		ds->pixelSize = dimensions;
		ds->size = Ogre::Vector2(1.0f, 1.0f);
		ds->displacement = Ogre::Vector2(0.0f, 0.0f);
		ds->material.setNull();
		ds->texture.setNull();

		mRepresentedGlyphs.insert(std::make_pair(chr, ds));
	}

	//------------------------------------------------------
	DrawSourcePtr FontDrawSource::getGlyph(FontCharType chr) {
		assert(mBuilt);

		GlyphMap::iterator it = mRepresentedGlyphs.find(chr);

		if (it!=mRepresentedGlyphs.end())
			return it->second;

		return NULL;
	}

	//------------------------------------------------------
	void FontDrawSource::build() {
		assert(!mBuilt);

		GlyphMap::iterator it = mRepresentedGlyphs.begin();
		while (it != mRepresentedGlyphs.end()) {
			DrawSourcePtr& ds = (it++)->second;

			// atlas
			mContainer->_addDrawSource(ds);
		}

		mBuilt = true;
	}

	//------------------------------------------------------
	void FontDrawSource::populateImageFromMono(DrawSourcePtr& dsp, const PixelSize& dimensions, size_t rowlen, void* data, const RGBAQuad* pal) {
		// We'll use the first two records in the pal for conversion.
		// first we take the number of the bits to process
		size_t cr = 0;
		size_t pixelcount = dimensions.getPixelCount();

		// Target data - 32 bit unsigned per pixel
		uint32_t* pixels = new uint32_t[pixelcount];
		// current pixel pointer on the destination buffer
		uint32_t* pixpos = pixels;

		// current pixel pointer on the source buffer
		unsigned char* srcdta = reinterpret_cast<unsigned char*>(data);

		// we have the row length to skip each row...
		for (cr = 0; cr < dimensions.height; ++cr) {
			for (size_t x = 0; x < dimensions.width; ++x, ++pixpos) {
				size_t idx = (srcdta[x >> 3]>>(7 - x & 0x07) & 1);

				*pixpos = pal[idx].blue | (pal[idx].green << 8) | (pal[idx].red << 16) | (pal[idx].alpha << 24);
			}

			srcdta += rowlen;
		}

		// we have the image data ready, now we'll populate image with it
		dsp->image.loadDynamicImage(reinterpret_cast<Ogre::uchar*>(pixels), dimensions.width,dimensions.height, 1, Ogre::PF_A8R8G8B8, false);

		delete[] pixels;
	}

	//------------------------------------------------------
	void FontDrawSource::populateImageFrom8BitPal(DrawSourcePtr& dsp, const PixelSize& dimensions, size_t rowlen, void* data, const RGBAQuad* pal) {
		// We'll use the first two records in the pal for conversion.
		// first we take the number of the bits to process
		size_t cr = 0;
		size_t pixelcount = dimensions.getPixelCount();

		// Target data - 32 bit unsigned per pixel
		uint32_t* pixels = new uint32_t[pixelcount];
		// current pixel pointer on the destination buffer
		uint32_t* pixpos = pixels;

		// current pixel pointer on the source buffer
		unsigned char* srcdta = reinterpret_cast<unsigned char*>(data);

		// we have the row length to skip each row...
		for (cr = 0; cr < dimensions.height; ++cr) {
			for (size_t x = 0; x < dimensions.width; ++x, ++pixpos) {
				size_t idx = srcdta[x];

				*pixpos = pal[idx].blue | (pal[idx].green << 8) | (pal[idx].red << 16) | (pal[idx].alpha << 24);
			}

			srcdta += rowlen;
		}

		// we have the image data ready, now we'll populate image with it
		dsp->image.loadDynamicImage(reinterpret_cast<Ogre::uchar*>(pixels), dimensions.width,dimensions.height, 1, Ogre::PF_A8R8G8B8, false);

		delete[] pixels;
	}

};
