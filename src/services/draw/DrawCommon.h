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


#ifndef __DRAWCOMMON_H
#define __DRAWCOMMON_H

#include "SharedPtr.h"
#include "integers.h"

#include <OgreVector3.h>
#include <OgreVector2.h>
#include <OgreColourValue.h>
#include <OgreImage.h>
#include <OgreTexture.h>
#include <OgreMaterial.h>

namespace Opde {

	/// Universal rect. Specifies left, right, top and bottom coordinates
	template<typename T> struct DrawRect {
		T left;
		T right;
		T top;
		T bottom;
	};

	/// A pixel coordinates type
	typedef std::pair<int, int> PixelCoord;

	/// A pixel dimensions type
	struct OPDELIB_EXPORT PixelSize {
		PixelSize(size_t w, size_t h) {
			width = w;
			height = h;
		};

		PixelSize() : width(0), height(0) {};

		PixelSize(const PixelSize& b) : width(b.width), height(b.height) { };

		inline size_t getPixelArea() const { return width * height; };

		const PixelSize& operator=(const PixelSize& b) { width = b.width; height = b.height; return *this; };

		size_t width;
		size_t height;
	};

	/// Draw quad - a single Rectangle that can be stored for rendering. All coordinates are already transformed as needed
	struct OPDELIB_EXPORT DrawQuad {
		/// Positions of the rect
		DrawRect<Ogre::Real> positions;
		/// Positions in the texture space
		DrawRect<Ogre::Real> texCoords;
		/// Color of the rect
		Ogre::ColourValue color;
		/// The Z value to use (after transform)
		Ogre::Real depth;
		// TODO: IBO and VBO position markers (mutable) (That means current position specifier)
	};


	/// A Clipping rectangle - used to set rendered boundaries for rendered (drawn) objects
	struct OPDELIB_EXPORT ClipRect {
		ClipRect() : left(0), right(0), top(0), bottom(0), noClip(true) {};
		Ogre::Real left;
		Ogre::Real right;
		Ogre::Real top;
		Ogre::Real bottom;

		/// If set to true (default) the clipping is not done
		bool noClip;

		void operator=(const ClipRect& b) {
			left = b.left;
			right = b.right;
			top = b.top;
			bottom = b.bottom;
			noClip = b.noClip;
		}
		
		/** Clips a specified DrawQuad to the specified boundaries.
		* @param quad The DrawQuad which is to be clipped
		* @note this method simplifies it's work by assuming the input is a rectangular DrawQuad, not a trapezoid
		* @return true if the clipping had a nonzero result
		*/
		bool clip(DrawQuad& quad) {
			// noclip rect, return true
			if (noClip)
				return true;

			// invalid cliprect, return false
			// bottom is less than the top (screen coordinates go negative downwards, as in graphs)
			if ((left > right) || (top < bottom))
				return false;

			// Okay, we should clip.

			// first we gather the txt coordinates of the corners (the same manner we use here)
			Ogre::Real tleft, tright, ttop, tbottom;

			tleft   = quad.texCoords.left;
			tright  = quad.texCoords.right;
			ttop    = quad.texCoords.top;
			tbottom = quad.texCoords.bottom;

			// Then we gather the screen positions
			Ogre::Real pleft, pright, ptop, pbottom;

			pleft   = quad.positions.left;
			pright  = quad.positions.right;
			ptop    = quad.positions.top;
			pbottom = quad.positions.bottom;

			// No intersection?
			if ((pleft > right) || (pright < left) || (ptop < bottom) || (pbottom > top))
				return false;

			// horizontal position to tex coord conversion coefficient (slope)
			Ogre::Real hconv = (tright-tleft)/(pright-pleft);
			// and a vertical one
			Ogre::Real vconv = (ttop-tbottom)/(ptop-pbottom);

			// And here we clip finally
			if (pleft < left) {
				// move the position
				quad.positions.left = left;
				
				// move the txt coord (original left plus the slope * difference added)
				quad.texCoords.left = tleft + (left - pleft)*hconv;
			}

			if (pright > right) {
				quad.positions.right = right;
				quad.texCoords.right = tright + (right - pright) * hconv;
			}

			if (pbottom < bottom) {
				quad.positions.bottom = bottom;
				quad.texCoords.bottom = tbottom + (bottom - pbottom) * vconv;
			}

			if (ptop > top) {
				quad.positions.top = top;
				quad.texCoords.top = ttop + (top - ptop) * vconv;				
			}

			return true;
		};
	};


	/// List of drawn quads (pointers to avoid copying)
	typedef std::vector<const DrawQuad*> DrawQuadList;

	/// Sorting comparison op.
	struct OPDELIB_EXPORT QuadLess {
		bool operator()(const DrawQuad* a, const DrawQuad* b) const;
	};

	// forward decl
	struct DrawSource;

	// A base rendering source info - material and texture
	class OPDELIB_EXPORT DrawSourceBase {
		public:
			/// Draw source image ID. Atlased draw sources have the same ID. It is used to organize buffers.
			typedef size_t ID;

			/// Constructor
			DrawSourceBase(ID srcID, const Ogre::MaterialPtr& mat, const Ogre::TexturePtr& tex);
			
			/// NULL-setting constructor
			DrawSourceBase();
			
			/// Material getter
			inline Ogre::MaterialPtr getMaterial() const { return mMaterial; };

			/// Texture getter
			inline Ogre::TexturePtr getTexture() const { return mTexture; };
			
			/// Source (texture id) getter
			inline ID getSourceID() const { return mSourceID; };
			
			/// Source (texture id) setter
			inline void setSourceID(ID id) { mSourceID = id; };

			/// Returns a vector of width and height
			inline Ogre::Vector2 getPixelSizeVector() { return Ogre::Vector2(mPixelSize.width, mPixelSize.height); };
			
			inline const PixelSize& getPixelSize() const { return mPixelSize; };

			/// @return The area covered by the image pixels
			inline size_t getPixelArea() const { return mPixelSize.getPixelArea(); };
			
		protected:
			/// Material used for rendering of this DrawSourceBase
			Ogre::MaterialPtr mMaterial;

			/// Texture used for rendering of this DrawSourceBase
			Ogre::TexturePtr mTexture;

			/// Identifies the texture id (image id).
			ID mSourceID;

			/// size in pixels of the DrawSource
			PixelSize mPixelSize;
	};
	
	/// A drawn bitmap source (can be a part of an atlas)
	class OPDELIB_EXPORT DrawSource : public DrawSourceBase {
		public:
			DrawSource();
			
			DrawSource(ID id, const Ogre::MaterialPtr& mat, const Ogre::TexturePtr& tex);
			
			/// Will transform the Texture coordinates to those usable for rendering
			Ogre::Vector2 transform(const Ogre::Vector2& input);
			
			/// Transforms the X part of the texture coordinate
			Ogre::Real transformX(Ogre::Real x);
			
			/// Transforms the Y part of the texture coordinate
			Ogre::Real transformY(Ogre::Real y);

			inline void* getPlacementPtr() const { return mPlacement; };
			
			inline void setPlacementPtr(void* placement) { mPlacement = placement; };
			
			/// Atlas insertion notifier. Will modify the image UV transforms to fit into the specified position 
			void atlas(const Ogre::MaterialPtr& mat, size_t x, size_t y, size_t width, size_t height);

			/// Image getter (Image is used for pre-atlas image data storage)
			inline Ogre::Image& getImage() { return mImage; };
			
			/** Loads image data into the mImage. This is preffered over getImage().load() because it also updates pixels size. */
			void loadImage(const Ogre::String& name, const Ogre::String& group);
			
			/// Updates the pixel size from the supplied image. Do NOT forget to call this after loading the image by hand.
			void updatePixelSizeFromImage();
			
			/// Fills the specified DrawRect to represent the full image of this draw source
			void fillTexCoords(DrawRect<Ogre::Real>& tc);
		protected:

			/** Source image of this draw source - may be lost after atlassing this, internal  */
			Ogre::Image	mImage;

			/// displacement of the Image in the storage (position in atlas)
			Ogre::Vector2 mDisplacement;

			/// size in units in storage (size in atlas)
			Ogre::Vector2 mSize;
	
			/// Additional pointer, used by atlas
			void* mPlacement;
			
			/// True if this ds is already part of some atlas
			bool mAtlassed;
	
	};

	struct OPDELIB_EXPORT DrawSourceLess {
		bool operator() (const DrawSource* a, const DrawSource* b) {
			size_t sa = a->getPixelArea();
			size_t sb = b->getPixelArea();
			return sa > sb;
		}
	};

	/// A font character type
	typedef uint16_t FontCharType;

	/// A single RGBA color
	struct RGBAQuad {
		uint8_t red;
		uint8_t green;
		uint8_t blue;
		uint8_t alpha;
	};

	typedef enum {
		DPF_MONO,
		DPF_8BIT,
		DPF_RGBA
	} DarkPixelFormat;



};

#endif
