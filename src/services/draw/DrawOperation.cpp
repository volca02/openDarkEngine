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

#include "DrawService.h"
#include "DrawOperation.h"
#include "DrawSheet.h"

namespace Opde {

	/*----------------------------------------------------*/
	/*-------------------- DrawBuffer --------------------*/
	/*----------------------------------------------------*/
	DrawOperation::DrawOperation(DrawService* owner, DrawOperation::ID id, const DrawSourcePtr& ds) :
			mID(id),
			mDrawSource(ds),
			mOwner(owner),
			mPosition(0,0),
			mZOrder(0) {
		// nothing to do besides this
	};

	//------------------------------------------------------
	DrawOperation::~DrawOperation() {
		// remove from all sheets left
		for (DrawSheetSet::iterator it = mUsingSheets.begin(); it != mUsingSheets.end(); ++it) {
			(*it)->_removeDrawOperation(this);
		}

		mUsingSheets.clear();
	};

	//------------------------------------------------------
	const DrawSourcePtr& DrawOperation::getDrawSource() const {
		return mDrawSource;
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
	void DrawOperation::_markDirty() {
		for (DrawSheetSet::iterator it = mUsingSheets.begin(); it != mUsingSheets.end(); ++it) {
			(*it)->_markDirty(this);
		}
	}
	
	//------------------------------------------------------
	void DrawOperation::setPosition(int x, int y) {
		mPosition.first = x;
		mPosition.second = y;
		
		positionChanged();
	}
	
	//------------------------------------------------------
	void DrawOperation::setZOrder(int z) {
		mZOrder = z;
		
		positionChanged();
	}

	//------------------------------------------------------
	void DrawOperation::positionChanged() {
		
	}
}
