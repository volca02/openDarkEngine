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

#include "DarkPortalTraversal.h"
#include "DarkBspTree.h"
#include "tracer.h"

namespace Ogre {

void DarkPortalTraversal::traverse(const Vector3 &pos,
                                   const Matrix4 &toScreen,
                                   const Plane &cutPlane,
                                   const PortalFrustum &cameraFrustum,
                                   bool cleanFirst)
{
    TRACE_METHOD;

    // this effectively invalidates the screen space info for all cells/portals
    ++mUpdateID;

    mRects.startUpdate(mBspTree->getPortalCount(), mBspTree->getCellCount(),
                       mUpdateID);

    // Look for our root cell
    BspNode *root = mBspTree->findLeaf(pos);

    if (root == NULL) // out of world
        return;

    // invalidate the associated screen rect
    mRects.invalidateCell(root->getID(), mUpdateID);

    // Root cell gets whole screen visibility
    root->refreshScreenRect(pos, mRects, toScreen, cameraFrustum);

    // the view rect is set to fill the screen
    mRects.cell(root->getID()).rect.setToScreen();

    // Root exists. traverse the portal tree
    BspNodeQueue cell_queue;
    cell_queue.reserve(1024);
    cell_queue.push_back(root);

    unsigned int finishedCells = 0;

    if (cleanFirst) {
        mVisibleCells.clear();
        mVisibleCells.reserve(1024);
    }

    mVisibleCells.insert(root);

    TRACE_SCOPE_OBJ(CELL_ITER, this);
    while (finishedCells < cell_queue.size()) {
        BspNode *cell = cell_queue[finishedCells++];

        if (cell == NULL)
            continue;

        if (cell->isVisBlocked())
            continue;

        // (Re-)evaluate the vision through all portals. If some of those
        // changed, add the cells to the queue (or move those to top if already
        // present)

        CellRectInfo &ci(mRects.cell(cell->getID()));

        for (const Portal * p : cell->outPortals()) { // for all portals
            PortalRectInfo &pinfo(mRects.portal(p->getID()));

            // Backface cull
            if (pinfo.portalCull)
                continue;

            PortalRect tgt;
            bool changed = false;

            BspNode *target_cell = p->getTarget();
            CellRectInfo &tgtinfo = mRects.cell(target_cell->getID());

            // Be sure to have the cell rect reset if it was invalid
            if (tgtinfo.updateID != mUpdateID)
                tgtinfo.invalidate(mUpdateID);

            // is there any intersection?
            if (pinfo.intersect(ci.rect, tgt)) {
                if (pinfo.unionActualWith(tgt)) {
                    changed = tgtinfo.updateScreenRect(tgt);
                }
            }

            // If a change happened, update the queue
            // Possible situation is that two cells are connected with two
            // portals In that case, we would end up having the cell added/moved
            // twice Because of this, the mInitialized flag exists, that
            // differentiates the state So the addition will update the frame
            // num, and the next portal will detect the position is > current,
            // and do nothing or, in case the cell was in queue already, but
            // moved, the queue position is then > current, and nothing is done
            if (changed) {
                // Update the queue
                if (!tgtinfo.initialized) {
                    target_cell->refreshScreenRect(pos, mRects, toScreen,
                                                   cutPlane);

                    // insert to the top
                    cell_queue.push_back(target_cell);
                    tgtinfo.listPosition = cell_queue.size() - 1;

                    mVisibleCells.insert(target_cell);
                } else {
                    // move to the top if below current position
                    if (finishedCells > tgtinfo.listPosition) {
                        // invalidate the current position, move to the new one
                        cell_queue.push_back(target_cell);
                        cell_queue[tgtinfo.listPosition] =
                            NULL; // NULL means a moved item (so we need not
                                  // reorganize)
                        tgtinfo.listPosition = cell_queue.size() - 1;
                    }
                }
            } // if changed
        }     // for all portals
    }
}

}
