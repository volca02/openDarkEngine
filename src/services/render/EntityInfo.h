/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
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

#ifndef __ENTITYINFO_H
#define __ENTITYINFO_H

#include "integers.h"
#include "Vector3.h"

namespace Ogre {
class SceneManager;
class Entity;
class SceneNode;
} // namespace Ogre

class EntityMaterialInstance;

namespace Opde {

/** Entity property realization object. A package of an entity and a
 * EntityMaterialInstance. Realizes all the per-object rendering related
 * properties. All the property handlers(of RenderedProperty class) use this
 * class to make the property values visible. */
class EntityInfo {
public:
    EntityInfo(Ogre::SceneManager *man, Ogre::Entity *entity,
               Ogre::SceneNode *node);

    // destructor - destroys the
    ~EntityInfo();

    // setters:
    void setHasRefs(bool _hasRefs);
    void setRenderType(unsigned int _renderType);
    void setSkip(bool _skip);
    void setAlpha(float alpha);
    void setZBias(size_t bias);
    void setScale(const Vector3 &scale);

    void setEntity(Ogre::Entity *newEntity);

    inline Ogre::Entity *getEntity(void) { return mEntity; };
    inline Ogre::SceneNode *getSceneNode(void) { return mNode; };

    /// refreshes the visibilty of the object based on hasRefs, renderType and
    /// skip
    void refreshVisibility();

protected:
    Ogre::SceneManager *mSceneMgr;

    unsigned int mRenderType;

    bool mHasRefs;

    /// Used by FX_Particle and such. Hides without interfering with the
    /// previous two
    bool mSkip;

    /// alpha transparency value of the object
    float mAlpha;

    /// z-bias of the object
    float mZBias;

    Ogre::Entity *mEntity;
    Ogre::SceneNode *mNode;
    std::unique_ptr<EntityMaterialInstance> mEmi;
};

} // namespace Opde

#endif /* __ENTITYINFO_H */
