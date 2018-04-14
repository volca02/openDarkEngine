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

#include "DarkSceneNode.h"
#include "DarkSceneManager.h"

#include <OgreSceneManager.h>

namespace Ogre {
//-------------------------------------------------------------------------
DarkSceneNode::DarkSceneNode(SceneManager *creator) : SceneNode(creator) {}

//-------------------------------------------------------------------------
DarkSceneNode::DarkSceneNode(SceneManager *creator, const String &name)
    : SceneNode(creator, name) {}

//-------------------------------------------------------------------------
void DarkSceneNode::_update(bool updateChildren, bool parentHasChanged) {
    bool checkMovables = false;

    if (mNeedParentUpdate || parentHasChanged) {
        // This means we've moved
        checkMovables = true;
    }

    // Call superclass
    SceneNode::_update(updateChildren, parentHasChanged);

    if (checkMovables) {
        // Check membership of attached objects
        ObjectMap::const_iterator it, itend;
        itend = mObjectsByName.end();
        for (it = mObjectsByName.begin(); it != itend; ++it) {
            MovableObject *mov = it->second;

            static_cast<DarkSceneManager *>(mCreator)->_notifyObjectMoved(
                mov, this->_getDerivedPosition());
        }
    }
}

//-------------------------------------------------------------------------
MovableObject *DarkSceneNode::detachObject(unsigned short index) {
    MovableObject *ret = SceneNode::detachObject(index);
    static_cast<DarkSceneManager *>(mCreator)->_notifyObjectDetached(ret);
    return ret;
}

//-------------------------------------------------------------------------
MovableObject *DarkSceneNode::detachObject(const String &name) {
    MovableObject *ret = SceneNode::detachObject(name);
    static_cast<DarkSceneManager *>(mCreator)->_notifyObjectDetached(ret);
    return ret;
}

//-------------------------------------------------------------------------
void DarkSceneNode::detachAllObjects(void) {
    ObjectMap::const_iterator i, iend;
    iend = mObjectsByName.end();
    for (i = mObjectsByName.begin(); i != iend; ++i) {
        static_cast<DarkSceneManager *>(mCreator)->_notifyObjectDetached(
            i->second);
    }
    SceneNode::detachAllObjects();
}

//-------------------------------------------------------------------------
void DarkSceneNode::setInSceneGraph(bool inGraph) {
    if (mIsInSceneGraph != inGraph) {
        ObjectMap::const_iterator i, iend;
        iend = mObjectsByName.end();
        for (i = mObjectsByName.begin(); i != iend; ++i) {
            if (!inGraph) {
                // Equivalent to detaching
                static_cast<DarkSceneManager *>(mCreator)
                    ->_notifyObjectDetached(i->second);
            } else {
                // move deals with re-adding
                static_cast<DarkSceneManager *>(mCreator)->_notifyObjectMoved(
                    i->second, this->_getDerivedPosition());
            }
        }
    }

    SceneNode::setInSceneGraph(inGraph);
}

} // namespace Ogre
