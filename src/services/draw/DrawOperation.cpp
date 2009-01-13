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

#include "DrawOperation.h"
#include "DrawSheet.h"

namespace Opde {

	/*----------------------------------------------------*/
	/*-------------------- DrawBuffer --------------------*/
	/*----------------------------------------------------*/
	DrawOperation::DrawOperation(DrawService* owner, ID id, size_t order, const Ogre::String& name)
			: mID(id),
			mImageName(name),
			mOwner(owner) {
		//
	};

	//------------------------------------------------------
	DrawOperation::~DrawOperation() {
	};

	//------------------------------------------------------
	const Ogre::String& DrawOperation::getMaterialName() const {
		return mImageName;
	};

	//------------------------------------------------------
	void DrawOperation::visitDrawBuffer(DrawBuffer* db) {
		// empty. To be overridden by ancestor to do the rendering (via DrawBuffer::_queueDrawQuad)
	};

	//------------------------------------------------------
	void DrawOperation::onSheetRegister(DrawSheet* sheet) {
		mUsingSheets.insert(sheet);
	};

	//------------------------------------------------------
	void DrawOperation::onSheetUnregister(DrawSheet* sheet) {
		mUsingSheets.erase(sheet);
	};

	//------------------------------------------------------
	void DrawOperation::onChange() {
		for (DrawSheetSet::iterator it = mUsingSheets.begin(); it != mUsingSheets.end(); ++it) {
			(*it)->_markDirty(this);
		}
	}
}
