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
 *
 *	  $Id$
 *
 *****************************************************************************/

#ifndef __FREESPACEINFO_H
#define __FREESPACEINFO_H

#include <memory>

namespace Opde {

/** Free space information storage - rectangular area in the lightmap (either
 * used or free to use) Organized in a binary tree. - A sort of a 2D texture
 * space allocator. thanks to this article for a tip:
 * http://www.blackpawn.com/texts/lightmaps/default.html
 */
class FreeSpaceInfo {
protected:
    int mMaxArea;
    bool mIsLeaf;

    // Children of this node, if it's a node
    std::unique_ptr<FreeSpaceInfo> mChild[2];

    FreeSpaceInfo() {
        this->x = 0;
        this->y = 0;
        this->w = -1;
        this->h = -1;

        mMaxArea = 0;
        mIsLeaf = true;
    }

public:
    int x;
    int y;
    int w;
    int h;

    // Constructor with the specified dimensions and position (leaf constructor)
    FreeSpaceInfo(int x, int y, int w, int h) {
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;

        mIsLeaf = true;
        mMaxArea = w * h;
    }

    // destructor. Deletes the children if any.
    ~FreeSpaceInfo() { }

    // Tests if this node has free space somewhere to store the specified sized
    // texture.
    bool Fits(int sw, int sh) const {
        if ((sw <= w) && (sh <= h))
            return true;

        return false;
    }

    // Returns the area this free space represents
    int getArea() const { return w * h; }

    /** returns the maximal allocatable area of this node/leaf */
    int getMaxArea() const { return mMaxArea; }

    /** allocate a space in the free area.
     * If this is a node, it searches both it's children. If it is a leaf, it
     * either returns NULL if it can't hold the specified dimensions, or a new
     * pointer to a leaf that holds the allocation requested (the space that was
     * not taken up by the alocation is put as a child of the node)
     * @return A free space rectangle of the requested space, or null if the
     * space could not be allocated */
    FreeSpaceInfo *allocate(int sw, int sh) {
        if (!mIsLeaf) { // split node.
            int reqa = sw * sh;

            for (int i = 0; i < 2; i++) {
                FreeSpaceInfo *result = nullptr;

                if (!mChild[i])
                    continue;

                if (mChild[i]->getMaxArea() >= reqa)
                    result = mChild[i]->allocate(sw, sh);

                if (result != NULL) { // allocation was ok
                    // refresh the maximal area
                    refreshMaxArea();

                    return result;
                }
            }

            return NULL; // no luck, sorry!
        } else {
            // we're the leaf node. Try to insert if possible
            if (!Fits(sw, sh))
                return NULL;

            // bottom will be created?
            if (sh < h)
                mChild[0].reset(new FreeSpaceInfo(x, y + sh, w, h - sh));

            // right will be created?
            if (sw < w)
                mChild[1].reset(new FreeSpaceInfo(x + sw, y, w - sw, sh));

            // modify this node to be non-leaf, as it was allocated
            mIsLeaf = false;

            w = sw;
            h = sh;

            // refresh the mMaxArea
            refreshMaxArea();

            return this;
        }
    }

    /** refreshes the maximal allocatable area for this node/leaf.
        Non-leaf nodes get the maximum as the maximum of the areas of the children
    */
    void refreshMaxArea() {
        if (mIsLeaf) {
            mMaxArea = w * h;
            return;
        }

        mMaxArea = -1;

        for (int j = 0; j < 2; j++) {
            if (!mChild[j])
                continue;

            if (mChild[j]->getMaxArea() > mMaxArea)
                mMaxArea = mChild[j]->getMaxArea();
        }
    }

    /** returns the maximal area of this node */
    int getLeafArea() {
        if (mIsLeaf) {
            return getArea();
        }

        int area = 0;

        for (int j = 0; j < 2; j++) {
            if (!mChild[j])
                continue;

            area += mChild[j]->getLeafArea();
        }

        return area;
    }
};

} // namespace Opde
#endif
