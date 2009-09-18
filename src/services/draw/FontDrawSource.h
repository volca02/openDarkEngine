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
#include "SharedPtr.h"
#include "TextureAtlas.h"

#include <OgreImage.h>
#include <OgreTexture.h>
#include <OgreMaterial.h>

namespace Opde {
	class TextureAtlas;

	/// A structure that holds a Font definition ready for usage (as numerous DrawSourcePtrs)
	class FontDrawSource {
		public:
			/** A font definition constructor
			* @param container The rendering atlas */
			FontDrawSource(const TextureAtlasPtr& container, const std::string& name);
			
			/// Destructor. Removes itself from the owner atlas
			~FontDrawSource();

			/** Adds a glyph definition
			* @param chr The character represented by this definition
			* @param dimensions the final dimensions of the glyph
			* @param pf The pixelformat of the source
			* @param rowlen the length of one row in bytes (e.g. the byte skip count to get to next row)
			* @param data The linear buffer pointer to the pixel data in the specified format 
			* @param pxoffset The offset in pixels for every scanline
			* @param palette The palette to use for color conversion */
			void addGlyph(FontCharType chr, const PixelSize& dimensions, DarkPixelFormat pf, size_t rowlen, void* data, size_t pxoffset, const RGBAQuad* palette = NULL);

			/** Retrieves a glyph as a draw source for rendering usage */
			DrawSourcePtr getGlyph(FontCharType chr);

			/// Returns the maximal width of any glyph from this font
			inline size_t getWidth() { return mMaxWidth; };

			/// Reurns the maximal height of any glyph from this font
			inline size_t getHeight() { return mMaxHeight; };

			/// Built indicator - After font build operation, this returns true. @see build()
			inline bool isBuilt() { return mBuilt; };

			/** Builds the font - this means that the font is complete and won't change any more, and informs us it's ready for atlassing. This method will queue all the glyphs in the owning atlas for atlassing. */
			void build();

			/// Returns the owning atlas of this font draw source 
			inline const TextureAtlasPtr& getAtlas() { return mContainer; };
			
			/** Calculates a width and height of the given text string.
			 *  The resulting size is of a unclipped, newline respecting text
			 */ 
			PixelSize calculateTextSize(const std::string& text);
						
			
		protected:
			/** populates the Image in the drawsource with a RGB conversion of the supplied 1Bit mono image
			 *  @note uses the first two records in the specified pallette for conversion
			 */
			void populateImageFromMono(const DrawSourcePtr& dsp, const PixelSize& dimensions, size_t rowlen, void* data, size_t pxoffset, const RGBAQuad* pal);

			/** populates the Image in the drawsource with a RGB conversion of the supplied 8Bit palletized image
			 *  @note uses the records in the specified pallette for conversion
			 */
			void populateImageFrom8BitPal(const DrawSourcePtr& dsp, const PixelSize& dimensions, size_t rowlen, void* data, size_t pxoffset, const RGBAQuad* pal);

			/// The map from character to the drawing source it represents
			typedef std::map<FontCharType, DrawSourcePtr> GlyphMap;

			/// Containing atlas
			TextureAtlasPtr mContainer;

			/// The map of the glyphs this font containts
			GlyphMap mRepresentedGlyphs;

			/// Name of this font
			std::string mName;

			/// Maximal height detected
			size_t mMaxHeight;

			/// Maximal width detected
			size_t mMaxWidth;

			/// Built flag (finalization indicator)
			bool mBuilt;
	};
};

#endif
