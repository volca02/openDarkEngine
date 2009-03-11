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

	/** Rendered label. This class represents single font text area, possibly wrapped and coloured. */
	class RenderedLabel : public DrawOperation {
		public:
			RenderedLabel(DrawService* owner, DrawOperation::ID id, FontDrawSource* fds, const std::string& label);
			
			virtual ~RenderedLabel();

			void visitDrawBuffer(DrawBuffer* db);

			/// A shortcut to call clear+addText(label);
			void setLabel(const std::string& label);

			/// Adds a segment of text with a given color
			void addText(const std::string& text, const Ogre::ColourValue& colour);
			
			/// Clears all the text from the label
			void clearText();
			
			/** Calculates a width and height of the given text string.
			 *  The resulting size is of a unclipped, newline respecting text
			 */ 
			PixelSize calculateTextSize(const std::string& text);
			
			DrawSourceBase* getDrawSourceBase() const;

		protected:
			struct TextSegment {
				Ogre::ColourValue colour;
				std::string text;
			};
			
			typedef std::list< TextSegment > SegmentList;
			
			/// Rebuilds the label - makes new glyph instances
			void _rebuild();

			/// Frees all allocated instances stored in the quad list, then clears the quad list itself
			void freeQuadList();
			
			void fillQuad(int x, int y, const unsigned char chr, DrawSource* ds, DrawQuad& dq);

			typedef std::list<DrawQuad*> DrawQuadList;

			DrawQuadList mDrawQuadList;

			FontDrawSource* mFontSource;

			SegmentList mText; 
	};

};

#endif
