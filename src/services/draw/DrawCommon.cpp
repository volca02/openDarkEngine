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

	/// Sorting comparison op.
	bool QuadLess::operator()(const DrawQuad* a, const DrawQuad* b) const {
		return a->positions.topleft.z < b->positions.topleft.z;
	};

	Vector2 DrawSource::transform(const Vector2& input) {
		Vector2 tran(input);

		tran *= size;
		tran += displacement;
	};

	DrawSourcePtr DrawSource::atlas(DrawSourcePtr& dsrc, const PixelCoord& position) {
		// we have to transform the pixel size of the image, recalculate the size
		// and position
		DrawSourcePtr nds = new DrawSource();

		nds->pixelSize = dsrc->pixelSize;
		nds->size.x = (float)nds->pixelSize.first / pixelSize.first;
		nds->size.y = (float)nds->pixelSize.second / pixelSize.second;
		nds->displacement.x = (float)position.first / pixelSize.first;
		nds->displacement.y = (float)position.second / pixelSize.second;

		nds->texture = texture;
		nds->material = material;

		return nds;
	}
};
