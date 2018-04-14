/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 *Software Foundation; either version 2 of the License, or (at your option) any
 *later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 *details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 *along with this program; if not, write to the Free Software Foundation, Inc.,
 *59 Temple Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 * http://www.gnu.org/copyleft/lesser.txt.
 *
 *
 *	$Id$
 *
 *****************************************************************************/

#ifndef __DARKCAMERA_H
#define __DARKCAMERA_H

#include "config.h"

#include "DarkBspPrerequisites.h"
// need this for screen rects.
#include "DarkPortal.h"

#include <OgreCamera.h>

namespace Ogre {

/** Camera specialized for BSP/Portal combination */
class OPDELIB_EXPORT DarkCamera : public Camera {
    friend class DarkSceneManager;

public:
    DarkCamera(const String &name, SceneManager *sm);

    ~DarkCamera();

    virtual void _notifyMoved(void);

    /// internal method used to retrieve the visible node list
    const BspNodeList &_getVisibleNodes(void) const;

    unsigned long getTraversalTime(void) const { return mTraversalTime; };
    unsigned long getVisibleCellCount(void) const { return mCellCount; };

protected:
    /// The camera's position has changed and need's a recalc (overriden from
    /// Frustum::updateFrustumImpl)
    virtual void updateFrustumImpl(void) const;

    virtual void updateViewImpl(void) const;

    void updateVisibleCellList() const;

    // The BSP tree used to update the visible cells
    BspTree *mBspTree;

    mutable BspNodeList mVisibleCells;

    mutable unsigned long mTraversalTime;
    mutable unsigned int mCellCount;
    mutable bool mIsDirty;

    mutable ScreenRectCache mRects;
    mutable unsigned int mUpdateID;
};

}; // namespace Ogre

#endif
