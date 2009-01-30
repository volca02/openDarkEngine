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

#include <OgreVector3.h>
#include <OgreVector2.h>
#include <OgreColourValue.h>
#include <OgreImage.h>
#include <OgreTexture.h>
#include <OgreMaterial.h>

namespace Opde {

	/// Universal rect. (Idea cloned after Canvas'es quad)
	template<typename T> struct DrawRect {
		T topleft;
		T topright;
		T bottomleft;
		T bottomright;
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

		size_t width;
		size_t height;
	};

	/// Draw quad - a single Rectangle that can be stored for rendering
	struct OPDELIB_EXPORT DrawQuad {
		DrawRect<Ogre::Vector3> positions;
		DrawRect<Ogre::Vector2> texCoords;
		DrawRect<Ogre::ColourValue> colors;

		// TODO: IBO and VBO position markers (mutable)
	};

	/// List of drawn quads (pointers to avoid copying)
	typedef std::vector<const DrawQuad*> DrawQuadList;

	/// Sorting comparison op.
	struct OPDELIB_EXPORT QuadLess {
		bool operator()(const DrawQuad* a, const DrawQuad* b) const;
	};

	// forward decl
	struct DrawSource;

	/// Shared ptr to draw source
	typedef shared_ptr<DrawSource> DrawSourcePtr;

	/// A drawn bitmap source
	struct OPDELIB_EXPORT DrawSource {
		/// Draw source image ID. Atlased draw sources have the same ID. It is used to organize buffers.
		typedef size_t ID;

		/// Identifies the texture id (image id).
		ID sourceID;

		/** Texture this draw source represents */
		Ogre::TexturePtr texture;
		
		/** Source image of this draw source - may be lost after atlassing this, internal  */
		Ogre::Image	image;

		/** Material this draw source represents */
		Ogre::MaterialPtr material;

		/// displacement of the Image in the storage (position in atlas)
		Ogre::Vector2 displacement;

		/// size in units in storage (size in atlas)
		Ogre::Vector2 size;

		/// size in pixels of the DrawSource
		PixelSize pixelSize;

		/// Additional pointer, used by atlas
		void* placement;

		inline Ogre::Vector2 getPixelSizeVector() { return Ogre::Vector2(pixelSize.width, pixelSize.height); };

		/// Will transform the Texture coordinates to those usable for rendering
		Ogre::Vector2 transform(const Ogre::Vector2& input);
	};

	struct DrawSourceLess {
		bool operator() (const DrawSourcePtr& a, const DrawSourcePtr& b) {
			size_t sa = a->pixelSize.width * a->pixelSize.height;
			size_t sb = b->pixelSize.width * b->pixelSize.height;
			return sa < sb;
		}
	};
};

#endif
