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
	struct QuadLess {
		bool operator()(const DrawQuad* a, const DrawQuad* b) const;
	};

	// forward decl
	struct DrawSource;

	/// Shared ptr to draw source
	typedef shared_ptr<DrawSource> DrawSourcePtr;

	/// A drawn bitmap source
	struct OPDELIB_EXPORT DrawSource {
		/** Texture this draw source represents */
		Ogre::TexturePtr texture;

		/** Material this draw source represents */
		Ogre::MaterialPtr material;

		/// displacement of the Image in the storage (position in atlas)
		Ogre::Vector2 displacement;

		/// size in units in storage (size in atlas)
		Ogre::Vector2 size;

		/// size in pixels of the DrawSource
		PixelCoord pixelSize;

		/// Will transform the Texture coordinates to those usable for rendering
		Ogre::Vector2 transform(const Ogre::Vector2& input);

		/** Helper transformation method for DrawSources now being atlased into the owner.
		* Position is coordinate in 0-1 of the atlas to which the image is translated while atlased.
		* Result is that the specified DrawSource is transformed into the atlas, and returned as a new pointer.
		* @param original The original DrawSource, not yet atlased
		* @param position The translation by which the image is moved, in pixels (size conversion is calculated)
		*/
		DrawSourcePtr atlas(DrawSourcePtr& dsrc, const PixelCoord& position);
	};
};

#endif
