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
		DrawOperation(owner, id), mFontSource(fds), mLabel(label) {

		rebuild();
	}

	//------------------------------------------------------
	void RenderedLabel::setLabel(const std::string& label) {
		mLabel = label;
		rebuild();
	}

	//------------------------------------------------------
	void RenderedLabel::_rebuild() {
		std::string::iterator it = mLabel.begin();
		std::string::iterator end = mLabel.end();

		mDrawQuadList.clear();
		int x = 0, y = 0;

		// TODO: Clipping, reuse of quads if the quad count does not change, etc.
		while (it != end) {
			const unsigned char& chr = *it++;

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
				x += ds->getPixelSize().width;
				mDrawQuadList.push_back(dq);
			} else {
				x += mFontSource->getWidth();
			}
		}
	}

	//------------------------------------------------------
	void RenderedLabel::visitDrawBuffer(DrawBuffer* db) {
		// TODO: For every generated glyph instance
		// db->_queueDrawQuad(&mDrawQuad);
		DrawQuadList::iterator it = mDrawQuadList.begin();
		DrawQuadList::iterator end = mDrawQuadList.end();
		
		for (; it != end; ++it) {
			db->_queueDrawQuad(&*it);
		}
	}

	//------------------------------------------------------
	DrawSourceBase* RenderedLabel::getDrawSourceBase() {
		return mFontSource->getAtlas();
	}

	//------------------------------------------------------
	void RenderedLabel::fillQuad(int x, int y, const unsigned char chr, DrawSource* ds, DrawQuad& dq) {
		const PixelSize& ps = ds->getPixelSize();

		dq.positions.topleft     = mOwner->convertToScreenSpace(mPosition.first + x, mPosition.second + y, mZOrder);
		dq.positions.topright    = mOwner->convertToScreenSpace(mPosition.first + x + ps.width, mPosition.second + y, mZOrder);
		dq.positions.bottomleft  = mOwner->convertToScreenSpace(mPosition.first + x, mPosition.second + y + ps.height, mZOrder);
		dq.positions.bottomright = mOwner->convertToScreenSpace(mPosition.first + x + ps.width, mPosition.second + y + ps.height, mZOrder);

		ds->fillTexCoords(dq.texCoords);
	}

}
