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

#ifndef __DARKLIGHT_H
#define __DARKLIGHT_H

#include "config.h"

#include "DarkBspPrerequisites.h"
#include <OgreLight.h>

namespace Ogre {

/** Specialized version of Ogre::Light that caches cells it affects */
class OPDELIB_EXPORT DarkLight : public Light {
public:
    DarkLight();

    DarkLight(const String &name);

    ~DarkLight();

    const String &getMovableType(void);

    virtual void _notifyMoved(void);

    virtual void _notifyAttached(Node *parent, bool isTagPoint);

    /// Refreshes the list of cells affected by this light
    void _updateAffectedCells(void);

    /// Clears the list of affected lights, unregisters itself from the cells it
    /// affected
    void _clearAffectedCells(void);

    /// adds a cell affection (the light affects the given BSP leaf)
    void affectsCell(BspNode *leaf);

    /// Called by SceneManager after a call to
    /// DarkSceneManager::queueLightForUpdate(); Causes the light to be
    /// reevaluated in terms of coverage before using it for rendering
    virtual void _updateNeeded(void);

    /// toggles the dynamic flag for this light
    void setIsDynamic(bool dynamic);

    /// Returns true for the dynamic lights
    bool isDynamic() const { return mIsDynamic; };

protected:
    /// Traverses the portal tree taking the given portal as the starting point
    /// (root), inserts encountered cells into mAffectedCells
    void _traversePortalTree(PortalFrustum &frust, Portal *p, BspNode *srcCell,
                             Real dist);

    BspNodeSet mAffectedCells;

    bool mNeedsUpdate;

    /// true for dynamic lights - those which update their sphere of influence
    /// dynamically
    bool mIsDynamic;
};

class OPDELIB_EXPORT DarkLightFactory : public MovableObjectFactory {
protected:
    MovableObject *createInstanceImpl(const String &name,
                                      const NameValuePairList *params);

public:
    DarkLightFactory(){};
    ~DarkLightFactory(){};

    static const String FACTORY_TYPE_NAME;

    const String &getType(void) const;

    virtual void destroyInstance(MovableObject *obj);
};
}; // namespace Ogre

#endif
