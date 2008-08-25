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


#include <Ogre.h>
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

using namespace Ogre;

// #define __SG

namespace Opde {
	// Implementation of the WorldRep service
	WorldRepService::WorldRepService(ServiceManager *manager, const std::string& name) : Service(manager, name) {
	    // ResourceGroupManager::getSingleton().setWorldResourceGroupName(TEMPTEXTURE_RESOURCE_GROUP);

	}

    bool WorldRepService::init() {
        mRenderService = ServiceManager::getSingleton().getService("RenderService").as<RenderService>();

        if (mRenderService.isNull()) {
            LOG_ERROR("RenderService instance was not found. Fatal");
            return false;
        }

   		mRoot = mRenderService->getOgreRoot();
        mSceneMgr = dynamic_cast<DarkSceneManager *>(mRenderService->getSceneManager());
        
        return true;
    }

	void WorldRepService::bootstrapFinished() {
	    // Get a reference to the sceneManager. We can get DarkSceneManager directly because of the format of the data we load (BSP/Portals)

		mCells = NULL;
		mExtraPlanes = NULL;

		mAtlas = NULL;

		// Some standard image format extensions to try, when constructing the material manually
		mTextureExtensions.insert(String(".tga"));
		mTextureExtensions.insert(String(".jpg"));
		mTextureExtensions.insert(String(".jpeg"));
		mTextureExtensions.insert(String(".pcx"));
		mTextureExtensions.insert(String(".png"));

		// try to create lightmap resource group used for lightmap storage
		try {
			ResourceGroupManager::getSingleton().createResourceGroup(TEMPTEXTURE_RESOURCE_GROUP);
		} catch (Exception &e) {
			LOG_ERROR("Cannot create temporary texture/materials resource group '%s'. Exception : %s",
				 TEMPTEXTURE_RESOURCE_GROUP, e.getDescription().c_str());
		}

		mTextures = NULL;
		mFamilies = NULL;

        LOG_DEBUG("WorldRepService: Registering as a listener to the database messages");
		mDbCallback = new ClassCallback<DatabaseChangeMsg, WorldRepService>(this, &WorldRepService::onDBChange);

		mDatabaseService = ServiceManager::getSingleton().getService("DatabaseService").as<DatabaseService>();
		mDatabaseService->registerListener(mDbCallback, DBP_WORLDREP);

	}

	WorldRepService::~WorldRepService() {
		mDatabaseService->unregisterListener(mDbCallback);
		clearData();
	}


    void WorldRepService::addWorldMaterial(const MaterialPtr& material) {
        mLoadedMaterials.push_back(material);
    }

	// ---- The ServiceInterface mathods ---
	void WorldRepService::onDBChange(const DatabaseChangeMsg& m) {
	    LOG_DEBUG("WorldRepService::onDBChange called by a callback");
	    if (m.change == DBC_DROPPING) {
	        unload();
	        return;
	    }

		if (m.change == DBC_LOADING && m.dbtype == DBT_MISSION) {
            // Initialize materials here:
            loadMaterials(m.db);

            int lightSize = 1;

            FilePtr wrChunk;

            if (m.db->hasFile("WR")) {
                wrChunk = m.db->getFile("WR");
            } else if (m.db->hasFile("WRRGB")) {
                lightSize = 2;
                wrChunk = m.db->getFile("WRRGB");
            } else {
                // Still no data?
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Could not find WR nor WRRGB chunk...","WorldRepService::loadFromDarkDatabase");
            }

            loadFromChunk(wrChunk, lightSize);

            // --- Finally, set sky according to the SKY chunk
            setSkyBox(m.db);
		}
	}


	void WorldRepService::setSkyBox(const FileGroupPtr& db) {
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


	void WorldRepService::unload() {
	    mIndexes.setNull();
	    
	    mSceneMgr->clearScene();
	    
		mTxtScaleMap.clear();

		clearData();
	}

	void WorldRepService::clearData() {
		LOG_INFO("WorldRepService::clearData called");

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

		delete mAtlas;
		mAtlas = NULL;

		// Unregister all the resources in the WorldResourceGroup, including unreloadable

		// if there is a LightMap or WrTextures resource group, clean up those...
		MaterialList::const_iterator it = mLoadedMaterials.begin();

		while ( it != mLoadedMaterials.end() ) {
		    LOG_DEBUG("WorldRepService::clearData Removing material %s", (*it)->getName().c_str());
		    MaterialManager::getSingleton().remove((*it)->getName());

		    ++it;
		}

		mLoadedMaterials.clear();

		delete[] mFamilies;
		mFamilies = NULL;


		delete[] mTextures;
		mTextures = NULL;

        delete mAtlas;
        mAtlas = NULL;

		mNumCells = 0;

		LOG_INFO("WorldRepService::clearData : finished cleaning up");
	}

	// ----------------------- The level loading methods follow
	void WorldRepService::loadFromChunk(FilePtr& wrChunk, int lightSize) {
		wr_hdr_t header;
		wrChunk->read(&header, sizeof(wr_hdr_t));

		// If there is some scene already, clear it
		// clearData();

		mNumCells = header.num_cells;

		mAtlas = new LightAtlasList();

		mCells = new WRCell*[header.num_cells];

		mWorldGeometry = mSceneMgr->createGeometry("LEVEL_GEOMETRY"); // will be deleted on clear_scene

		for (uint32_t i = 0; i<header.num_cells; i++) {
            mCells[i] = new WRCell(this, mWorldGeometry);
		}

		unsigned int idx;
		for (idx=0; idx < header.num_cells; idx++) {
			// Load one Cell
			mCells[idx]->loadFromChunk(idx, wrChunk, lightSize);
		}

		// -- Load the extra planes
		wrChunk->read(&mExtraPlaneCount, sizeof(uint32_t));

		mExtraPlanes = new wr_plane_t[mExtraPlaneCount];
		wrChunk->read(mExtraPlanes, sizeof(wr_plane_t) * mExtraPlaneCount);

		LOG_INFO("Worldrep: queueing lightmaps");
		// -- Atlas the lightmaps
		for (idx=0; idx < header.num_cells; idx++)
			mCells[idx]->atlasLightMaps(mAtlas);


		LOG_INFO("Worldrep: Atlasing lightmaps");
		// Render the lmaps! This could be moved to a pre-run stage, as the mission difficulties alter the light states (and we do not know a s**t about that here)
		mAtlas->render();

		LOG_INFO("Worldrep: Atlasing Done");

		
		// --------------------------------------------------------------------------------
		// -- Load and process the BSP tree
		uint32_t BspRows;
		wrChunk->read(&BspRows, sizeof(uint32_t));

		// Load the BSP, and construct it
		wr_BSP_node_t *Bsp = new wr_BSP_node_t[BspRows];
		wrChunk->read(Bsp, BspRows * sizeof(wr_BSP_node_t));

		// Create the BspTree
		createBSP(BspRows, Bsp);
		
		delete[] Bsp;

		// assign the leaf nodes
		for (idx=0; idx < header.num_cells; idx++) {
			BspNode* node = mSceneMgr->getBspLeaf(idx);
			mCells[idx]->setBspNode(node);
		}
		
		// --------------------------------------------------------------------------------
		// Attach the portals to the BSP tree leafs
		int optimized = 0;

		for (idx=0; idx < header.num_cells; idx++) {
			optimized += mCells[idx]->attachPortals(mSceneMgr);
		}

		LOG_INFO("Worldrep: Optimization removed %d vertices", optimized);

		// --------------------------------------------------------------------------------
		// Now construct the static geometry
#ifdef __SG
		Ogre::StaticGeometry* sg = mSceneMgr->createStaticGeometry("MISSION_GEOMETRY");
		
		// 100 units per sg region
		DVariant size = 100.0f;
		
		// Try to get an override from the ConfigService
		ConfigServicePtr cfs = ServiceManager::getSingleton().getService("ConfigService").as<ConfigService>();
		
		if (cfs->getParam("region_size", size)) {
			LOG_INFO("Worldrep: Found a region size override in the config file... new value : %10.2f", size.toFloat());
		}
		
		sg->setRegionDimensions(Vector3(size.toFloat(), size.toFloat(), size.toFloat()));
		
		// no displacement
		sg->setOrigin(Vector3(0, 0, 0));
		
		std::vector< std::string > nodesToDestroy;
#endif
		
		//TODO: Portal meshes need to be constructed. This will guarantee the other meshes can be attached if this one works ok
		// Hmm. Actually, it renders quite a lot of the meshes that should not be seen.
		for (idx=0; idx < header.num_cells; idx++) {
			mCells[idx]->constructPortalMeshes(mSceneMgr);
			
			
#ifdef __SG
			Ogre::SceneNode* node = mCells[idx]->createSceneNode(mSceneMgr);
			
			// Non - sg rendering - slower
			sg->addSceneNode(node);
			// mSceneMgr->destroySceneNode(node->getName());
			nodesToDestroy.push_back(node->getName());
#else
			// mSceneMgr->getRootSceneNode()->addChild(node);
			mCells[idx]->createCellGeometry();
#endif			
		}

#ifdef __SG
		LOG_DEBUG("Worldrep: Building static geometry...");
		
		sg->setCastShadows(false);

		sg->build();

		std::vector<std::string>::iterator it = nodesToDestroy.begin();

		for(; it != nodesToDestroy.end(); it++)
			mSceneMgr->destroySceneNode(*it);
			
		// render SG before everything else
		sg->setRenderQueueGroup(RENDER_QUEUE_MAIN - 1);
#else
		mWorldGeometry->build();
		mSceneMgr->setActiveGeometry(mWorldGeometry);
#endif


		// --------------------------------------------------------------------------------
		// We have done all we could, bringing the level data to the SceneManager. Now delete the used data
		LOG_DEBUG("Worldrep: Freeing temporary buffers");
		
		delete[] mCells;
		mCells = NULL;
		delete[] mExtraPlanes;
		mExtraPlanes = NULL;
		LOG_DEBUG("Worldrep: Freeing done");
	}

	// ---------------------------------------------------------------------
	void WorldRepService::createBSP(unsigned int BspRows, wr_BSP_node_t *tree) {
		// First pass - creates all the BSP nodes
		for (unsigned int i = 0; i < BspRows; i++) {
			const wr_BSP_node_t& wr_node = tree[i];
			
			if (_BSP_FLAGS(wr_node) & 0x01) { // leaf node
				mSceneMgr->createBspNode(i, wr_node.front);
			} else {
				// split node
				mSceneMgr->createBspNode(i);
			}
		}
		
		// Second Pass. Set front, back pointers on the tree split nodes
		for (unsigned int i = 0; i < BspRows; i++) {
			const wr_BSP_node_t& wr_node = tree[i];
			
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
				assert(wr_node.plane < mExtraPlaneCount);
				node->setSplitPlane(constructPlane(mExtraPlanes[wr_node.plane])); // Extra planes
			} else {
				assert(wr_node.cell < mNumCells);
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

	//-----------------------------------------------------------------------
	Ogre::Plane WorldRepService::constructPlane(wr_plane_t plane) {
		Vector3	normal(plane.normal.x, plane.normal.y, plane.normal.z);
		float dist = plane.d;
		Ogre::Plane oplane;
		oplane.normal = normal;
		oplane.d = dist;
		return oplane;
	}

	//-----------------------------------------------------------------------
	// TODO: Move this into texture/flow service
	void WorldRepService::loadFlowTextures(const FileGroupPtr& db) {
		// Load the TXLIST chunk from the resource mission file.
		Opde::FilePtr flow_tex;
		try {
			flow_tex = db->getFile("FLOW_TEX");
		} catch (FileException) {
			LOG_INFO("Flow chunk does not exist. Water materials may not be correctly displayed", "WorldRepService::loadFlowTextures");
			return;
		}

		// TODO: Exception handling on the chunk readout!
		// Okay, we are ready to map the arrays
		DarkDBChunkFLOW_TEX flows;
		int flow_count = flow_tex->size() / 32; // The record is 32 bytes long, this way we do not fail if the chunk is shorter

		try {
			// load
			flow_tex->read(&flows, flow_tex->size()); // To be sure we do not overlap
		} catch (Ogre::Exception& e) {
			// Connect the original exception to the printout:
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("Could not load flow textures : ") + e.getFullDescription(), "WorldRepService::loadFlowTextures");
		}


		// now try to load non-zero flow textures as materials
		for (int fnum = 0; fnum < flow_count; fnum++) {
			if (strlen(flows.flow[fnum].name) > 0) { // nonzero name, try to load
				// Construct the basic name of the material
				std::string matname("water/");
				matname += flows.flow[fnum].name;

				// try to find the texture definition. If found, clone to the @template + the in_texture/out_texture number
				if (MaterialManager::getSingleton().resourceExists(matname+"_in")) {
					MaterialPtr origMat = MaterialManager::getSingleton().getByName(matname+"_in");

					StringUtil::StrStreamType matName;
					matName << "@template" << flows.flow[fnum].in_texture;

					if (MaterialManager::getSingleton().resourceExists(matName.str())) {
						MaterialManager::getSingleton().remove(matName.str());
					}

					MaterialPtr shadMat = origMat->clone(matName.str() );
					shadMat->load();

					addWorldMaterial(shadMat);
					LOG_INFO("Flow now defined : %s (template %s_in)", matName.str().c_str(), matname.c_str());
				} else {
					LOG_ERROR("Material not found : %s_in", matname.c_str());
				}

				// OUT
				if (MaterialManager::getSingleton().resourceExists(matname+"_out")) {
					MaterialPtr origMat = MaterialManager::getSingleton().getByName(matname+"_out");

					StringUtil::StrStreamType matName;
					matName << "@template" << flows.flow[fnum].out_texture;

					if (MaterialManager::getSingleton().resourceExists(matName.str())) {
						MaterialManager::getSingleton().remove(matName.str());
					}

					MaterialPtr shadMat = origMat->clone(matName.str() );
					shadMat->load();

					addWorldMaterial(shadMat);
					LOG_INFO("Flow now defined : %s (template %s_in)", matName.str().c_str(), matname.c_str());
				} else {
					LOG_ERROR("Material not found : %s_out", matname.c_str());
				}

			}
		}
	}

	//-----------------------------------------------------------------------
	std::pair<float, float> WorldRepService::getTextureScale(const std::string& txtName) {
		TxtScaleMap::const_iterator it = mTxtScaleMap.find(txtName);
		
		if (it != mTxtScaleMap.end()) {
			return it->second;
		} else {
			// Default to 1,1
			return std::pair<float, float>(1.0f, 1.0f);
		}
	}
    
    //-----------------------------------------------------------------------        
    void WorldRepService::setTextureScale(const std::string& txtName, std::pair<float, float> scale) {
    	mTxtScaleMap.insert(make_pair(txtName, scale));
    }

	//-----------------------------------------------------------------------
	void WorldRepService::createStandardMaterial(std::string matName, std::string textureName, std::string resourceGroup) {
		Image tex;
		bool loaded = false; // indicates we were succesful finding the texture

		// Let's try the extensions from the extensions vector
		std::set< String >::iterator it = mTextureExtensions.begin();

		for (;it != mTextureExtensions.end(); it++) { // Try loading
			try {
				tex.load(textureName + (*it), resourceGroup);

				TextureManager::getSingleton().loadImage(textureName, resourceGroup, tex, TEX_TYPE_2D, 5, 1.0f );

				loaded = true;

				break; // we got it!
			} catch (Ogre::Exception) {
				// Nothing. We are trying more extensions
			}
		}

		if (!loaded) // TODO: Find a replacement? I don't think so...
			LogManager::getSingleton().logMessage("Image " + textureName + " was not found, texture will be invalid!");


		// Construct a material out of this texture. We'll just clone the material upstairs to enable lmap-txture combinations
		MaterialPtr shadMat = MaterialManager::getSingleton().create(matName, resourceGroup);
		
		shadMat->setReceiveShadows(true);
		
		Pass *shadPass = shadMat->getTechnique(0)->getPass(0);

		shadPass->setAmbient(0.5, 0.5, 0.5);
		shadPass->setDiffuse(1, 1, 1, 1);
		shadPass->setSpecular(1, 1, 1, 1);

		// Texture unit state for the main texture...
		TextureUnitState* tus = shadPass->createTextureUnitState(textureName);

		// Set replace on all first layer textures for now
		// tus->setColourOperation(LBO_REPLACE);
		tus->setTextureAddressingMode(TextureUnitState::TAM_WRAP);
		tus->setTextureCoordSet(0);
		tus->setTextureFiltering(TFO_BILINEAR);

		tus->setTextureUScale(1.0f);
		tus->setTextureVScale(1.0f);
		// tus->setTextureFiltering(TFO_NONE);

		// Set culling mode to none
		// shadMat->setCullingMode(CULL_ANTICLOCKWISE);

		// No dynamic lighting
		shadMat->setLightingEnabled(false);
		
		// DYNL:
		shadMat->load();


		addWorldMaterial(shadMat);
	}

	// ---------------------------------------------------------------------
	void WorldRepService::loadMaterials(const FileGroupPtr& db) {
		if (!db->hasFile("TXLIST"))
			throw Exception(Exception::ERR_ITEM_NOT_FOUND,
				"Mission file does not contain Texture list chunk (TXLIST)",
				"WorldRepService::loadMaterials");

		// Load the TXLIST chunk from the resource mission file.
		Opde::FilePtr txtList = db->getFile("TXLIST");

		// TODO: Exception handling on the chunk readout!
		// Okay, we are ready to map the arrays
		if (mFamilies != NULL)
			delete[] mFamilies;

		if (mTextures != NULL)
			delete[] mTextures;

		try {
			// TODO: This should be implemented using dtypes
			// Read the header...
			txtList->read(&mTxlistHeader, sizeof(DarkDBChunkTXLIST));

			// now read all the families

			// allocate the needed space
			mFamilies = new DarkDBTXLIST_fam[mTxlistHeader.fam_count];
			// load
			txtList->read(&(mFamilies[0]), sizeof(DarkDBTXLIST_fam) * mTxlistHeader.fam_count);

			// Now read the textures. Same as before

			// allocate the needed space
			mTextures = new DarkDBTXLIST_texture[mTxlistHeader.txt_count];
			// load texture names
			txtList->read(&(mTextures[0]), sizeof(DarkDBTXLIST_texture) * mTxlistHeader.txt_count);
		} catch (Ogre::Exception& e) {
			if (mFamilies != NULL)
				delete[] mFamilies;
			if (mTextures != NULL)
				delete[] mTextures;

			// Connect the original exception to the printout:
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("Could not load texture list : ") + e.getFullDescription(), "BspLevel::loadMaterials");
		}

		// Okay, we are ready to load the materials now!

		// Our resource group
		String resourceGroup = ResourceGroupManager::getSingleton().getWorldResourceGroupName();

		createSkyHackMaterial(resourceGroup);
		createJorgeMaterial(resourceGroup);

		// ------------- Following code loads the standard materials -------------------

		// Iterate through all materials, and load them (tries to load material as a script (named family/texture), and if it fails,
		// it constructs one with the default settings, and tries a few extensions too for the image)

		for (unsigned int id = 1; id < mTxlistHeader.txt_count; id++) {
			// Try to find the family for the texture
			std::string path = getMaterialName(id);

			// Resulting material name
			StringUtil::StrStreamType matName;
			matName << "@template" << id;

			if (MaterialManager::getSingleton().resourceExists(matName.str())) // if the material is already defined
				// remove, as we have to redefine it
				MaterialManager::getSingleton().remove(matName.str());

			// See if the material is defined by a script. If so, clone it to be named @templateXXXX (XXXX = texture number)
			// We seek material named: familly/texture
			if (MaterialManager::getSingleton().resourceExists(path)) {
				LOG_INFO("loadMaterials: Found material definition for %s", path.c_str());
				MaterialPtr origMat = MaterialManager::getSingleton().getByName(path);

				MaterialPtr shadMat = origMat->clone(matName.str(), true, resourceGroup);

				shadMat->load();

				addWorldMaterial(shadMat);
			} else { // The material script was not found
				createStandardMaterial(matName.str(), path, resourceGroup);
			}
			// This is it. Material @templateXX created
		}

		// Initialize the flow textures (do this first so water specialisation will override)
		loadFlowTextures(db);
	}

	// ---------------------------------------------------------------------
	Ogre::SceneManager *WorldRepService::getSceneManager() {
		return mSceneMgr;
	}

	// ---------------------------------------------------------------------
	void WorldRepService::createSkyHackMaterial(const Ogre::String& resourceGroup) {
		// First, we'll create the sky materials, named SkyShader and WaterShader
		// The sky material does basically only write the Z value (no color write), and should be rendered prior to any other material
		// This is because we render sky(box/sphere/dome/plane) first, and we want it to be visible through the faces textured by this material
		std::string shaderName("SkyShader");

		if (!MaterialManager::getSingleton().resourceExists(shaderName)) {
			MaterialPtr SkyShader = MaterialManager::getSingleton().create(shaderName, resourceGroup);

			Pass *shadPass = SkyShader->getTechnique(0)->getPass(0);

			// No texture for this pass
			// Set the pass to be totally transparent - no color writing
			shadPass->setColourWriteEnabled(false);

			// Set culling mode to none
			shadPass->setCullingMode(CULL_NONE);

			// No dynamic lighting (Sky!)
			shadPass->setLightingEnabled(false);
			// ---- End of skyhack ----
		}
	}
	
	// ---------------------------------------------------------------------
	void WorldRepService::createJorgeMaterial(const Ogre::String& resourceGroup) {
		std::string shaderName("@template0");

		if (!MaterialManager::getSingleton().resourceExists(shaderName)) {
			MaterialPtr shadMat = MaterialManager::getSingleton().create(shaderName, resourceGroup);
			shadMat->setReceiveShadows(true);
		
			Pass *shadPass = shadMat->getTechnique(0)->getPass(0);

			shadPass->setAmbient(0.5, 0.5, 0.5);
			shadPass->setDiffuse(1, 1, 1, 1);
			shadPass->setSpecular(1, 1, 1, 1);

			// Texture unit state for the main texture...
			TextureUnitState* tus = shadPass->createTextureUnitState("jorge.png");

			// Set replace on all first layer textures for now
			tus->setTextureAddressingMode(TextureUnitState::TAM_WRAP);
			tus->setTextureCoordSet(0);
			tus->setTextureFiltering(TFO_BILINEAR);

			tus->setTextureUScale(1.0f);
			tus->setTextureVScale(1.0f);

			// No dynamic lighting
			shadMat->setLightingEnabled(false);
			
			shadMat->load();
			addWorldMaterial(shadMat);
		}
	}

	// ---------------------------------------------------------------------
	Ogre::String WorldRepService::getMaterialName(int mat_index) {
		std::string path = "";
		if ((mTextures[mat_index].fam != 0) && ((mTextures[mat_index].fam - 1) < (int)mTxlistHeader.fam_count)) {
			path = mFamilies[mTextures[mat_index].fam - 1].name;
			path += "/";
		}

		path += mTextures[mat_index].name; // TODO: Multiplatformness... is it solved by ogre?

		return path;
	}

	//-------------------------- Factory implementation
	std::string WorldRepServiceFactory::mName = "WorldRepService";

	WorldRepServiceFactory::WorldRepServiceFactory() : ServiceFactory() {
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
}
