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
 *****************************************************************************/


#ifndef __WORLDREPSERVICE_H
#define __WORLDREPSERVICE_H

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "Ogre.h"
#include "OgreDarkSceneNode.h"
#include "WRTypes.h"
#include "WRCell.h"
#include "LightmapAtlas.h"
#include "integers.h"
#include "DatabaseService.h"
#include "Callback.h"

#include <OgreHardwareBufferManager.h>
#include <OgreDefaultHardwareBufferManager.h>
#include <OgreStaticFaceGroup.h>
#include <vector>

#include "FileGroup.h"
#include "DarkDBDefs.h"
#include "SharedPtr.h"

// The name of the group that stores the built textures and materials
#define TEMPTEXTURE_RESOURCE_GROUP "WrTextures"

using namespace Dark;

namespace Opde {

	/** @brief WorldRep service - Level geometry loader.
	*
	* This service is responsible for the level geometry initialization.
	* @note Should handle world-geometry releted methods later on. For example - Light switching */
	class WorldRepService : public Service {
		public:
			/** Initializes the Service */
			WorldRepService(ServiceManager* manager);

			/** Destructs the WorldRepService instance, and unallocates the data, if any. */
			virtual ~WorldRepService();

			Ogre::SceneManager *getSceneManager();

			/** get the material name from it's index
			* @note name is formed from FAMILY and NAME like this: FAMILY/NAME or NAME (for no-family textures) */
			Ogre::String getMaterialName(int mat_index);

            void addWorldMaterial(const Ogre::MaterialPtr material);
		protected:
            virtual void bootstrapFinished();

			/// Database change callback
			void onDBChange(const DatabaseChangeMsg& m);

            /** Internal method. Clears all the used data and scene */
			void clearData();

			/** Unloads the worldrep. Clears the scene */
			void unload();

			/** Internal method which loads the data from the found WR/WRRGB chunk, and constructs level geometry for the SceneManager */
			void loadFromChunk(FilePtr& wrChunk, int lightSize);

			/** Sets sky box according to the SKYMODE chunk contents. Does not do NewSky */
			void setSkyBox(FileGroupPtr db);

			/** A failback method, which construct a default-valued material, if a material script for a certain family/texture was not found */
			void createStandardMaterial(std::string matName, std::string textureName, std::string resourceGroup);

			/** Called by loadMaterials. Load the FLOW_TEX and initializes \@templateXXXX according to IN and OUT texture numbers and names. Needs material definitions
			* named water/AAAA_in and water/AAAA_out where AAAA is the expected flow texture name (usualy bl, gr, l2, l3, l4) */
			void loadFlowTextures(FileGroupPtr db);

			/** Internal method which loads the textures from the dark database definitions. Families, textures and flowgroups
			* Texture preparation - prepares materials in the form of \@templateXXXX where XXXX is the texture number */
			void loadMaterials(FileGroupPtr db);

			/** Internal method. Creates a BspNode instance tree, and supplies it to the sceneManager */
			void createBSP(unsigned int BspRows, wr_BSP_node_t *tree);

			/** Internal method. Constructs Ogre plane out of worldrep plane */
			Ogre::Plane constructPlane(wr_plane_t plane);

			/** Internal method. Creates skyhack material */
			void createSkyHack(Ogre::String resourceGroup);

			// TODO: Move this away... RenderService->getRoot?
			Ogre::Root *mRoot;

			// TODO: Move this away... RenderService->getSceneManager
			Ogre::DarkSceneManager *mSceneMgr;

			/** Lightmap atlas list */
			LightAtlasList *mAtlas;

			/** Loaded structure of the cells */
			WRCell** mCells;

			/** Bsp Leaf nodes */
			Ogre::BspNode* mLeafNodes;

			/** Bsp non-leaf nodes */
			Ogre::BspNode* mNonLeafNodes;

			uint32_t mExtraPlaneCount;
			wr_plane_t* mExtraPlanes;

			/** Cell count from header */
			uint32_t mNumCells;

			std::set< Ogre::String > mTextureExtensions;

			/// -- The vertex and index buffers
			Ogre::VertexData* mVertexData;

			/// system-memory buffer
			Ogre::HardwareIndexBufferSharedPtr mIndexes;

			/// Face groups
			Ogre::StaticFaceGroup* mFaceGroups;

			/// Material name - the family part
			DarkDBTXLIST_fam* mFamilies;

			/// Material name - the texture part
			DarkDBTXLIST_texture* mTextures;

			/// Material TXLIST header
			DarkDBChunkTXLIST mTxlistHeader;

            /// Database callback
            DatabaseService::ListenerPtr mDbCallback;

            /// Database service
            DatabaseServicePtr mDatabaseService;

            /// Set of loaded materials
            typedef std::vector< Ogre::MaterialPtr > MaterialList;

            /// Gets filled with all the materials loaded (for unloading)
            MaterialList mLoadedMaterials;
	};

	/// Shared pointer to worldrep service
	typedef shared_ptr<WorldRepService> WorldRepServicePtr;

	/// Factory for the WorldRep service
	class WorldRepServiceFactory : public ServiceFactory {
		public:
			WorldRepServiceFactory();
			~WorldRepServiceFactory() {};

			/** Creates a WorldRepService instance */
			Service* createInstance(ServiceManager* manager);

			const std::string& getName();

            virtual const uint getMask();

		private:
			static std::string mName;
	};
}


#endif
