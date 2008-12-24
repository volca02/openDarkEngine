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

/**
 @file MaterialService.h
 @brief A material service implementation. Handles terrain/object materials - loading, unloading, object service
 properties of them.
 */

#ifndef __MATERIALSERVICE_H
#define __MATERIALSERVICE_H

#include "config.h"
#include "File.h"
#include "DatabaseService.h"
#include "LightService.h"
#include "ConfigService.h"

namespace Opde {
	// --- Following are family and txlist chunk structures shamelessly ripped from Telliamed's code.
	// To be replaced by a competent reader/writer - we should have a handling class for this anyway
	struct DarkDBChunkFAMILY {
			uint32_t size; // size of each entry: 0x18
			uint32_t count; // number of entries: 0x12
			char fam[18][24];
			// The first two entries are reserved for water families
	};

	struct DarkDBChunkTXLIST {
			uint32_t length; // length of TXLIST
			uint32_t txt_count; // number of individual textures
			uint32_t fam_count; // number of families
			// array of family names; first entry is "fam"
			// array of DarkDBTXLIST_texture; first entry is "null"
	};

	struct DarkDBTXLIST_fam {
			char name[16];
	};

	struct DarkDBTXLIST_texture {
			uint8_t one; // 0x01 (except on "null" texture)
			uint8_t fam; // number of family (one-based count, 0 for no fam)
			uint16_t zero; // 0x00
			char name[16]; // texture name
	};


	/*
	 * FLOW_TEX [MIS] [*]
	 * Association of flow textures to flow colors.
	 */
	struct DarkDBChunkFLOW_TEX {
			struct {
					int16_t in_texture; // index in texture palette
					int16_t out_texture; // index in texture palette
					char name[28];
			} flow[256];
	};


	/** @brief Material Service - Service which handles materials for terrain and objects - their loading, unloading, cloning, etc. */
	class OPDELIB_EXPORT MaterialService: public Service {
		public:
			/** Constructor
			 * @param manager The ServiceManager that created this service
			 * @param name The name this service should have (For Debugging/Logging)
			 */
			MaterialService(ServiceManager *manager, const std::string& name);

			/// Destructor
			virtual ~MaterialService();

			/** Returns a prepared material pointer for combination texture/tag
			*/
			Ogre::MaterialPtr getWRMaterialInstance(unsigned int texture, int tag, unsigned int flags);

			typedef std::pair<Ogre::uint, Ogre::uint> TextureDimensions2D;

			/** Getter for WR texture size (after scaling with the custom material parameters)
			 * @param texture The wr texture id
			 */
			TextureDimensions2D getTextureDimensions(unsigned int texture);

			/** Prepares a single TextureUnitState filled with all the animation frames of the loaded image set.
			 * Searches for all images that have the same image name, or have a _NUMBER added to the filename.
			 * If none additional textures found, the result is a steady texture specified.
			 *
			 * @param skeleton The skeletal material to fill with the animation/static texture
			 * @param baseTextureName The base name of the texture, incl. the extension
			 * @param fps The desired FPS of the animation
			 * @return TextureUnitState pointer which was filled with the animation/static texture
			 * */
			Ogre::TextureUnitState* createAnimatedTextureState(Ogre::Pass* pass, const Ogre::String& baseTextureName, const Ogre::String& resourceGroup,  float fps);

			/// Creates a model material

			static const unsigned short int SKY_TEXTURE_ID;
			static const unsigned short int JORGE_TEXTURE_ID;

			static const Ogre::String TEMPTEXTURE_RESOURCE_GROUP;

		protected:
			/// Service initialization
			bool init();

			/// removes all the stored lights
			void clear();

			/// @see Service::bootstrapFinished
			void bootstrapFinished();

			/// DB load/unload event callback method
			void onDBChange(const DatabaseChangeMsg& m);

			/** Called by loadMaterials. Load the FLOW_TEX and initializes \@templateXXXX according to IN and OUT texture numbers and names. Needs material definitions
			 * named water/AAAA_in and water/AAAA_out where AAAA is the expected flow texture name (usualy bl, gr, l2, l3, l4) */
			void loadFlowTextures(const FileGroupPtr& db);

			/** Internal method which loads the textures from the dark database definitions. Families, textures and flowgroups
			 * Texture preparation - prepares materials in the form of \@templateXXXX where XXXX is the texture number */
			void loadMaterials(const FileGroupPtr& db);

			/** A fail-back method, which construct a default-valued material, if a material script for a certain family/texture was not found */
			void createStandardMaterial(unsigned int idx, std::string matName, std::string textureName, std::string resourceGroup);

			/** Internal method. Creates skyhack material */
			void createSkyHackMaterial(const Ogre::String& resourceGroup);

			/** Internal method. Creates Jorge material (template0) from internal jorge.png */
			void createJorgeMaterial(const Ogre::String& resourceGroup);

			/// registers the specified material as a terrain material
			void addWorldMaterialTemplate(unsigned int idx, const Ogre::MaterialPtr& material);

			/// Gets the material template for the given material slot index
			Ogre::MaterialPtr getWorldMaterialTemplate(unsigned int idx);

			/// Gets the material instance for the given material slot index and tag (autocreates, but with no modifications - clone only)
			Ogre::MaterialPtr getWorldMaterialInstance(unsigned int idx, int tag);

			/// modifies the material to contain lightmap
			void prepareMaterialInstance(Ogre::MaterialPtr& mat, unsigned int idx, int tag);

			/** Creates a vector containing all accessible animated textures in a sequence.
			 * This means that given Eng_3/GOO.PCX, this method will return Eng_3/GOO.PCX, Eng_3/GOO_1.PCX Eng_3/GOO_2.PCX Eng_3/GOO_3.PCX strings (as found in ss2's fam.crf)
			 * */
			Ogre::StringVectorPtr getAnimTextureNames(const Ogre::String& basename, const Ogre::String& resourceGroup);

			/** get the material name from it's index
			* @note name is formed from FAMILY and NAME like this: FAMILY/NAME or NAME (for no-family textures) */
			Ogre::String getMaterialName(int mat_index);

			// Texture scale getter (for custom texture overrides)
			std::pair<float, float> getWRTextureScale(unsigned int idx);

			// Texture scale setter (for custom texture overrides)
			void setWRTextureScale(unsigned int idx, std::pair<float, float> scale);

			/// Database callback
			DatabaseService::ListenerPtr mDbCallback;

			/// Database service
			DatabaseServicePtr mDatabaseService;

			/// Set of loaded materials
			typedef std::vector<Ogre::MaterialPtr> MaterialList;

			typedef std::map<unsigned int, Ogre::MaterialPtr> WorldMaterialMap;

			typedef std::map<unsigned int, Ogre::MaterialPtr> SlotMaterialMap;

			typedef std::map<unsigned int, SlotMaterialMap > WorldMaterialInstanceMap;

			/// Map of Texture sizes (this is needed for the WR texturing to work) for WR
			typedef std::map<unsigned int, std::pair<float, float> > TxtScaleMap;

			/// Map of texture dimensions, after user-defined scaling
			typedef std::map<unsigned int, TextureDimensions2D > TextureDimensionMap;


			/// map of the texture sizes (fam/texture -> size). TODO: Evaluate the possibility to always scale to predefined dimensions...
			TxtScaleMap mTxtScaleMap;

			TextureDimensionMap mTextureDimensionMap;

			/// map of material templates per material id
			WorldMaterialMap mTemplateMaterials;

			WorldMaterialInstanceMap mWorldMaterials;

			/// Material name - the family part
			DarkDBTXLIST_fam* mFamilies;

			/// Material name - the texture part
			DarkDBTXLIST_texture* mTextures;

			/// Material TXLIST header
			DarkDBChunkTXLIST mTxlistHeader;

			/// reference to light service (for material instancing)
			LightServicePtr mLightService;

			/// Reference to config service (for language string setting)
			ConfigServicePtr mConfigService;
	};


	/// Shared pointer to light service
	typedef shared_ptr<MaterialService> MaterialServicePtr;


	/// Factory for the LightService objects
	class OPDELIB_EXPORT MaterialServiceFactory: public ServiceFactory {
		public:
			MaterialServiceFactory();
			~MaterialServiceFactory() {
			}
			;

			/** Creates a GameService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

			virtual const uint getMask();

		private:
			static std::string mName;
	};
}

#endif
