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
 *        $Id$
 *
 *****************************************************************************/

#include "DrawSheet.h"

namespace Opde {

	/*----------------------------------------------------*/
	/*-------------------- DrawSheet ---------------------*/
	/*----------------------------------------------------*/
	DrawSheet::DrawSheet() : mActive(false) {
		// nothing to do!
	};

	//------------------------------------------------------
	DrawSheet::~DrawSheet() {
		// TODO: destroy all the render buffers
		DrawBufferMap::iterator it = mDrawBufferMap.begin();

		for (; it != mDrawBufferMap.end(); ++it) {
			delete it->second;
		}

		mDrawBufferMap.clear();
	}

	//------------------------------------------------------
	void DrawSheet::activate() {
		mActive = true;

		// TODO: Need to inform all DrawBuffers?
	}

	//------------------------------------------------------
	void DrawSheet::deactivate() {
		mActive = false;
	}

	//------------------------------------------------------
	void DrawSheet::addDrawOperation(DrawOperation* drawOp) {
		DrawBuffer* buf = getBufferForOperation(drawOp);

		assert(buf);

		buf->addDrawOperation(drawOp);

		mDrawOpMap[drawOp->getID()] = drawOp;
	}

	//------------------------------------------------------
	void DrawSheet::removeDrawOperation(DrawOperation* toRemove) {
		DrawBuffer* buf = getBufferForOperation(toRemove);

		assert(buf);

		buf->removeDrawOperation(toRemove);

		DrawOperationMap::iterator it = mDrawOpMap.find(toRemove->getID());

		if (it != mDrawOpMap.end())
			mDrawOpMap.erase(it);
	}

	//------------------------------------------------------
	void DrawSheet::_markDirty(DrawOperation* drawOp) {
		// look up the DrawBuffer for this DrawOp.
		DrawBuffer* buf = getBufferForOperation(drawOp);

		assert(buf);

		// mark the buffer dirty, queue the change
		buf->queueUpdate(drawOp);
	}

	//------------------------------------------------------
	DrawBuffer* DrawSheet::getBufferForOperation(DrawOperation* drawOp, bool autoCreate) {

		DrawBufferMap::iterator it = mDrawBufferMap.find(drawOp->getImageName());

		if (it != mDrawBufferMap.end()) {
			return it->second;
		}

		if (autoCreate) {
			const Ogre::String& inm = drawOp->getImageName();
			DrawBuffer* db = new DrawBuffer(inm);
			mDrawBufferMap[inm] = db;
			return db;
		} else {
			return NULL;
		}
	}

	//------------------------------------------------------
	void DrawSheet::rebuildBuffers() {
		DrawBufferMap::iterator it = mDrawBufferMap.begin();
		
		for (; it != mDrawBufferMap.end(); ++it) {
			if (it->second->isDirty())
				it->second->update();
		}
	}
};
