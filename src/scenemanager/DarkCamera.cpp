/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
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

#include "DarkCamera.h"
#include "DarkBspNode.h"
#include "DarkBspTree.h"
#include "DarkSceneManager.h"
#include "OgreRoot.h"
#include "OgreTimer.h"

#include "tracer.h"

namespace Ogre {

// ----------------------------------------------------------------------
DarkCamera::DarkCamera(const String &name, SceneManager *sm)
    : Camera(name, sm),
      mTraversal(),
      mIsDirty(false)
{
}

// ----------------------------------------------------------------------
DarkCamera::~DarkCamera() {}

// ----------------------------------------------------------------------
void DarkCamera::updateFrustumImpl(void) const {
    // first call the parent update, then do our stuff
    Camera::updateFrustumImpl();

    // If we're here, camera changed, so we need to update the cell list
    mIsDirty = true;
}

// ----------------------------------------------------------------------
void DarkCamera::updateViewImpl(void) const {
    // first call the parent update, then do our stuff
    Camera::updateViewImpl();

    // If we're here, camera changed, so we need to update the cell list
    mIsDirty = true;
}

// ----------------------------------------------------------------------
void DarkCamera::_notifyMoved(void) {
    Camera::_notifyMoved();

    mIsDirty = true;
}

// ----------------------------------------------------------------------
void DarkCamera::clearVisibleCells() {
    mTraversal.clear();
}

// ----------------------------------------------------------------------
const BspNodeSet &DarkCamera::_getVisibleNodes(void) const {
    return mTraversal.visibleCells();
}

// ----------------------------------------------------------------------
void DarkCamera::updateVisibleCellList() const {
    TRACE_METHOD;

    if (!mIsDirty) // nothing to do...
        return;

    unsigned long startTime =
        Root::getSingleton().getTimer()->getMilliseconds();

    // cut plane, with a small correction to avoid projection singularities
    Plane cutPlane(getRealDirection(),
                   getRealPosition() + getRealDirection() * 0.01f);


    const Matrix4 &viewM = getViewMatrix();
    const Matrix4 &projM = getProjectionMatrix();

    Matrix4 toScreen = projM * viewM;

    PortalFrustum cameraFrustum = PortalFrustum(this);

    mTraversal.traverse(static_cast<DarkSceneManager *>(mSceneMgr)->getBspTree(),
                        getDerivedPosition(),
                        toScreen, cutPlane,
                        cameraFrustum);

    mIsDirty = false;
    mCellCount = mTraversal.visibleCells().size();

    // All done!
    mTraversalTime =
        Root::getSingleton().getTimer()->getMilliseconds() - startTime;
}

}; // namespace Ogre
