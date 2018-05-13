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
#include "DrawService.h"
#include "DrawSheet.h"

namespace Opde {

/*----------------------------------------------------*/
/*-------------------- 	DrawOperation ----------------*/
/*----------------------------------------------------*/
DrawOperation::DrawOperation(DrawService *owner)
    : mOwner(owner),
      mActiveSheet(NULL),
      mPosition(0, 0),
      mZOrder(0),
      mIsDirty(false)
{
    // nothing to do besides this
};

//------------------------------------------------------
DrawOperation::~DrawOperation() {
    // at least break the circle (but clear should've been called anyway)
    mUsingSheets.clear();
};

//------------------------------------------------------
void DrawOperation::visitDrawBuffer(DrawBuffer *db){
    // empty. To be overridden by ancestor to do the rendering (via
    // DrawBuffer::_queueDrawQuad)
};

//------------------------------------------------------
void DrawOperation::onSheetRegister(DrawSheet *sheet) {
    mUsingSheets.insert(sheet);
};

//------------------------------------------------------
void DrawOperation::onSheetUnregister(DrawSheet *sheet) {
    mUsingSheets.erase(sheet);
};

//------------------------------------------------------
void DrawOperation::_sourceChanged(const DrawSourcePtr &old) {
    for (DrawSheetSet::iterator it = mUsingSheets.begin();
         it != mUsingSheets.end(); ++it) {
        (*it)->_sourceChanged(this, old);
    }

    // Texture didn't change, be sure to invalidate at least
    if (old->getSourceID() == getDrawSourceBase()->getSourceID()) {
        _markDirty();
    }
}

//------------------------------------------------------
void DrawOperation::_markDirty() {
    mIsDirty = true;

    for (DrawSheetSet::iterator it = mUsingSheets.begin();
         it != mUsingSheets.end(); ++it) {
        (*it)->_markDirty(this);
    }
}

//------------------------------------------------------
void DrawOperation::setPosition(int x, int y) {
    mPosition.first = x;
    mPosition.second = y;

    _markDirty();
}

//------------------------------------------------------
void DrawOperation::setPosition(const PixelCoord &pos) {
    mPosition = pos;

    _markDirty();
}

//------------------------------------------------------
void DrawOperation::setZOrder(int z) {
    mZOrder = z;

    _markDirty();
}

//------------------------------------------------------
void DrawOperation::rebuild() {
    // update the clip rect -> on screen projection
    if (mActiveSheet) {
        mActiveSheet->convertClipToScreen(mClipRect, mClipOnScreen);
    } else {
        mClipOnScreen.noClip = true;
    }

    _rebuild();
    mIsDirty = false;
}

//------------------------------------------------------
void DrawOperation::_notifyActiveSheet(DrawSheet *actsh) {
    mActiveSheet = actsh;

    // need rebuild thanks to different sheet dimensions, etc.
    _markDirty();
}

//------------------------------------------------------
void DrawOperation::_rebuild() {
    // nothing
}

//------------------------------------------------------
void DrawOperation::clear() {
    // remove from all sheets left
    for (DrawSheetSet::iterator it = mUsingSheets.begin();
         it != mUsingSheets.end(); ++it) {
        (*it)->_removeDrawOperation(this);
    }

    mUsingSheets.clear();
}

//------------------------------------------------------
void DrawOperation::setClipRect(const ClipRect &cr) {
    mClipRect = cr;

    _markDirty();
}

} // namespace Opde
