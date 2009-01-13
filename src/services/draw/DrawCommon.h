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

#include "DrawOperation.h"

#include <OgreVector3.h>
#include <OgreVector2.h>
#include <OgreColourValue.h>

namespace Opde {

	/// Universal rect. (Idea cloned after Canvas'es quad)
	template<typename T> struct DrawRect {
		T topleft;
		T topright;
		T bottomleft;
		T bottomright;
	};

	/// Draw quad - a single Rectangle that can be stored for rendering
	struct DrawQuad {
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
};

#endif
