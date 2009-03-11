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


#ifndef __RENDEREDIMAGE_H
#define __RENDEREDIMAGE_H

#include "DrawCommon.h"
#include "DrawOperation.h"

#include <OgreVector3.h>

namespace Opde {
	class RenderedImage : public DrawOperation {
		public:
			RenderedImage(DrawService* owner, DrawOperation::ID id, DrawSource* ds);

			void visitDrawBuffer(DrawBuffer* db);
			
			DrawSourceBase* getDrawSourceBase() const;
			
			void setDrawSource(DrawSource* nsrc);

		protected:
			/// override that updates the image and marks dirty
			void _rebuild();
			
			void _setNewDrawSource();
			
			DrawQuad mDrawQuad;
			
			DrawSource *mDrawSource;
			
			bool mInClip;
	};

};

#endif
