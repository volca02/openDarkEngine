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


#include "DarkSceneManager.h"
#include "DarkBspTree.h"
#include "DarkCamera.h"
#include "DarkPortal.h"
#include "DarkSceneNode.h"
#include "DarkGeometry.h"

#include <OgreRoot.h>
#include <OgreEntity.h>
#include <OgreTimer.h>

namespace Ogre {

	// ----------------------------------------------------------------------
	DarkSceneManager::DarkSceneManager(const String& instanceName) : SceneManager(instanceName), mFrameNum(1), mActiveGeometry(NULL) {
		mBspTree = new BspTree(this);
		mDarkLightFactory = new DarkLightFactory();

		Root::getSingleton().addMovableObjectFactory(mDarkLightFactory);
	}


	// ----------------------------------------------------------------------
	DarkSceneManager::~DarkSceneManager() {
		// Just to be sure
		clearScene();

		// Delete the bsp tree
		delete mBspTree;

		Root::getSingleton().removeMovableObjectFactory(mDarkLightFactory);

		// and the factory for darklights
		delete mDarkLightFactory;
	}


	// ----------------------------------------------------------------------
	void DarkSceneManager::clearScene(void) {
		// Remove lights first, as those are affecting cells and we'd reference invalid mem otherwise
		destroyAllLights();

		// Destroy all the portals
		PortalList::iterator it = mPortals.begin();

		for (; it != mPortals.end(); ++it) {
			(*it)->detach();
			delete *it;
		}

		mPortals.clear();

		// clear the BSP tree
		mBspTree->clear();

		// clear all the geometries
		destroyAllGeometries();

		// clear the visible cell list for all cameras
		CameraList::iterator cit = mCameras.begin();

		for (; cit != mCameras.end(); ++cit)
			static_cast<DarkCamera*>(cit->second)->mVisibleCells.clear();

		SceneManager::clearScene();
	}


	// ----------------------------------------------------------------------
	void DarkSceneManager::destroyPortal(Portal* portal) {
		PortalList::iterator it = mPortals.find(portal);

		if (it != mPortals.end()) {
			(*it)->detach();
			mPortals.erase(it);

			delete *it;
		}
	}


	// ----------------------------------------------------------------------
	Portal * DarkSceneManager::createPortal(BspNode* src, BspNode* dst, const Plane& plane) {
		Portal* p = new Portal(src, dst, plane);

		mPortals.insert(p);

		p->attach();
		return p;
	}

	// ----------------------------------------------------------------------
	Portal * DarkSceneManager::createPortal(int srcLeafID, int dstLeafID, const Plane& plane) {
		Portal* p = new Portal(getBspLeaf(srcLeafID), getBspLeaf(dstLeafID), plane);

		mPortals.insert(p);

		p->attach();
		return p;
	}

	// ----------------------------------------------------------------------
	void DarkSceneManager::_updateSceneGraph(Camera* cam) {
		// The cam should be updated with the current cell list
		// TODO: Specialize here. Only update the part of the world visible by the camera
		unsigned long startt = Root::getSingleton().getTimer()->getMilliseconds();

		updateDirtyLights();

		SceneManager::_updateSceneGraph(cam);

		mFrameNum++;

		mSceneGraphTime = Root::getSingleton().getTimer()->getMilliseconds() - startt;
	}


	// ----------------------------------------------------------------------
	Camera * DarkSceneManager::createCamera(const String& name) {
		if (mCameras.find(name) != mCameras.end())
		{
				OGRE_EXCEPT(
						Exception::ERR_DUPLICATE_ITEM,
						"A camera with the name " + name + " already exists",
						"DarkSceneManager::createCamera" );
		}

        Camera * c = new DarkCamera( name, this );

        mCameras.insert( CameraList::value_type( name, c ) );

        // Visible objects bounds info for the camera
        mCamVisibleObjectsMap[c] = VisibleObjectsBoundsInfo();


		return c;
	}

	// ----------------------------------------------------------------------
	BspNode* DarkSceneManager::createBspNode(int id, int leafID) {
		BspNode* node = mBspTree->createNode(id, leafID);

		return node;
	}


	// ----------------------------------------------------------------------
	BspNode* DarkSceneManager::getBspNode(int id) {
		return mBspTree->getNode(id);
	}

	// ----------------------------------------------------------------------
	BspNode* DarkSceneManager::getBspLeaf(int leafID) {
		return mBspTree->getLeafNode(leafID);
	}

	// ----------------------------------------------------------------------
	void DarkSceneManager::setRootBspNode(int id) {
		mBspTree->setRootNode(id);
	}

	// ----------------------------------------------------------------------
	SceneNode* DarkSceneManager::createSceneNode(void) {
		DarkSceneNode * sn = new DarkSceneNode( this );
        mSceneNodes[ sn->getName() ] = sn;
		return sn;
	}

	// ----------------------------------------------------------------------
	SceneNode* DarkSceneManager::createSceneNode(const String& name) {
		if (mSceneNodes.find(name) != mSceneNodes.end())
		{
				OGRE_EXCEPT(
						Exception::ERR_DUPLICATE_ITEM,
						"A SceneNode with the name " + name + " already exists",
						"DarkSceneManager::createSceneNode" );
		}

		DarkSceneNode * sn = new DarkSceneNode( this, name );
		mSceneNodes[ sn->getName() ] = sn;
		return sn;
	}

	// ----------------------------------------------------------------------
	void DarkSceneManager::_notifyObjectMoved(const MovableObject* mov, const Vector3& pos) {
			if (mBspTree != NULL) {
					mBspTree->_notifyObjectMoved(mov, pos);
			}
	}

	//-----------------------------------------------------------------------
	void DarkSceneManager::_notifyObjectDetached(const MovableObject* mov) {
			if (mBspTree != NULL) {
					mBspTree->_notifyObjectDetached(mov);
			}
	}

	//-----------------------------------------------------------------------
	void DarkSceneManager::_findVisibleObjects(Camera* cam, VisibleObjectsBoundsInfo* visibleBounds, bool onlyShadowCasters) {
		unsigned long startt = Root::getSingleton().getTimer()->getMilliseconds();

		// Clear the render queue first
		RenderQueue* renderQueue = getRenderQueue();

        renderQueue->clear();
        visibleBounds->reset();

		// Ensure the camera is updated (otherwise we'd have problems if it updated while in the upcoming loop)
		static_cast<DarkCamera*>(cam)->updateFrustum();
		static_cast<DarkCamera*>(cam)->updateView();

		// update the camera's internal visibility list
		static_cast<DarkCamera*>(cam)->updateVisibleCellList();


		MovablesForRendering movablesForRendering; // using a tag (frameNum) would probably be faster. hmm.

		// clear the current visibility list
		DarkCamera* dcam = static_cast<DarkCamera*>(cam);
		BspNodeList::iterator it = dcam->mVisibleCells.begin();
		BspNodeList::iterator end = dcam->mVisibleCells.end();

		// Insert all movables that are visible according to the parameters
		while (it != end) {
			BspNode *node = *(it++);

			// insert the movables of this cell to the movables for rendering
			const BspNode::IntersectingObjectSet& objects = node->getObjects();
			BspNode::IntersectingObjectSet::const_iterator oi, oiend;

			oiend = objects.end();
			for (oi = objects.begin(); oi != oiend; ++oi) {
				if (movablesForRendering.find(*oi) == movablesForRendering.end())
				{
					// It hasn't been rendered yet
					MovableObject *mov = const_cast<MovableObject*>(*oi); // hacky

					if (mov == cam)
						continue;


					if (mov->isVisible() &&
						(!onlyShadowCasters || mov->getCastShadows())) { //  && cam->isVisible(mov->getWorldBoundingBox())
							mov->_notifyCurrentCamera(cam);
							mov->_updateRenderQueue( getRenderQueue() );

							visibleBounds->merge(mov->getBoundingBox(), mov->getWorldBoundingSphere(), cam);

							movablesForRendering.insert(*oi);
					}
				}
			}
		}

		mFindVisibleObjectsTime = Root::getSingleton().getTimer()->getMilliseconds() - startt;


		if (mActiveGeometry) {
			startt = Root::getSingleton().getTimer()->getMilliseconds();

			mActiveGeometry->updateFromCamera(static_cast<DarkCamera*>(cam));
			mActiveGeometry->queueForRendering(mRenderQueue);

			mStaticBuildTime = Root::getSingleton().getTimer()->getMilliseconds() - startt;
		}
	}

	//-----------------------------------------------------------------------
	const String& DarkSceneManager::getTypeName(void) const {
		return DarkSceneManagerFactory::FACTORY_TYPE_NAME;
	}

	//-----------------------------------------------------------------------
	Light* DarkSceneManager::createLight(const String& name) {
		return static_cast<Light*>(createMovableObject(name, DarkLightFactory::FACTORY_TYPE_NAME));

	}

	//-----------------------------------------------------------------------
	Light* DarkSceneManager::getLight(const String& name) {
		return static_cast<Light*>(getMovableObject(name, DarkLightFactory::FACTORY_TYPE_NAME));
	}


	//-----------------------------------------------------------------------
	bool DarkSceneManager::hasLight(const String& name) {
		return hasMovableObject(name, DarkLightFactory::FACTORY_TYPE_NAME);
	}


	//-----------------------------------------------------------------------
	void DarkSceneManager::destroyLight(const String& name) {
		destroyMovableObject(name, DarkLightFactory::FACTORY_TYPE_NAME);
	}


	//-----------------------------------------------------------------------
	void DarkSceneManager::destroyAllLights(void) {
		destroyAllMovableObjectsByType(DarkLightFactory::FACTORY_TYPE_NAME);
	}

	//-----------------------------------------------------------------------
	void DarkSceneManager::queueLightForUpdate(Light* l) {
		static_cast<DarkLight*>(l)->_updateNeeded(); // Adds itself into queue
	}

	//-----------------------------------------------------------------------
	void DarkSceneManager::_queueLightForUpdate(Light* l) {
		mLightsForUpdate.insert(static_cast<DarkLight*>(l));
	}

	//-----------------------------------------------------------------------
	void DarkSceneManager::destroyAllGeometries(void) {
		DarkGeometryMap::iterator it = mDarkGeometryMap.begin();

		while (it != mDarkGeometryMap.end()) {
			DarkGeometryMap::iterator it2 = it++;

			DarkGeometry* g = it2->second;
			mDarkGeometryMap.erase(it2);

			delete g;
		}
	}


	//-----------------------------------------------------------------------
	void DarkSceneManager::updateDirtyLights() {
		LightSet::iterator it = mLightsForUpdate.begin();
		LightSet::iterator end = mLightsForUpdate.end();

		while (it != end) {
			DarkLight* l = *(it++);

			l->_updateAffectedCells();
		}

		mLightsForUpdate.clear();
	}

	//-----------------------------------------------------------------------
	void DarkSceneManager::findLightsAffectingFrustum(const Camera* camera) {
		unsigned long startt = Root::getSingleton().getTimer()->getMilliseconds();

		// Collect lights from visible cells
		std::set<Light*> lightSet;

		const DarkCamera* dcam = static_cast<const DarkCamera*>(camera);

		BspNodeList::const_iterator it = dcam->mVisibleCells.begin();
		BspNodeList::const_iterator end = dcam->mVisibleCells.end();

		while (it != end) {
			BspNode* n = *(it++);

			lightSet.insert(n->mAffectingLights.begin(), n->mAffectingLights.end());
		}

		// Transfer into mTestLightInfos
		std::set<Light*>::iterator lit = lightSet.begin();
		std::set<Light*>::iterator lend = lightSet.end();

		mTestLightInfos.clear();
		mTestLightInfos.reserve(lightSet.size());

		while (lit != lend) {
			LightInfo lightInfo;

			Light* light = *(lit++);

			lightInfo.light = light;
			lightInfo.type = light->getType();

			if (lightInfo.type == Light::LT_DIRECTIONAL) {
				lightInfo.position = Vector3::ZERO;
				lightInfo.range = 0;
				mTestLightInfos.push_back(lightInfo);
			} else {
				lightInfo.range = light->getAttenuationRange();
				lightInfo.position = light->getDerivedPosition();

				Sphere sphere(lightInfo.position, lightInfo.range);

				if (camera->isVisible(sphere)) {
						mTestLightInfos.push_back(lightInfo);
				}
			}
		}

		// Now process the same as SceneManager::findLightsAffectingFrustum does
		// Code copied, in fact:
		// Update lights affecting frustum if changed
		if (mCachedLightInfos != mTestLightInfos) {
			mLightsAffectingFrustum.resize(mTestLightInfos.size());

			LightInfoList::const_iterator i;
			LightList::iterator j = mLightsAffectingFrustum.begin();

			for (i = mTestLightInfos.begin(); i != mTestLightInfos.end(); ++i, ++j) {
				*j = i->light;

				// add cam distance for sorting if texture shadows
				if (isShadowTechniqueTextureBased()) {
					(*j)->tempSquareDist = (camera->getDerivedPosition() - (*j)->getDerivedPosition()).squaredLength();
				}
			}

			// Sort the lights if using texture shadows, since the first 'n' will be
			// used to generate shadow textures and we should pick the most appropriate
			if (isShadowTechniqueTextureBased()) {
				// Allow a Listener to override light sorting
				// Reverse iterate so last takes precedence
				bool overridden = false;

				for (ShadowListenerList::reverse_iterator ri = mShadowListeners.rbegin();
						ri != mShadowListeners.rend(); ++ri) {

						overridden = (*ri)->sortLightsAffectingFrustum(mLightsAffectingFrustum);
						if (overridden)
								break;
				}
				if (!overridden) {
						// default sort (stable to preserve directional light ordering
						std::stable_sort(
								mLightsAffectingFrustum.begin(), mLightsAffectingFrustum.end(),
								lightsForShadowTextureLess());
				}
			}

			// Use swap instead of copy operator for efficiently
			mCachedLightInfos.swap(mTestLightInfos);

			// notify light dirty, so all movable objects will re-populate
			// their light list next time

			// TODO: This takes too much time per frame!
			_notifyLightsDirty();
		}

		mLightCount = mCachedLightInfos.size();
		mLightListTime = Root::getSingleton().getTimer()->getMilliseconds() - startt;
	}

	//-----------------------------------------------------------------------
	void DarkSceneManager::_populateLightList(const Vector3 &position, Real radius, LightList &destList) {
		/* It's a pity we don't get a movable object here,
		since that would mean we'd save one BSP tree traversal per MovableObject light list population.

		The root of the problem is that many classes base on MovableObject, and that one queries with pos and radius already,
		so we would have to create our own versions of Entity, BillBoardChain, etc. just to override one method (queryLights)

		As a solution, this scenemanager uses MovableObject::Listener on BspTree, and as the BspTree caches light lists for cells,
		it returns the light list for object immediately
		*/

		if (!mBspTree)
			return;

		return;

		BspNodeList leafList;

		mBspTree->findLeafsForSphere(leafList, position, radius);

		//  List of all leafs the sphere is in
		BspNodeList::iterator it = leafList.begin();

		// This set ensures the resulting destination list will get unique set of lights
		std::set<DarkLight*> resLights;

		destList.clear();

		while (it != leafList.end()) {
			BspNode* node = *(it++);

			BspNode::AffectingLights::const_iterator lit = node->mAffectingLights.begin();
			BspNode::AffectingLights::const_iterator lend = node->mAffectingLights.end();

			// Fill the light list by querying if the light's radius is touching the movable
			while (lit != lend) {
				DarkLight* lt = *(lit++);

				// If it has not been considered yet, reconsider this light
				// (no need to consider light twice)
				if (resLights.find(lt) == resLights.end()) {
					resLights.insert(lt);

					if (lt->getType() == Light::LT_DIRECTIONAL)	{
						// No distance
						lt->tempSquareDist = 0.0f;
						destList.push_back(lt);
					} else	{
						// Calc squared distance
						lt->tempSquareDist = (lt->getDerivedPosition() - position).squaredLength();

						// only add in-range lights
						Real range = lt->getAttenuationRange();

						Real maxDist = range + radius;

						if (lt->tempSquareDist <= Math::Sqr(maxDist)) {
							destList.push_back(lt);
						}
					}
				}
			}
		}


		// the end is the same as in Ogre::SceneManager:

		// Sort (stable to guarantee ordering on directional lights)
        if (isShadowTechniqueTextureBased()) {
                // Note that if we're using texture shadows, we actually want to use
                // the first few lights unchanged from the frustum list, matching the
                // texture shadows that were generated
                // Thus we only allow object-relative sorting on the remainder of the list
                if (destList.size() > getShadowTextureCount()) {
                        LightList::iterator start = destList.begin();
                        std::advance(start, getShadowTextureCount());
                        std::stable_sort(start, destList.end(), lightLess());
                }
        } else {
                std::stable_sort(destList.begin(), destList.end(), lightLess());
        }
	}

	//-----------------------------------------------------------------------
	Entity *DarkSceneManager::createEntity(const String &entityName, const String &meshName) {
		Entity* e = SceneManager::createEntity(entityName, meshName);

		e->setListener(mBspTree);

		return e;
	}

	//-----------------------------------------------------------------------
	DarkGeometry *DarkSceneManager::createGeometry(const String& geomName) {
		DarkGeometryMap::iterator it = mDarkGeometryMap.find(geomName);

		if (it != mDarkGeometryMap.end()) {
			return it->second;
		}

		DarkGeometry* geom = new DarkGeometry(geomName, RENDER_QUEUE_WORLD_GEOMETRY_1);

		mDarkGeometryMap.insert(make_pair(geomName, geom));

		return geom;
	}

	//-----------------------------------------------------------------------
	void DarkSceneManager::destroyGeometry(const String& name) {
		DarkGeometryMap::iterator it = mDarkGeometryMap.find(name);

		if (it != mDarkGeometryMap.end()) {
			DarkGeometry* g = it->second;
			mDarkGeometryMap.erase(it);
			delete g;
		}
	}


	//-----------------------------------------------------------------------
	DarkGeometry *DarkSceneManager::getGeometry(const String& name) {
		DarkGeometryMap::iterator it = mDarkGeometryMap.find(name);

		if (it != mDarkGeometryMap.end()) {
			return it->second;
		}

		// TODO: The damned NULL/Exception throw dilema all over again
		return NULL;
	}

	//-----------------------------------------------------------------------
	void DarkSceneManager::setActiveGeometry(const String& name) {
		mActiveGeometry = getGeometry(name);
	}

	//-----------------------------------------------------------------------
	void DarkSceneManager::setActiveGeometry(DarkGeometry* geom) {
		mActiveGeometry = geom;
	}

	//-----------------------------------------------------------------------
	bool DarkSceneManager::getOption(const String &strKey, void *pDestValue) {
		if (strKey == "StaticBuildTime") {
			*(static_cast<unsigned long*>(pDestValue)) = mStaticBuildTime;
			return true;
		} else if (strKey == "FindVisibleObjectsTime") {
			*(static_cast<unsigned long*>(pDestValue)) = mFindVisibleObjectsTime;
			return true;
		} else if (strKey == "LightListTime") {
			*(static_cast<unsigned long*>(pDestValue)) = mLightListTime;
			return true;
		} else if (strKey == "SceneGraphTime") {
			*(static_cast<unsigned long*>(pDestValue)) = mSceneGraphTime;
			return true;
		} else if (strKey == "LightCount") {
			*(static_cast<unsigned long*>(pDestValue)) = mLightCount;
			return true;
		}


		return SceneManager::getOption(strKey, pDestValue);
	}

	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	const String DarkSceneManagerFactory::FACTORY_TYPE_NAME = "DarkSceneManager";
	//-----------------------------------------------------------------------
	void DarkSceneManagerFactory::initMetaData(void) const {
			mMetaData.typeName = FACTORY_TYPE_NAME;
			mMetaData.description = "Scene manager for BSP/Portal based maps";
			mMetaData.sceneTypeMask = ST_INTERIOR;
			mMetaData.worldGeometrySupported = false;
	}
	//-----------------------------------------------------------------------
	SceneManager* DarkSceneManagerFactory::createInstance(
			const String& instanceName) {
			return new DarkSceneManager(instanceName);
	}
	//-----------------------------------------------------------------------
	void DarkSceneManagerFactory::destroyInstance(SceneManager* instance) {
			delete instance;
	}

};
