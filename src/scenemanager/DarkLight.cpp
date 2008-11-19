/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 * http://www.gnu.org/copyleft/lesser.txt.
 *
 *
 *	$Id$
 *
 *****************************************************************************/


#include "DarkLight.h"
#include "DarkBspTree.h"
#include "DarkSceneManager.h"

namespace Ogre {

	// -----------------------------------------------------------
	DarkLight::DarkLight() : Light() {
		mNeedsUpdate = true;
		mIsDynamic = false;
	}

	// -----------------------------------------------------------
	DarkLight::DarkLight(const String& name) : Light(name) {
		mNeedsUpdate = true;
	}

	// -----------------------------------------------------------
	DarkLight::~DarkLight() {
		_clearAffectedCells();
	}

	// -----------------------------------------------------------
	const String& DarkLight::getMovableType(void) {
		return DarkLightFactory::FACTORY_TYPE_NAME;
	}

	// -----------------------------------------------------------
	void DarkLight::_notifyMoved(void) {
		Light::_notifyMoved();

		_updateNeeded();
	}

	// -----------------------------------------------------------
	void DarkLight::_notifyAttached(Node* parent, bool isTagPoint) {
		Light::_notifyAttached(parent, isTagPoint);

		_updateNeeded();
	}

	// -----------------------------------------------------------
	void DarkLight::_updateAffectedCells(void) {
		// Because there is no way that would inform us about a change of parameters, I just ignore
		if (!mNeedsUpdate)
			return;

		mNeedsUpdate = false;

		Vector3 position = getDerivedPosition();

		// only dynamic lights are allowed inside. Static lights have the affected BSP leaves hard-coded.
		if (!mIsDynamic)
					return;

		_clearAffectedCells();

		// Process the affected cell list.
		// For this, we simply recursively check the portal frustum intersections,
		// up to the distance of light's bounding radius

		// This is slower than on-screen check, but lights don't move that much, and we need precision here.
		// Also, omni can't be solved through projection...

		// For omnidirectional lights, the root cell emits frustums for all portals
		// For spot lights, the root cell only emits frustums for those portals which are visible through the light's frustum

		BspTree *t = static_cast<DarkSceneManager*>(mManager)->getBspTree();

		//Vector3 position = getPosition();

		BspNode* rootNode = t->findLeaf(position);

		if (rootNode != NULL) {
			mAffectedCells.insert(rootNode);

			// We're in the world
			// Update the cell list
			LightTypes lt = getType();

			if (lt == Light::LT_POINT || lt == Light::LT_SPOTLIGHT) {
				// omni/point light. Create frustum for every portal in reach, traverse with that frustum
				Real rad = getAttenuationRange();

				BspNode::PortalIterator pit = rootNode->outPortalBegin();
				BspNode::PortalIterator pend = rootNode->outPortalEnd();

				PortalFrustum *slf = NULL;

				// Create a frustum for the spotlight (Reduces the portals which are affected)
				if (lt == Light::LT_SPOTLIGHT) {
				    // More planes, more processing (4 should be good aproximation)
				    slf = new PortalFrustum(position, getDirection(), getSpotlightOuterAngle(), 4);
				}

				for (; pit != pend; ++pit) {
					// Backface cull...
					if ((*pit)->isBackfaceCulledFor(position))
						continue;

					if ((*pit)->getDistanceFrom(position) > rad && rad > 0)
						continue;

					// TODO: Create a test mission that shows off if the cone is detected right (N portals, one in spotlight's cone)
					if (lt == Light::LT_SPOTLIGHT) {
					    // Spotlight...
					    bool didc = false;

					    // Cut the portal with the Spotlight's frustum
					    Portal* cut = slf->clipPortal(*pit, didc);

						if (cut != NULL) {
							// create a portalFrustum to traverse with
							// For spotlight, it covers the intersection of the light's
							// aproximated cone and the portal
							PortalFrustum f = PortalFrustum(position, cut);

							_traversePortalTree(f, cut, rootNode, 0);

							if (didc)
								delete cut;
						}
					} else {
					    // create a portalFrustum to traverse with
					    // For POINT light, it covers the whole portal
					    PortalFrustum f = PortalFrustum(position, *pit);

					    _traversePortalTree(f, *pit, rootNode, 0);
					}
				}

				// Delete the spotlight's frustum if it was used
				delete slf;

			} else {
				// Nothing to do, directional lights are not supported
			}
		}

		// After the cell list has been built, inform the affected cells
		BspNodeSet::iterator it = mAffectedCells.begin();
		BspNodeSet::iterator end = mAffectedCells.end();

		for (; it != end; ++it) {
			(*it)->addAffectingLight(this);
		}
	}

	// -----------------------------------------------------------
	void DarkLight::_clearAffectedCells(void) {
		BspNodeSet::iterator it = mAffectedCells.begin();
		BspNodeSet::iterator end = mAffectedCells.end();

		for (; it != end; ++it) {
			(*it)->removeAffectingLight(this);
		}

		mAffectedCells.clear();
	}

	// -----------------------------------------------------------
	void DarkLight::affectsCell(BspNode* leaf) {
		mAffectedCells.insert(leaf);
		leaf->addAffectingLight(this);
	}

	// -----------------------------------------------------------
	void DarkLight::_traversePortalTree(PortalFrustum& frust, Portal* p, BspNode* srcCell, Real dist) {
		BspNode* tgt = p->getTarget();
		Vector3 pos = getDerivedPosition();
		Real radius = getAttenuationRange();

		if (tgt != NULL) {
			mAffectedCells.insert(tgt);

			BspNode::PortalIterator pit = tgt->outPortalBegin();
			BspNode::PortalIterator pend = tgt->outPortalEnd();

			for (; pit != pend; ++pit) {
				// If facing towards the light
				if ((*pit)->isBackfaceCulledFor(pos))
					continue;

				Real cdist = (*pit)->getDistanceFrom(pos);

				if (cdist < 0)
					continue;

				if (cdist < dist)
					continue;

				// and not too far to reach
				if (cdist > radius)
					continue;

				// and not going back to the source cell (X->n->X traversal)
				if (srcCell == (*pit)->getTarget())
					continue;

				bool didc = false;

				// TODO: The frustum could be constructable directly from frustum and portal
				// This would mean no additional needed steps besides constructing a new frustum
				Portal* cut = frust.clipPortal(*pit, didc);

				if (cut != NULL) {
					// create a portalFrustum to traverse with
					PortalFrustum f = PortalFrustum(pos, cut);

					_traversePortalTree(f, *pit, tgt, cdist);

					if (didc)
						delete cut;
				}
			}
		}
	}

	// -----------------------------------------------------------
	void DarkLight::_updateNeeded(void) {
		mNeedsUpdate = true;

		// Just to be sure
		static_cast<DarkSceneManager*>(mManager)->_queueLightForUpdate(this);
	}

	// -----------------------------------------------------------
	void DarkLight::setIsDynamic(bool dynamic) {
		mIsDynamic = dynamic;

		if (mIsDynamic) // guarantee consistency
			_updateNeeded();
	}

	// -----------------------------------------------------------
	// -----------------------------------------------------------
	const String DarkLightFactory::FACTORY_TYPE_NAME = "DarkLight";

	// -----------------------------------------------------------
	const String& DarkLightFactory::getType(void) const {
		return FACTORY_TYPE_NAME;
	}

	// -----------------------------------------------------------
	void DarkLightFactory::destroyInstance(MovableObject* obj) {
		delete obj;
	}

	// -----------------------------------------------------------
	MovableObject * DarkLightFactory::createInstanceImpl(const String& name, const NameValuePairList* params) {

		return new DarkLight(name);

	}

}
