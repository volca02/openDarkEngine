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

#include "EntityInfo.h"
#include "RenderCommon.h"
#include "RenderTypeProperty.h"
#include "EntityMaterialInstance.h"

#include <OgreEntity.h>

namespace Opde {

/*--------------------------------------------------------*/
/*--------------------- EntityInfo -----------------------*/
/*--------------------------------------------------------*/
EntityInfo::EntityInfo(Ogre::SceneManager *man, Ogre::Entity *entity,
                       Ogre::SceneNode *node)
    : mSceneMgr(man),
      mRenderType(RENDER_TYPE_NORMAL),
      mHasRefs(true),
      mSkip(false),
      mAlpha(1.0f),
      mZBias(0.0f),
      mEntity(entity),
      mNode(node),
      mEmi(new EntityMaterialInstance(mEntity))
{
    // TODO: This causes some serious perf. issues, and should not be needed!
    mEmi->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
    // mEmi->setSceneBlending(SBT_MODULATE);
};

// --------------------------------------------------------------------------
EntityInfo::~EntityInfo() {
    //
    mNode->detachObject(mEntity);

    mEmi.reset();

    mSceneMgr->destroyEntity(mEntity);

    mSceneMgr->destroySceneNode(mNode->getName());
}

// --------------------------------------------------------------------------
void EntityInfo::setHasRefs(bool _hasRefs) {
    mHasRefs = _hasRefs;
    refreshVisibility();
};

// --------------------------------------------------------------------------
void EntityInfo::setRenderType(unsigned int _renderType) {
    mRenderType = _renderType;
    refreshVisibility();
};

// --------------------------------------------------------------------------
void EntityInfo::setSkip(bool _skip) {
    mSkip = _skip;
    refreshVisibility();
};

// --------------------------------------------------------------------------
void EntityInfo::setAlpha(float alpha) {
    mAlpha = alpha;
    mEmi->setTransparency(1.0f - mAlpha);
};

// --------------------------------------------------------------------------
void EntityInfo::setZBias(size_t bias) {
    mZBias = bias;
    mEmi->setZBias(bias);
};

// --------------------------------------------------------------------------
void EntityInfo::setScale(const Vector3 &scale) { mNode->setScale(scale); };

// --------------------------------------------------------------------------
void EntityInfo::setEntity(Ogre::Entity *entity) {
    if (mEntity == entity)
        return;

    // detach the old entity
    mNode->detachObject(mEntity);

    // attach the new entity
    mNode->attachObject(entity);

    mEmi->setEntity(entity);

    // destroy the previous entity
    mSceneMgr->destroyEntity(mEntity);

    mEntity = entity;

    refreshVisibility();
};

// --------------------------------------------------------------------------
void EntityInfo::refreshVisibility() {
    // calculate the visibilities:
    bool brType = true;

    if (mRenderType == RENDER_TYPE_NOT_RENDERED)
        brType = false;

    // todo: editor mode handling

    mNode->setVisible(!mSkip && mHasRefs && brType, true);
};


} // namespace Opde
