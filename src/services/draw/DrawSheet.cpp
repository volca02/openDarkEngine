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
#include "DrawService.h"

#include <OgreRenderQueue.h>

namespace Opde {

	/*----------------------------------------------------*/
	/*-------------------- DrawSheet ---------------------*/
	/*----------------------------------------------------*/
	DrawSheet::DrawSheet(DrawService* owner, const std::string& sheetName) : 
			mActive(false), 
			mSheetName(sheetName), 
			mOwner(owner),
			mResOverride(false),
			mWidth(1),
			mHeight(1) {
		// default to no visibility
		setVisible(false);
		
		// initial size
		mWidth = mOwner->getActualWidth();
		mHeight = mOwner->getActualHeight();
	};

	//------------------------------------------------------
	DrawSheet::~DrawSheet() {
		DrawOperationMap::iterator iend = mDrawOpMap.end();

		for (DrawOperationMap::iterator doi = mDrawOpMap.begin(); doi != iend; ++doi) {
			removeDrawOperation(doi->second);
		}

		mDrawOpMap.clear();

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

		setVisible(true);

		// Inform all buffers that the parent changed
		for (DrawBufferMap::iterator it = mDrawBufferMap.begin(); it != mDrawBufferMap.end(); ++it) {
			DrawBuffer* db = it->second;

			db->_parentChanged(this);
		}
		
		// Inform all the operations about the new sheet
		DrawOperationMap::iterator iend = mDrawOpMap.end();

		for (DrawOperationMap::iterator doi = mDrawOpMap.begin(); doi != iend; ++doi) {
			doi->second->_notifyActiveSheet(this);
		}
	}

	//------------------------------------------------------
	void DrawSheet::deactivate() {
		mActive = false;

		setVisible(false);
	}

	//------------------------------------------------------
	void DrawSheet::addDrawOperation(DrawOperation* drawOp) {
		DrawBuffer* buf = getBufferForOperation(drawOp, true);

		assert(buf);

		buf->addDrawOperation(drawOp);

		mDrawOpMap[drawOp->getID()] = drawOp;

		drawOp->onSheetRegister(this);
	}

	//------------------------------------------------------
	void DrawSheet::removeDrawOperation(DrawOperation* toRemove) {
		_removeDrawOperation(toRemove);

		toRemove->onSheetUnregister(this);
	}

	//------------------------------------------------------
	void DrawSheet::_sourceChanged(DrawOperation* op, const DrawSourcePtr& oldsrc) {
		DrawSourceBase::ID oldID = oldsrc->getSourceID();
		DrawSourceBase::ID newID = op->getDrawSourceBase()->getSourceID();
		
		if (oldID != newID) {
			DrawBuffer* dbo = getBufferForSourceID(oldID);
			DrawBuffer* dbn = getBufferForSourceID(oldID);
			
			// remove from the old buffer
			// add into the new one
			dbo->removeDrawOperation(op);
			dbn->addDrawOperation(op);
		}
	}
	
	//------------------------------------------------------
	void DrawSheet::_removeDrawOperation(DrawOperation* toRemove) {
		DrawBuffer* buf = getBufferForOperation(toRemove);

		assert(buf);

		buf->removeDrawOperation(toRemove);

		DrawOperationMap::iterator it = mDrawOpMap.find(toRemove->getID());

		if (it != mDrawOpMap.end())
			mDrawOpMap.erase(it);
	}

	//------------------------------------------------------
	void DrawSheet::_markDirty(DrawOperation* drawOp) {
		// Marking dirty could mean the buffer changes.
		// We ignore that. Atlasing can only happen before creating the Draw Ops.

		// look up the DrawBuffer for this DrawOp.
		DrawBuffer* buf = getBufferForOperation(drawOp);

		assert(buf);

		// mark the buffer dirty, queue the change
		buf->queueUpdate(drawOp);
	}

	//------------------------------------------------------
	DrawBuffer* DrawSheet::getBufferForOperation(DrawOperation* drawOp, bool autoCreate) {
		DrawSourceBasePtr dsb = drawOp->getDrawSourceBase();
		
		// todf
		DrawBuffer *db = getBufferForSourceID(dsb->getSourceID());
		
		if (db) // if found, return
			return db;
		
		// not found, autocreate?
		if (autoCreate) {
			const Ogre::MaterialPtr& mp = dsb->getMaterial();
			DrawBuffer* db = new DrawBuffer(mp);
			// TODO: And here as well. ID should be enough
			mDrawBufferMap[dsb->getSourceID()] = db;
			return db;
		} else {
			return NULL; // nope, just return null
		}
	}
	
	//------------------------------------------------------
	DrawBuffer* DrawSheet::getBufferForSourceID(DrawSourceBase::ID id) {
		DrawBufferMap::iterator it = mDrawBufferMap.find(id);

		if (it != mDrawBufferMap.end()) {
			return it->second;
		}
		
		return NULL;
	}

	//------------------------------------------------------
	void DrawSheet::rebuildBuffers() {
		DrawBufferMap::iterator it = mDrawBufferMap.begin();

		for (; it != mDrawBufferMap.end(); ++it) {
			if (it->second->isDirty())
				it->second->update();
		}
	}
	
	//------------------------------------------------------
	void DrawSheet::setResolutionOverride(bool override, size_t width, size_t height) {
		mResOverride = override;
		
		if (mResOverride) {
			assert(mWidth);
			assert(mHeight);
			
			mWidth = width;
			mHeight = height;
		} else {
			mWidth  = mOwner->getActualWidth();
			mHeight = mOwner->getActualHeight();
		}
		
		// must queue rebuild, the quad coords are invalid now		
		markBuffersDirty();
	}
	
	//------------------------------------------------------
	Ogre::Real DrawSheet::convertToScreenSpaceX(int x) {
		return mOwner->convertToScreenSpaceX(x, mWidth);
	}
	
	//------------------------------------------------------
	Ogre::Real DrawSheet::convertToScreenSpaceY(int y) {
		return mOwner->convertToScreenSpaceY(y, mHeight);
	}
	
	//------------------------------------------------------
	Ogre::Real DrawSheet::convertToScreenSpaceZ(int z) {
		return mOwner->convertToScreenSpaceZ(z);
	}
	
	//------------------------------------------------------
	void DrawSheet::_setResolution(size_t width, size_t height) {
		if (!mResOverride) {
			mWidth = width;
			mHeight=  height;
		}
	}
	
	//------------------------------------------------------
	ClipRect DrawSheet::getClipRect(int left, int right, int top, int bottom) {
		ClipRect cr;
		
		cr.left   = convertToScreenSpaceX(left);
		cr.right  = convertToScreenSpaceX(right);
		cr.top    = convertToScreenSpaceY(top);
		cr.bottom = convertToScreenSpaceY(bottom);
		cr.noClip = false;
		
		return cr;
	}
	
	//------------------------------------------------------
	const Ogre::String& DrawSheet::getMovableType() const {
		static Ogre::String type = "DrawSheet";
        return type;
	};

	//------------------------------------------------------
	const Ogre::AxisAlignedBox& DrawSheet::getBoundingBox() const {
		static Ogre::AxisAlignedBox box;
		box.setInfinite();
		return box;
	};

	//------------------------------------------------------
	Ogre::Real DrawSheet::getBoundingRadius() const {
		return 1.0; // What radius is appropriate?
	};

	//------------------------------------------------------
	void DrawSheet::visitRenderables(Ogre::Renderable::Visitor* vis, bool debugRenderables) {
		// visit all the renderables - all draw buffers
		for (DrawBufferMap::iterator it = mDrawBufferMap.begin(); it != mDrawBufferMap.end(); ++it) {
			vis->visit(it->second, 0, debugRenderables);
		}
	};

	//------------------------------------------------------
	bool DrawSheet::isVisible() const {
		return true;
	}

	//------------------------------------------------------
	void DrawSheet::_updateRenderQueue(Ogre::RenderQueue* queue) {
		// do this for all the Buffers
		for (DrawBufferMap::iterator it = mDrawBufferMap.begin(); it != mDrawBufferMap.end(); ++it) {
			DrawBuffer* db = it->second;

			// could be done in Renderable::preRender hidden from the eyes
			if (db->isDirty())
				db->update();

			queue->addRenderable(db, db->getRenderQueueID(), OGRE_RENDERABLE_DEFAULT_PRIORITY);
		}
	};
	
	//-----------------------------------------------------
	void DrawSheet::markBuffersDirty() {
		for (DrawBufferMap::iterator it = mDrawBufferMap.begin(); it != mDrawBufferMap.end(); ++it) {
			DrawBuffer* db = it->second;

			db->markDirty();
		}
	}
};
