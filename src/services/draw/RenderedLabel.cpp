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

#include "RenderedLabel.h"
#include "DrawService.h"
#include "DrawBuffer.h"
#include "TextureAtlas.h"

using namespace Ogre;

namespace Opde {

	/*----------------------------------------------------*/
	/*-------------------- RenderedLabel -----------------*/
	/*----------------------------------------------------*/
	RenderedLabel::RenderedLabel(DrawService* owner, DrawOperation::ID id, FontDrawSource* fds, const std::string& label) :
		DrawOperation(owner, id), mFontSource(fds), mText() {

		rebuild();
	}
	
	//------------------------------------------------------
	RenderedLabel::~RenderedLabel() {
		freeQuadList();
	}

	//------------------------------------------------------
	void RenderedLabel::setLabel(const std::string& label) {
		clearText();
		addText(label, Ogre::ColourValue::White);
		
		_markDirty();
	}

	//------------------------------------------------------
	void RenderedLabel::addText(const std::string& text, const Ogre::ColourValue& colour) {
		TextSegment seg;
		seg.colour = colour;
		seg.text = text;
		
		mText.push_back(seg);
		_markDirty();
	}
				
	//------------------------------------------------------
	void RenderedLabel::clearText() {
		mText.clear();
		_markDirty();
	}

	//------------------------------------------------------
	PixelSize RenderedLabel::calculateTextSize(const std::string& text) {
		std::string::const_iterator cit = text.begin();
		std::string::const_iterator cend = text.end();
		
		size_t x = 0, y = 0;
		PixelSize sz(0,0);
		
		while (cit != cend) {
			const unsigned char chr = *cit++;

			if (chr == '\n') {
				y += mFontSource->getHeight();
				if (sz.width < x)
					sz.width = x;
				x = 0;
				continue;
			}

			// eat DOS line feeds as well...
			if (chr == '\r') {
				continue;
			}

			DrawSource* ds = mFontSource->getGlyph(chr);

			if (ds != NULL) {
				DrawQuad dq;
				
				x += ds->getPixelSize().width;
			} else {
				// move the maximal width (maybe 1px would be better?)
				x += mFontSource->getWidth(); 
			}
		}
		
		// not a first char on the line, so some text would be drawn. 
		// have to include the line's height though
		if (x != 0)
			sz.height = y + mFontSource->getHeight();;
			
		return sz;
	}
	
	//------------------------------------------------------
	void RenderedLabel::_rebuild() {
		SegmentList::iterator it = mText.begin();
		SegmentList::iterator end = mText.end();

		freeQuadList();
		
		int x = 0, y = 0;

		// TODO: Reuse of quads if the quad count does not change, etc.
		while (it != end) {
			// for each segment
			TextSegment& ts = *it++;
			
			std::string::iterator cit = ts.text.begin();
			std::string::iterator cend = ts.text.end();
			
			while (cit != cend) {
				const unsigned char chr = *cit++;

				if (chr == '\n') {
					y += mFontSource->getHeight();
					x = 0;
					continue;
				}
	
				// eat DOS line feeds as well...
				if (chr == '\r') {
					continue;
				}
	
				DrawSource* ds = mFontSource->getGlyph(chr);
	
				if (ds != NULL) {
					DrawQuad dq;
					
					fillQuad(x, y, chr, ds, dq);
					
					dq.color = ts.colour;
					
					x += ds->getPixelSize().width;
					
					// if clipping produced some non-empty result
					if (mClipRect.clip(dq)) { 
						// the quad is queued (by making a dynamically allocated copy)
						DrawQuad* toStore = new DrawQuad(dq);
						mDrawQuadList.push_back(toStore);
					}
				} else {
					x += mFontSource->getWidth(); // move the maximal width (maybe 1px would be better?)
				}
			}
		}
	}

	//------------------------------------------------------
	void RenderedLabel::freeQuadList() {
		DrawQuadList::iterator it = mDrawQuadList.begin();
		DrawQuadList::iterator end = mDrawQuadList.end();
		
		for (; it != end; ++it) {
			delete *it;
		}
		
		mDrawQuadList.clear();
	}
	
	//------------------------------------------------------
	void RenderedLabel::visitDrawBuffer(DrawBuffer* db) {
		DrawQuadList::iterator it = mDrawQuadList.begin();
		DrawQuadList::iterator end = mDrawQuadList.end();
		
		for (; it != end; ++it) {
			db->_queueDrawQuad(*it);
		}
	}

	//------------------------------------------------------
	DrawSourceBase* RenderedLabel::getDrawSourceBase() const {
		return mFontSource->getAtlas();
	}

	//------------------------------------------------------
	void RenderedLabel::fillQuad(int x, int y, const unsigned char chr, DrawSource* ds, DrawQuad& dq) {
		const PixelSize& ps = ds->getPixelSize();

		dq.positions.left    = mOwner->convertToScreenSpaceX(mPosition.first + x);
		dq.positions.right   = mOwner->convertToScreenSpaceX(mPosition.first + x + ps.width);
		dq.positions.top     = mOwner->convertToScreenSpaceY(mPosition.second + y);
		dq.positions.bottom  = mOwner->convertToScreenSpaceY(mPosition.second + y + ps.height);
		dq.depth = mOwner->convertToScreenSpaceZ(mZOrder);

		ds->fillTexCoords(dq.texCoords);
	}

}
