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
    // Children of this node, if it's a node
    std::unique_ptr<FreeSpaceInfo> mChild[2];

    FreeSpaceInfo() {
        this->x = 0;
        this->y = 0;
        this->w = 0;
        this->h = 0;
    }

public:
    int x;
    int y;
    int w;
    int h;
    bool used = false;

    // Constructor with the specified dimensions and position (leaf constructor)
    FreeSpaceInfo(int x, int y, int w, int h) {
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
    }

    // destructor. Deletes the children if any.
    ~FreeSpaceInfo() { }

    // Returns the area this free space represents
    int getArea() const { return w * h; }

    bool isLeaf() const { return !mChild[0]; }

    /** allocate a space in the free area.
     * If this is a node, it searches both it's children. If it is a leaf, it
     * either returns NULL if it can't hold the specified dimensions, or a new
     * pointer to a leaf that holds the allocation requested (the space that was
     * not taken up by the alocation is put as a child of the node)
     * @return A free space rectangle of the requested space, or null if the
     * space could not be allocated */
    FreeSpaceInfo *allocate(int sw, int sh) {
        if (!isLeaf()) { // split node.
            FreeSpaceInfo *result = mChild[0]->allocate(sw, sh);
            if (result) return result;
            return mChild[1]->allocate(sw, sh);
        }


        if (used) return nullptr;

        // we're the leaf node. Try to insert if possible
        if ((sw > w) || (sh > h))
            return nullptr;

        // exact fit. Return this node as result
        if ((w == sw) && (h == sh)) {
            used = true;
            return this;
        }

        // we have to split this into two leafs.
        int dw = w - sw;
        int dh = h - sh;

        // choose split based on bigger delta
        if (dw > dh) {
            mChild[0].reset(new FreeSpaceInfo(x,      y,     sw, h));
            mChild[1].reset(new FreeSpaceInfo(x + sw, y, w - sw, h));
        } else {
            mChild[0].reset(new FreeSpaceInfo(x, y,      w,     sh));
            mChild[1].reset(new FreeSpaceInfo(x, y + sh, w, h - sh));
        }

        return mChild[0]->allocate(sw, sh);
    }

    /** returns the allocated area of this subtree */
    int getLeafArea() {
        if (isLeaf()) return w*h;

        int area = mChild[0]->getLeafArea() + mChild[1]->getLeafArea();
        return area;
    }
};

} // namespace Opde
#endif
