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


#ifndef __FONTDRAWSOURCE_H
#define __FONTDRAWSOURCE_H

#include "DrawCommon.h"
#include "TextureAtlas.h"

#include <OgreImage.h>
#include <OgreTexture.h>
#include <OgreMaterial.h>

namespace Opde {


	/// A structure that holds a Font definition ready for usage (as numerous DrawSourcePtrs)
	class FontDrawSource {
		public:
			/** A font definition constructor
			* @param container The rendering atlas */
			FontDrawSource(TextureAtlas* container, const std::string& name);

			/** Adds a glyph definition
			* @param chr The character represented by this definition
			* @param dimensions the final dimensions of the glyph
			* @param pf The pixelformat of the source
			* @param data The linear buffer pointer to the pixel data in the specified format */
			void addGlyph(FontCharType chr, const PixelSize& dimensions, Ogre::PixelFormat pf, void* data, RGBQuad* pallette = NULL);

			/** Retrieves a glyph as a draw source for rendering usage */
			DrawSourcePtr getGlyph(FontCharType chr);

			/// Returns the maximal width of any glyph from this font
			inline size_t getWidth() { return mMaxWidth; };

			/// Reurns the maximal height of any glyph from this font
			inline size_t getHeight() { return mMaxHeight; };

			/** Bakes the font - this means that the font is complete and won't change any more, and informs us it's ready for atlassing. This method will queue all the glyphs in the owning atlas for atlassing. */
			void bake();

		protected:
			/// The map from character to the drawing source it represents
			typedef std::map<FontCharType, DrawSourcePtr> GlyphMap;

			/// Containing atlas
			TextureAtlas* mContainer;

			/// The map of the glyphs this font containts
			GlyphMap mRepresentedGlyphs;

			/// Name of this font
			std::string mName;

			/// Maximal height detected
			size_t mMaxHeight;

			/// Maximal width detected
			size_t mMaxWidth;
	};

};

#endif
