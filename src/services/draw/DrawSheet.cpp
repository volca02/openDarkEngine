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
DrawSheet::DrawSheet(DrawService *owner, const std::string &sheetName)
    : mActive(false), mSheetName(sheetName), mOwner(owner), mResOverride(false),
      mWidth(1), mHeight(1) {
    // default to no visibility
    setVisible(false);

    // initial size
    mWidth = mOwner->getActualWidth();
    mHeight = mOwner->getActualHeight();
};

//------------------------------------------------------
DrawSheet::~DrawSheet() { clear(); }

//------------------------------------------------------
void DrawSheet::activate() {
    mActive = true;

    setVisible(true);

    // Inform all buffers that the parent changed
    for (auto &p : mDrawBufferMap) {
        p.second->_parentChanged(this);
    }

    // Inform all the operations about the new sheet
    for (auto &op : mDrawOps) {
        op->_notifyActiveSheet(this);
    }
}

//------------------------------------------------------
void DrawSheet::deactivate() {
    mActive = false;
    setVisible(false);
}

//------------------------------------------------------
void DrawSheet::addDrawOperation(DrawOperation *drawOp) {
    DrawBuffer *buf = getBufferForOperation(drawOp, true);

    assert(buf);

    if (buf) {
        buf->addDrawOperation(drawOp);
        mDrawOps.insert(drawOp);
        drawOp->onSheetRegister(this);
    }
}

//------------------------------------------------------
void DrawSheet::removeDrawOperation(DrawOperation *toRemove) {
    _removeDrawOperation(toRemove);

    toRemove->onSheetUnregister(this);
}

//------------------------------------------------------
void DrawSheet::_sourceChanged(DrawOperation *op, const DrawSourcePtr &oldsrc) {
    auto oldID = oldsrc->getSource();
    auto newID = op->getDrawSourceBase()->getSource();

    if (oldID != newID) {
        DrawBuffer *dbo = getBufferForSource(oldID);
        DrawBuffer *dbn = getBufferForSource(newID);

        // remove from the old buffer
        // add into the new one
        dbo->removeDrawOperation(op);
        dbn->addDrawOperation(op);
    }
}

//------------------------------------------------------
void DrawSheet::_removeDrawOperation(DrawOperation *toRemove) {
    DrawBuffer *buf = getBufferForOperation(toRemove);

    if (buf)
        buf->removeDrawOperation(toRemove);

    mDrawOps.erase(toRemove);
}

//------------------------------------------------------
void DrawSheet::_markDirty(DrawOperation *drawOp) {
    // Marking dirty could mean the buffer changes.
    // We ignore that. Atlasing can only happen before creating the Draw Ops.

    // look up the DrawBuffer for this DrawOp.
    DrawBuffer *buf = getBufferForOperation(drawOp);

    assert(buf);

    // mark the buffer dirty, queue the change
    buf->queueUpdate(drawOp);
}

//------------------------------------------------------
DrawBuffer *DrawSheet::getBufferForOperation(DrawOperation *drawOp,
                                             bool autoCreate) {
    DrawSourceBasePtr dsb = drawOp->getDrawSourceBase();

    // todf
    DrawBuffer *db = getBufferForSource(dsb->getSource());

    if (db) // if found, return
        return db;

    // not found, autocreate?
    if (autoCreate) {
        const Ogre::MaterialPtr &mp = dsb->getMaterial();
        DrawBuffer *db = new DrawBuffer(mp);
        mDrawBufferMap[dsb->getSource()].reset(db);
        return db;
    } else {
        return NULL; // nope, just return null
    }
}

//------------------------------------------------------
DrawBuffer *DrawSheet::getBufferForSource(DrawSourceBase *id) {
    DrawBufferMap::iterator it = mDrawBufferMap.find(id);

    if (it != mDrawBufferMap.end()) {
        return it->second.get();
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
void DrawSheet::setResolutionOverride(bool override, size_t width,
                                      size_t height) {
    mResOverride = override;

    if (mResOverride) {
        assert(mWidth);
        assert(mHeight);

        mWidth = width;
        mHeight = height;
    } else {
        mWidth = mOwner->getActualWidth();
        mHeight = mOwner->getActualHeight();
    }

    // must queue rebuild, the quad coords are invalid now
    markBuffersDirty();
}

//------------------------------------------------------
Ogre::Real DrawSheet::convertToScreenSpaceX(int x) const {
    return mOwner->convertToScreenSpaceX(x, mWidth);
}

//------------------------------------------------------
Ogre::Real DrawSheet::convertToScreenSpaceY(int y) const {
    return mOwner->convertToScreenSpaceY(y, mHeight);
}

//------------------------------------------------------
Ogre::Real DrawSheet::convertToScreenSpaceZ(int z) const {
    return mOwner->convertToScreenSpaceZ(z);
}

//------------------------------------------------------
void DrawSheet::_setResolution(size_t width, size_t height) {
    if (!mResOverride) {
        mWidth = width;
        mHeight = height;
    }
}

//------------------------------------------------------
void DrawSheet::convertClipToScreen(const ClipRect &cr, ScreenRect &tgt) const {
    tgt.left = convertToScreenSpaceX(cr.left);
    tgt.right = convertToScreenSpaceX(cr.right);
    tgt.top = convertToScreenSpaceY(cr.top);
    tgt.bottom = convertToScreenSpaceY(cr.bottom);
    tgt.noClip = cr.noClip;
}

//------------------------------------------------------
void DrawSheet::queueRenderables(Ogre::RenderQueue *rq) {
    for (auto &p : mDrawBufferMap) {
        auto &db = p.second;

        // could be done in Renderable::preRender hidden from the eyes
        if (db->isDirty())
            db->update();

        // see if the renderable should be queued
        if (db->isEmpty())
            continue;

        rq->addRenderable(db.get(), db->getRenderQueueID(),
                          OGRE_RENDERABLE_DEFAULT_PRIORITY);
    }
}

//------------------------------------------------------
void DrawSheet::clear() {
    // this ensures we have not shared pointers hanging in there to cause
    // troubles for example.
    for (auto &dop : mDrawOps) {
        DrawBuffer *buf = getBufferForOperation(dop);

        assert(buf);

        buf->removeDrawOperation(dop);
        dop->onSheetUnregister(this);
    }

    mDrawOps.clear();
    mDrawBufferMap.clear();
}

//------------------------------------------------------
const Ogre::String &DrawSheet::getMovableType() const {
    static Ogre::String type = "DrawSheet";
    return type;
};

//------------------------------------------------------
const Ogre::AxisAlignedBox &DrawSheet::getBoundingBox() const {
    static Ogre::AxisAlignedBox box;
    box.setInfinite();
    return box;
};

//------------------------------------------------------
Ogre::Real DrawSheet::getBoundingRadius() const {
    return 1.0; // What radius is appropriate?
};

//------------------------------------------------------
void DrawSheet::visitRenderables(Ogre::Renderable::Visitor *vis,
                                 bool debugRenderables)
{
    for (auto &p : mDrawBufferMap) {
        vis->visit(p.second.get(), 0, debugRenderables);
    }
};

//------------------------------------------------------
bool DrawSheet::isVisible() const { return true; }

//------------------------------------------------------
void DrawSheet::_updateRenderQueue(Ogre::RenderQueue *queue){
    // do this for all the Buffers
    /*for (DrawBufferMap::iterator it = mDrawBufferMap.begin(); it !=
    mDrawBufferMap.end(); ++it) { DrawBuffer* db = it->second;

            // could be done in Renderable::preRender hidden from the eyes
            if (db->isDirty())
                    db->update();

            queue->addRenderable(db, db->getRenderQueueID(),
    OGRE_RENDERABLE_DEFAULT_PRIORITY);
    }*/
};

//-----------------------------------------------------
void DrawSheet::markBuffersDirty() {
    for (auto &p : mDrawBufferMap) {
        p.second->markDirty();
    }
}
}; // namespace Opde
