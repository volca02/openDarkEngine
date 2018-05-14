/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option) any
later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc., 59
Temple Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------

Rewritten to be used in the openDarkEngine project by Filip Volejnik
<f.volejnik@centrum.cz>

$Id$

*/

#ifndef __BSPTREE_H
#define __BSPTREE_H

#include "config.h"

#include "DarkBspNode.h"
#include "DarkBspPrerequisites.h"

#include <OgreResource.h>
#include <OgreSceneManager.h>

namespace Ogre {

/** BspTree handling class */
class BspTree : public Ogre::MovableObject::Listener {
    friend class DarkSceneManager;

public:
    /** Constructor. */
    BspTree(DarkSceneManager *owner);

    /** Destructor. Does unallocate the BSP tree (e.g. leaf and non-leaf node
     * list) */
    ~BspTree();

    /** Returns a pointer to the root node (BspNode) of the BSP tree. */
    const BspNode *getRootNode(void);

    /** Sets a new root of the BSP tree */
    void setRootNode(BspNode *node);

    /** Sets a new root of the BSP tree */
    void setRootNode(int id);

    /** Walks the entire BSP tree and returns the leaf
        which contains the given point.
    */
    BspNode *findLeaf(const Vector3 &point) const;

    /** Ensures that the MovableObject is attached to the right leaves of the
        BSP tree.
    */
    void _notifyObjectMoved(const MovableObject *mov, const Vector3 &pos);

    /** Internal method, makes sure an object is removed from the leaves when
     * detached from a node. */
    void _notifyObjectDetached(const MovableObject *mov);

    /** Creates a new BSP node
    * @param id The BSP node id
    * @param leafID the leaf id of this node (if < 0 or unfilled, the node is
    not a leaf, otherwise it is marked as leaf and registered in mLeafNodeMap)
  */
    BspNode *createNode(int id, int leafID = -1);

    /** Gets the BSP node by id */
    BspNode *getNode(int id);

    /** Gets a BSP leaf node by a leaf id */
    BspNode *getLeafNode(int leafid);

    /** clears the BSP tree */
    void clear();

    /** finds leaf nodes the given sphere touches */
    void findLeafsForSphere(BspNodeList &destList, const Vector3 &pos,
                            Real radius);

    /** Listener's callback that returns light list of the movable object */
    const LightList *objectQueryLights(const MovableObject *movable) override;

    /** Listener's callback - flushes light list */
    void objectDestroyed(MovableObject *movable) override;

    unsigned int getPortalCount() const;
    unsigned int getCellCount() const;

protected:
    void findLeafsForSphereFromNode(BspNode *node, BspNodeList &destList,
                                    const Vector3 &pos, Real radius);

    void flushLightCacheForMovable(const MovableObject *movable);

    void populateLightListForMovable(const MovableObject *movable,
                                     LightList &destList);

    /** Root Bsp Node
     */
    BspNode *mRootNode;

    typedef std::map<int, BspNode *> BspNodeMap;

    /// Map of the BSP nodes
    BspNodeMap mBspNodeMap;

    /// Map leaf ID -> BspNode
    BspNodeMap mLeafNodeMap;

    /// Owner of the BSP tree
    DarkSceneManager *mOwner;

    typedef std::unordered_map<const MovableObject *, std::vector<BspNode *>>
        MovableToNodeMap;

    // A cache of light lists for object. The entry is removed upon a change to
    // a movable the list is flushed upon a light change (could be made more
    // inteligent later on)
    typedef std::unordered_map<const MovableObject *, LightList>
        MovableLightListCache;

    /// Map for locating the nodes a movable is currently a member of
    MovableToNodeMap mMovableToNodeMap;

    /// Map of lights that affect movables (repopulated as needed)
    MovableLightListCache mMovableLightsCache;

    void tagNodesWithMovable(BspNode *node, const MovableObject *mov,
                             const Vector3 &pos);
};
} // namespace Ogre

#endif
