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

#ifndef __DARKPORTALTRAVERSAL_H
#define __DARKPORTALTRAVERSAL_H

#include "DarkBspPrerequisites.h"
#include "DarkPortal.h"

namespace Ogre {

class DarkBspTree;

class DarkPortalTraversal {
public:
    using BspNodes = BspNodeSet;

    DarkPortalTraversal(BspTree *t) : mBspTree(t) {}

    const BspNodes &visibleCells() const { return mVisibleCells; }

    // adds a given cell manually
    void addCell(BspNode *n) { mVisibleCells.insert(n); }

    void clear() { mVisibleCells.clear(); }

    /** Enumerates leafs for given view/orientation
     * @param pos the view position from which we enumerate
     * @param toScreen to-screen projection transform matrix (for the given view
     * position/orientation
     * @param cutPlane near-view cut plane in the direction of the view cuts
     * parts of portals behind camera
     * @param cameraFrustum the frustum to use when cutting the initial cell's
     * portals
     */
    void traverse(const Vector3 &pos, const Matrix4 &toScreen,
                  const Plane &cutPlane,
                  const PortalFrustum &cameraFrustum,
                  bool cleanFirst = true);

protected:
    BspTree *mBspTree;

    /// Cache of screen projected rectangles
    ScreenRectCache mRects;

    /// Serial update id (run id) of the findLeafsForView call
    unsigned int mUpdateID;

    BspNodes mVisibleCells;
};

} // namespace Ogre

#endif
