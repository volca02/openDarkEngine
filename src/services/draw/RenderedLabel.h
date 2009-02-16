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


#ifndef __RENDEREDLABEL_H
#define __RENDEREDLABEL_H

#include "DrawOperation.h"
#include "FontDrawSource.h"

#include <OgreVector3.h>

namespace Opde {

	class RenderedLabel : public DrawOperation {
		public:
			RenderedLabel(DrawService* owner, DrawOperation::ID id, FontDrawSource* fds, const std::string& label);

			void visitDrawBuffer(DrawBuffer* db);

			void setLabel(const std::string& label);

			DrawSourceBase* getDrawSourceBase();

		protected:
			/// Rebuilds the label - makes new glyph instances
			void rebuild();

			/// override that updates the image and marks dirty
			void positionChanged();

			void fillQuad(int x, int y, const unsigned char chr, DrawSource* ds, DrawQuad& dq);

			typedef std::list<DrawQuad> DrawQuadList;

			DrawQuadList mDrawQuadList;

			FontDrawSource* mFontSource;

			std::string mLabel;
	};

};

#endif
