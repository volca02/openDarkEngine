/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
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
 *	  $Id$
 *
 *****************************************************************************/

#include <OgreSceneNode.h>

#include "config.h"

#include "ServiceCommon.h"
#include "WorldRepService.h"
#include "ConfigService.h"
#include "WRTypes.h"

#include "DarkBspNode.h"
#include "DarkSceneManager.h"

#include "integers.h"
#include "OpdeException.h"
#include "WRCommon.h"
#include "logger.h"
#include "File.h"
#include "FileCompat.h"

using namespace Ogre;

// #define __SG

namespace Opde {
	/*----------------------------------------------------*/
	/*----------------- WorldRep Service -----------------*/
	/*----------------------------------------------------*/
	template<> const size_t ServiceImpl<WorldRepService>::SID = __SERVICE_ID_WORLDREP;
	
	WorldRepService::WorldRepService(ServiceManager *manager, const std::string& name) :
		ServiceImpl< Opde::WorldRepService >(manager, name), mNumCells(0) {
		// ResourceGroupManager::getSingleton().setWorldResourceGroupName(TEMPTEXTURE_RESOURCE_GROUP);

	}

	//------------------------------------------------------
	bool WorldRepService::init() {
		mRenderService
		        = GET_SERVICE(RenderService);

		if (mRenderService.isNull()) {
			LOG_ERROR("RenderService instance was not found. Fatal");
			return false;
		}

		mRoot = mRenderService->getOgreRoot();
		mSceneMgr = dynamic_cast<DarkSceneManager *> (mRenderService->getSceneManager());

		return true;
	}

	//------------------------------------------------------
	void WorldRepService::bootstrapFinished() {
		// Get a reference to the sceneManager. We can get DarkSceneManager directly because of the format of the data we load (BSP/Portals)
		mCells = NULL;
		mExtraPlanes = NULL;

		LOG_DEBUG("WorldRepService: Registering as a listener to the database messages");
		mDatabaseService = GET_SERVICE(DatabaseService);
 		mDatabaseService->registerListener(this, DBP_WORLDREP);

		mLightService = GET_SERVICE(LightService);
	}

	//------------------------------------------------------
	void WorldRepService::shutdown() {
		mDatabaseService->unregisterListener(this);
		mDatabaseService.setNull();
		clearData();

		mRenderService.setNull();
	}

	//------------------------------------------------------
	WorldRepService::~WorldRepService() {
	}


	//------------------------------------------------------
	void WorldRepService::onDBLoad(const FileGroupPtr& db, uint32_t curmask) {
		LOG_INFO("WorldRepService::onDBLoad called.");
		
		if (!(curmask & DBM_MIS_DATA))
			return;
		
		// If there is some scene already, clear it
		clearData();

		int lightSize = 1;

		FilePtr wrChunk;

		if (db->hasFile("WR")) {
			wrChunk = db->getFile("WR");
		} else if (db->hasFile("WRRGB")) {
			lightSize = 2;
			wrChunk = db->getFile("WRRGB");
		} else {
			// Still no data?
			OPDE_EXCEPT("Could not find WR nor WRRGB chunk...", "WorldRepService::loadFromDarkDatabase");
		}

		loadFromChunk(wrChunk, lightSize);
	}
	
	//------------------------------------------------------
	void WorldRepService::onDBSave(const FileGroupPtr& db, uint32_t tgtmask) {
		LOG_INFO("WorldRepService::onDBSave called.");
		// TODO: Stub
	}
	
	//------------------------------------------------------
	void WorldRepService::onDBDrop(uint32_t dropmask) {
		LOG_INFO("WorldRepService::onDBDrop called.");
		// if mission or gamesys is dropped,
		// drop here as well
		if (dropmask & DBM_MIS_DATA)
			unload();
	}

	//------------------------------------------------------
	void WorldRepService::setSkyBox(const FileGroupPtr& db) {
		return;
		FilePtr skyChunk = db->getFile("SKYMODE");

		if (!skyChunk.isNull()) { // Thief1 sky. Thief2 has NewSky. Will need to make a custom scene node for this.
			uint32_t skyMode;

			skyChunk->readElem(&skyMode, sizeof(skyMode), 1);

			if (skyMode == 0) {
				mSceneMgr->setSkyBox(true, "Skybox/daysky", 1000);
			} else {
				mSceneMgr->setSkyBox(true, "Skybox/nightsky", 1000);

			}
		}
	}

	//------------------------------------------------------
	void WorldRepService::unload() {
		mIndexes.setNull();

		mSceneMgr->destroyGeometry("LEVEL_GEOMETRY");

		clearData();
	}

	//------------------------------------------------------
	void WorldRepService::clearData() {
		LOG_INFO("WorldRepService::clearData called");

		// this might cause problems because of render service trying to
		// release all the entities later, when those are already invalid
		// a special care must be taken
		mSceneMgr->clearScene();

		if (mCells != NULL) {
			for (uint32_t i = 0; i < mNumCells; i++) {
				LOG_DEBUG("WorldRepService::clearData deleting cell %d of %d", i, mNumCells);
				delete mCells[i];
			}
		}

		delete[] mCells;
		mCells = NULL;

		delete[] mExtraPlanes;
		mExtraPlanes = NULL;

		// inform the Light service that it can unload now
		mLightService->clear();

		mNumCells = 0;

		LOG_INFO("WorldRepService::clearData : finished cleaning up");
	}


	//------------------------------------------------------
	// ----------------------- The level loading methods follow
	void WorldRepService::loadFromChunk(FilePtr& wrChunk, size_t lightSize) {
		LOG_DEBUG("WorldRepService: Loading WR/WRRGB");
		WRHeader header;
		*wrChunk >> header;

		mNumCells = header.numCells;

		mCells = new WRCell*[header.numCells];

		LOG_DEBUG("WorldRepService: Loading Cells");
		
		mWorldGeometry = mSceneMgr->createGeometry("LEVEL_GEOMETRY"); // will be deleted on clear_scene
		mWorldGeometry->setCellCount(header.numCells);

		for (uint32_t i = 0; i < mNumCells; i++) {
			mCells[i] = new WRCell(this, mWorldGeometry);
		}

		mLightService->setLightPixelSize(lightSize);

		unsigned int idx;
		for (idx = 0; idx < header.numCells; idx++) {
			// Load one Cell
			mCells[idx]->loadFromChunk(idx, wrChunk, lightSize);
		}

		LOG_DEBUG("WorldRepService: Loading Extra planes");
		
		// -- Load the extra planes
		wrChunk->read(&mExtraPlaneCount, sizeof(uint32_t));

		mExtraPlanes = new Plane[mExtraPlaneCount];
		for (size_t i = 0; i < mExtraPlaneCount; ++i)
			*wrChunk >> mExtraPlanes[i];
		
		// --------------------------------------------------------------------------------
		// -- Load and process the BSP tree
		LOG_DEBUG("WorldRepService: Loading BSP");

		uint32_t BspRows;
		wrChunk->read(&BspRows, sizeof(uint32_t));

		// Load the BSP, and construct it
		WRBSPNode *Bsp = new WRBSPNode[BspRows];
		wrChunk->read(Bsp, BspRows * sizeof(WRBSPNode));

		// Create the BspTree
		createBSP(BspRows, Bsp);

		delete[] Bsp;

		// --------------------------------------------------------------------------------
		// let the light service build the atlases, etc
		LOG_DEBUG("WorldRepService: Loading Lights table");
		mLightService->_loadTableFromTagFile(wrChunk);
		mLightService->build();

		// assign the leaf nodes
		for (idx = 0; idx < header.numCells; idx++) {
			BspNode* node = mSceneMgr->getBspLeaf(idx);
			mCells[idx]->setBspNode(node);
		}

		// --------------------------------------------------------------------------------
		LOG_DEBUG("WorldRepService: Attaching portals");
		// Attach the portals to the BSP tree leafs
		int optimized = 0;

		for (idx = 0; idx < header.numCells; idx++) {
			optimized += mCells[idx]->attachPortals(mSceneMgr);
		}

		LOG_INFO("Worldrep: Optimization removed %d vertices", optimized);

		// --------------------------------------------------------------------------------
		LOG_DEBUG("WorldRepService: Creating WR geometry");
		// Build the portal meshes and cell geometry
		for (idx = 0; idx < header.numCells; idx++) {
			mCells[idx]->constructPortalMeshes(mSceneMgr);
			mCells[idx]->createCellGeometry();
		}

		// build the buffers
		LOG_DEBUG("WorldRepService: Building WR geometry");
		mWorldGeometry->build();
		mSceneMgr->setActiveGeometry(mWorldGeometry);

		// --------------------------------------------------------------------------------
		// We have done all we could, bringing the level data to the SceneManager. Now delete the used data
		LOG_DEBUG("Worldrep: Freeing temporary buffers");

		delete[] mExtraPlanes;
		mExtraPlanes = NULL;
		LOG_DEBUG("Worldrep: Freeing done");
	}


	// ---------------------------------------------------------------------
	void WorldRepService::createBSP(unsigned int BspRows, WRBSPNode *tree) {
		// First pass - creates all the BSP nodes
		for (unsigned int i = 0; i < BspRows; i++) {
			const WRBSPNode& wr_node = tree[i];

			if (_BSP_FLAGS(wr_node) & 0x01) { // leaf node
				mSceneMgr->createBspNode(i, wr_node.front);
			} else {
				// split node
				mSceneMgr->createBspNode(i);
			}
		}

		// Second Pass. Set front, back pointers on the tree split nodes
		for (unsigned int i = 0; i < BspRows; i++) {
			const WRBSPNode& wr_node = tree[i];


			// If this is a leaf node, go to next row
			if (_BSP_FLAGS(wr_node) & 0x01)
				continue;

			int front = -1;
			int back = -1;

			if (wr_node.front != 0xFFFFFF)
				front = wr_node.front;

			if (wr_node.back != 0xFFFFFF)
				back = wr_node.back;

			// get the represented (pre-created) node
			BspNode* node = mSceneMgr->getBspNode(i);


			// Set the split plane
			if (wr_node.cell < 0) {
				assert(wr_node.plane >= 0);
				assert((unsigned int) (wr_node.plane) < mExtraPlaneCount);
				node->setSplitPlane(mExtraPlanes[wr_node.plane]); // Extra planes
			} else {
				assert((unsigned int) (wr_node.cell) < mNumCells);
				node->setSplitPlane(mCells[wr_node.cell]->getPlane(wr_node.plane));
			}


			// if swap flag set, swap front and back
			if (_BSP_FLAGS(wr_node) & 0x04) { // inverted plane normal, swap front n' back child
				if (front >= 0)
					node->setBackChild(mSceneMgr->getBspNode(front));

				if (back >= 0)
					node->setFrontChild(mSceneMgr->getBspNode(back));
			} else {
				if (front >= 0)
					node->setFrontChild(mSceneMgr->getBspNode(front));

				if (back >= 0)
					node->setBackChild(mSceneMgr->getBspNode(back));
			}
		}

		mSceneMgr->setRootBspNode(0);
		// DONE!

		LOG_DEBUG("Worldrep: Finished the BSP build");
	}

	// ---------------------------------------------------------------------
	Ogre::SceneManager *WorldRepService::getSceneManager() {
		return mSceneMgr;
	}


	//-------------------------- Factory implementation
	std::string WorldRepServiceFactory::mName = "WorldRepService";

	WorldRepServiceFactory::WorldRepServiceFactory() :
		ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
	};

	Service* WorldRepServiceFactory::createInstance(ServiceManager* manager) {
		return new WorldRepService(manager, mName);
	}

	const std::string& WorldRepServiceFactory::getName() {
		return mName;
	}

	const uint WorldRepServiceFactory::getMask() {
		return SERVICE_DATABASE_LISTENER | SERVICE_RENDERER | SERVICE_ENGINE;
	}
	
	const size_t WorldRepServiceFactory::getSID() {
		return WorldRepService::SID;
	}
}
