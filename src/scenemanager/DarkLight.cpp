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

#include "DarkLight.h"
#include "DarkBspTree.h"
#include "DarkSceneManager.h"

#include "tracer.h"

namespace Ogre {

// -----------------------------------------------------------
DarkLight::DarkLight(BspTree *tree)
    : Light(), mNeedsUpdate(true), mTraversal(tree), mIsDynamic(false)
{
}

// -----------------------------------------------------------
DarkLight::DarkLight(BspTree *tree, const String &name)
    : Light(name), mNeedsUpdate(true), mTraversal(tree), mIsDynamic(false)
{
}

// -----------------------------------------------------------
DarkLight::~DarkLight() { _clearAffectedCells(); }

// -----------------------------------------------------------
const String &DarkLight::getMovableType(void) {
    return DarkLightFactory::FACTORY_TYPE_NAME;
}

// -----------------------------------------------------------
void DarkLight::_notifyMoved(void) {
    Light::_notifyMoved();
    _updateNeeded();
}

// -----------------------------------------------------------
void DarkLight::_notifyAttached(Node *parent, bool isTagPoint) {
    Light::_notifyAttached(parent, isTagPoint);
    _updateNeeded();
}

// -----------------------------------------------------------
void DarkLight::_updateAffectedCells(void) {
    TRACE_METHOD;

    // Because there is no way that would inform us about a change of
    // parameters, I just ignore
    if (!mNeedsUpdate)
        return;

    mNeedsUpdate = false;

    // only dynamic lights are allowed inside. Static lights have the affected
    // BSP leaves hard-coded.
    if (!mIsDynamic)
        return;

    _clearAffectedCells();

    // Process the affected cell list.
    // For this, we simply recursively check the portal frustum intersections,
    // up to the distance of light's bounding radius

    // This is slower than on-screen check, but lights don't move that much, and
    // we need precision here. Also, omni can't be solved through projection...

    // For omnidirectional lights, the root cell emits frustums for all portals
    // For spot lights, the root cell only emits frustums for those portals
    // which are visible through the light's frustum

    BspTree *t = static_cast<DarkSceneManager *>(mManager)->getBspTree();

    static const Vector3 sDirections[6] = {
        Vector3::UNIT_X,
        Vector3::NEGATIVE_UNIT_X,
        Vector3::UNIT_Y,
        Vector3::NEGATIVE_UNIT_Y,
        Vector3::UNIT_Z,
        Vector3::NEGATIVE_UNIT_Z
    };

    static const Vector3 sScale(1.0, 1.0, 1.0);
    Vector3 position = getDerivedPosition();

    // We're in the world
    // Update the cell list
    LightTypes lt = getType();

    if (lt == Light::LT_POINT) {
        // omni/point light. Create frustum for every portal in reach,
        // traverse with that frustum
        Real rad = getAttenuationRange();

        // For all 6 sides of the view cube of the light
        for (unsigned dir = 0; dir < 6; ++dir) {
            // traverse the scene in this direction
            // TODO: What is the rotation of the frustum planes? It may be incorrect!
            auto direction = sDirections[dir];

            PortalFrustum frustum(position, direction,
                                  Radian(Math::HALF_PI), 4);

            // view matrix to transform to view space. has to be created from position and direction
            Matrix4 viewM;
            viewM.makeTransform(position, sScale, Quaternion(Radian(0), direction));
            /* TODO: PERSPECTIVE projection matrix, 90 degrees. Can be fixed, probably
            const Matrix4 &projM = getProjectionMatrix(); 
            Matrix4 toScreen = projM * viewM;
            mTraveral.traverse(position, toScreen, cutPlane, frustum, false);
            */
        }
    } else if (lt == Light::LT_SPOTLIGHT) {
        auto direction = getDirection();

        // More planes, more processing (4 should be good aproximation)
        PortalFrustum frustum(position, getDirection(), getSpotlightOuterAngle(), 4);

        Plane cutPlane(position, direction,
                       position + direction * 0.01f);

        /* TODO:
        // view matrix - depends on position and view direction
        const Matrix4 &viewM = getViewMatrix();
        // projection matrix. Could be parametrized by outer angle of the spotlight
        const Matrix4 &projM = getProjectionMatrix();
        Matrix4 toScreen = projM * viewM;

        mTraveral.traverse(position, toScreen, cutPlane, frustum);*/
    } else {
        // Nothing to do, directional lights are not supported
    }

    // After the cell list has been built, inform the affected cells
    for (BspNode *n : mTraversal.visibleCells()) {
        n->addAffectingLight(this);
    }
}

// -----------------------------------------------------------
void DarkLight::_clearAffectedCells(void) {
    for (BspNode *n : mTraversal.visibleCells()) {
        n->removeAffectingLight(this);
    }

    mTraversal.clear();
}

// -----------------------------------------------------------
void DarkLight::affectsCell(BspNode *leaf) {
    mTraversal.addCell(leaf);
    leaf->addAffectingLight(this);
}

// -----------------------------------------------------------
void DarkLight::_updateNeeded(void) {
    mNeedsUpdate = true;

    // Just to be sure
    if (mIsDynamic) // guarantee consistency
        static_cast<DarkSceneManager *>(mManager)->_queueLightForUpdate(this);
}

// -----------------------------------------------------------
void DarkLight::setIsDynamic(bool dynamic) {
    mIsDynamic = dynamic;
    _updateNeeded();
}

// -----------------------------------------------------------
// -----------------------------------------------------------
const String DarkLightFactory::FACTORY_TYPE_NAME = "DarkLight";

// -----------------------------------------------------------
const String &DarkLightFactory::getType(void) const {
    return FACTORY_TYPE_NAME;
}

// -----------------------------------------------------------
void DarkLightFactory::destroyInstance(MovableObject *obj) { delete obj; }

// -----------------------------------------------------------
MovableObject *
DarkLightFactory::createInstanceImpl(const String &name,
                                     const NameValuePairList *params) {

    return new DarkLight(mSceneMgr->getBspTree(), name);
}

} // namespace Ogre
