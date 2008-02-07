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
		
		_clearAffectedCells();
		
		/*if (!isInScene())
			return;*/
		
		// Process the affected cell list.
		// For this, we simply recursively check the portal frustum intersections, 
		// up to the distance of light's bounding radius 
		
		// This is slower than on-screen check, but lights don't move that much, and we need precision here.
		// Also, omni can't be solved through projection...
		
		// For omnidirectional lights, the root cell emits frustums for all portals
		// For spot lights, the root cell only emits frustums for those portals which are visible through the light's frustum
		
		BspTree *t = static_cast<DarkSceneManager*>(mManager)->getBspTree();
		
		BspNode* rootNode = t->findLeaf(getDerivedPosition());
		
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
				    slf = new PortalFrustum(getDerivedPosition(), getDirection(), getSpotlightOuterAngle(), 4); 
				}
				
				for (; pit != pend; ++pit) {
					// Backface cull...
					if ((*pit)->isBackfaceCulledFor(getDerivedPosition()))
						continue;
						
					if ((*pit)->getDistanceFrom(getDerivedPosition()) > rad)
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
							PortalFrustum f = PortalFrustum(getDerivedPosition(), cut);
					
							_traversePortalTree(f, cut, rad);
					
							if (didc)
								delete cut;
						}
					} else { 
					    // create a portalFrustum to traverse with 
					    // For POINT light, it covers the whole portal
					    PortalFrustum f = PortalFrustum(mPosition, *pit);
					
					    _traversePortalTree(f, *pit, rad);
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
	void DarkLight::_traversePortalTree(PortalFrustum& frust, Portal* p, Real radius) {
		BspNode* tgt = p->getTarget();
		
		std::cout << "TPT " << getName() << " " << tgt->getCellNum() << std::endl;
				
		if (tgt != NULL) {
			mAffectedCells.insert(tgt);
			
			BspNode::PortalIterator pit = tgt->outPortalBegin();
			BspNode::PortalIterator pend = tgt->outPortalEnd();
			
			for (; pit != pend; ++pit) {
				// If facing towards the light
				if ((*pit)->isBackfaceCulledFor(getDerivedPosition()))
					continue;
					
				if ((*pit)->getDistanceFrom(getDerivedPosition()) > radius)
					continue;

				// TODO: check if the distance is in radius limit

				bool didc = false;
				
				Portal* cut = frust.clipPortal(*pit, didc);
				
				if (cut != NULL) {
					// create a portalFrustum to traverse with
					PortalFrustum f = PortalFrustum(getDerivedPosition(), cut);
					
					_traversePortalTree(f, *pit, radius);
					
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
