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

#include "ServiceCommon.h"
#include "MaterialService.h"
#include "logger.h"

#include <OgreException.h>
#include <OgreCommon.h>
#include <OgreMaterialManager.h>
#include <OgreMaterial.h>
#include <OgrePass.h>
#include <OgreTechnique.h>
#include <OgreStringVector.h>
#include <OgreStringConverter.h>
#include <OgreTextureManager.h>
#include <OgreTexture.h>

using namespace std;
using namespace Ogre;

namespace Opde {
	const unsigned short int MaterialService::SKY_TEXTURE_ID = 249;
	const unsigned short int MaterialService::JORGE_TEXTURE_ID = 0;

	const String MaterialService::TEMPTEXTURE_RESOURCE_GROUP = "WrTextures";


	/*----------------------------------------------------*/
	/*-------------------- MaterialService ---------------*/
	/*----------------------------------------------------*/
	template<> const size_t ServiceImpl<MaterialService>::SID = __SERVICE_ID_MATERIAL;
	
	MaterialService::MaterialService(ServiceManager *manager, const std::string& name) :
		ServiceImpl< Opde::MaterialService >(manager, name) {
	}


	//------------------------------------------------------
	MaterialService::~MaterialService() {
		clear();
	}


	//------------------------------------------------------
	void MaterialService::clear() {
		mTxtScaleMap.clear();

		// release all the materials:
		WorldMaterialMap::const_iterator it = mTemplateMaterials.begin();

		while (it != mTemplateMaterials.end()) {
			LOG_DEBUG("MaterialService::clearData Removing material %s", it->second->getName().c_str());
			MaterialManager::getSingleton().remove(it->second->getName());

			++it;
		}

		mTemplateMaterials.clear();

		delete[] mFamilies;
		mFamilies = NULL;

		delete[] mTextures;
		mTextures = NULL;
	}


	//------------------------------------------------------
	bool MaterialService::init() {
		// nothing so far...
		return true;
	}


	//------------------------------------------------------
	void MaterialService::loadMaterials(const FileGroupPtr& db) {
		if (!db->hasFile("TXLIST"))
			throw Exception(Exception::ERR_ITEM_NOT_FOUND, "Mission file does not contain Texture list chunk (TXLIST)",
			        "MaterialService::loadMaterials");

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
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("Could not load texture list : ")
					+ e.getFullDescription(), "BspLevel::loadMaterials");
		}

		// Okay, we are ready to load the materials now!

		// Our resource group
		String resourceGroup = ResourceGroupManager::getSingleton().getWorldResourceGroupName();

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
			// We seek material named: family/texture
			if (MaterialManager::getSingleton().resourceExists(path)) {
				LOG_INFO("loadMaterials: Found material definition for %s", path.c_str());
				MaterialPtr origMat = MaterialManager::getSingleton().getByName(path);

				MaterialPtr shadMat = origMat->clone(matName.str(), true, resourceGroup);

				shadMat->load();

				addWorldMaterialTemplate(id, shadMat);
			} else { // The material script was not found
				createStandardMaterial(id, matName.str(), path, resourceGroup);
			}
			// This is it. Material @templateXX created
		}


		// Initialize the flow textures (do this first so water specialisation will override)
		loadFlowTextures(db);

		createSkyHackMaterial(resourceGroup);
		createJorgeMaterial(resourceGroup);
	}


	//------------------------------------------------------
	void MaterialService::addWorldMaterialTemplate(unsigned int idx, const Ogre::MaterialPtr& material) {
		mTemplateMaterials.insert(make_pair(idx, material));

		TextureDimensions2D dimensions;

		dimensions.first = 64;
		dimensions.second = 64;

		if (material->getNumTechniques() > 0) {
			Pass *shadPass = material->getTechnique(0)->getPass(0);


			// TXT scale to fit the size texture had originally
			if (shadPass->getNumTextureUnitStates() > 0) {

				if (shadPass->getNumTextureUnitStates() > 0) {
					TextureUnitState* tus = shadPass->getTextureUnitState(0);

					try {
						dimensions = tus->getTextureDimensions();

						// register the scale
						std::pair<float, float> tscale;

						tscale.first = tus->getTextureUScale();
						tscale.second = tus->getTextureVScale();

						// register the texture scale...
						setWRTextureScale(idx, tscale);

						// reset the scale back, it is canceled out by the fact we UV map with different txt dimensions
						tus->setTextureUScale(1.0f);
						tus->setTextureVScale(1.0f);

						dimensions.first = static_cast<unsigned int> (tscale.first * dimensions.first);
						dimensions.second = static_cast<unsigned int> (tscale.second * dimensions.second);
					} catch (Ogre::Exception &e) {
						// Nothing, just log it could not be done
						LOG_ERROR("MaterialService: Error getting texture dimensions : %s", e.getFullDescription().c_str());
					}
				}
			}
		}

		LOG_INFO("MaterialService: Registered a WR template material %u - %s", idx, material->getName().c_str());

		// insert
		mTextureDimensionMap.insert(make_pair(idx, dimensions));
	}


	//------------------------------------------------------
	MaterialPtr MaterialService::getWorldMaterialTemplate(unsigned int idx) {
		// look for material per slot, return if found
		WorldMaterialMap::iterator it = mTemplateMaterials.find(idx);

		if (it != mTemplateMaterials.end()) {
			return it->second;
		}

		LOG_FATAL("MaterialService::getWorldMaterialTemplate: Could not find material for index %u", idx);
		return MaterialPtr(NULL);
	}


	//------------------------------------------------------
	Ogre::MaterialPtr MaterialService::getWorldMaterialInstance(unsigned int idx, int tag) {
		if (tag < 0) {
			MaterialPtr mat = getWorldMaterialTemplate(idx);
			return mat;
		} else {
			MaterialPtr tmat = getWorldMaterialTemplate(idx);
			MaterialPtr imat;


			// have to look for it and clone
			WorldMaterialInstanceMap::iterator it = mWorldMaterials.find(idx);

			if (it == mWorldMaterials.end()) {
				// no slot for the idx. have to clone
				std::pair<WorldMaterialInstanceMap::iterator, bool> res = mWorldMaterials.insert(make_pair(idx,
				        SlotMaterialMap()));

				it = res.first;
			}

			// now search
			SlotMaterialMap::iterator sit = it->second.find(tag);

			if (sit == it->second.end()) {
				StringUtil::StrStreamType tmp;

				tmp << "Shader" << idx << "#" << tag;

				imat = tmat->clone(tmp.str());

				prepareMaterialInstance(imat, idx, tag);

				it->second.insert(make_pair(tag, imat));
			} else {
				imat = sit->second;
			}

			return imat;
		}
	}


	//------------------------------------------------------
	void MaterialService::prepareMaterialInstance(MaterialPtr& mat, unsigned int idx, int tag) {
		if (tag < 0) // Should not be here if the polygon is sky textured
			OPDE_EXCEPT("Non-instanced material instance requested", "MaterialService::prepareMaterialInstance");

		mat->setReceiveShadows(true);

		StringUtil::StrStreamType lightmapName;
		lightmapName << "@lightmap" << tag;

		Pass *shadPass = mat->getTechnique(0)->getPass(0);

		if (shadPass->getNumTextureUnitStates() <= 1) {
			// Lightmap texture is added here
			TextureUnitState* tex = shadPass->createTextureUnitState(lightmapName.str());


			// Blend
			tex->setColourOperation(LBO_MODULATE);
			// Use 2nd texture co-ordinate set
			tex->setTextureCoordSet(1);

			// Clamp
			tex->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);


			// Switch filtering off to see lmap pixels: TFO_NONE
			tex->setTextureFiltering(TFO_BILINEAR);
		} else {
			// There is a definition of the lightmapping pass already, we only update that definition
			TextureUnitState* tex = shadPass->getTextureUnitState(1);
			tex->setTextureName(lightmapName.str());
			tex->setTextureCoordSet(1);
		}
	}

	//------------------------------------------------------
	Ogre::StringVectorPtr MaterialService::getAnimTextureNames(const String& basename, const String& resourceGroup) {
		// split the filename into pieces, find all _*.* that apply
		LOG_INFO("MaterialService: Searching for anim_txt %s", basename.c_str());

		StringVectorPtr v(new StringVector());

		String name, ext;

		StringUtil::splitBaseFilename(basename, name, ext);

		// we suppose the original exists
		v->push_back(basename);

		size_t order = 1;
		bool goOn = true;

		while (goOn) {
			goOn = false;
			String newname = name + '_' + StringConverter::toString(order++) + '.' + ext;

			LOG_DEBUG("MaterialService: anim_txt trying %s", newname.c_str());

			if (ResourceGroupManager::getSingleton().resourceExists(resourceGroup, newname)) {
				goOn = true;
				v->push_back(newname);
			}
		}

		return v;
	}

	//------------------------------------------------------
	void MaterialService::bootstrapFinished() {
		// try to create lightmap resource group used for lightmap storage
		try {
			ResourceGroupManager::getSingleton().createResourceGroup(TEMPTEXTURE_RESOURCE_GROUP);
		} catch (Exception &e) {
			LOG_ERROR("Cannot create temporary texture/materials resource group '%s'. Exception : %s",
					TEMPTEXTURE_RESOURCE_GROUP.c_str(), e.getDescription().c_str());
		}

		mTextures = NULL;
		mFamilies = NULL;

		// db listener registration
		mDbCallback = DatabaseService::ListenerPtr(new ClassCallback<DatabaseChangeMsg, MaterialService> (this, &MaterialService::onDBChange));

		mDatabaseService = GET_SERVICE(DatabaseService);

		mDatabaseService->registerListener(mDbCallback, DBP_MATERIAL);

		mLightService = GET_SERVICE(LightService);

		mConfigService = GET_SERVICE(ConfigService);
	}


	//-----------------------------------------------------------------------
	void MaterialService::loadFlowTextures(const FileGroupPtr& db) {
		// Load the TXLIST chunk from the resource mission file.
		Opde::FilePtr flow_tex;
		try {
			flow_tex = db->getFile("FLOW_TEX");
		} catch (FileException) {
			LOG_ERROR("Flow chunk does not exist. Water materials may not be correctly displayed",
					"MaterialService::loadFlowTextures");
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
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("Could not load flow textures : ")
					+ e.getFullDescription(), "MaterialService::loadFlowTextures");
		}


		// now try to load non-zero flow textures as materials
		for (int fnum = 0; fnum < flow_count; fnum++) {
			if (strlen(flows.flow[fnum].name) > 0) { // nonzero name, try to load
				// Construct the basic name of the material
				std::string matname("water/");
				matname += flows.flow[fnum].name;

				// try to find the texture definition. If found, clone to the @template + the in_texture/out_texture number
				if (MaterialManager::getSingleton().resourceExists(matname + "_in")) {
					MaterialPtr origMat = MaterialManager::getSingleton().getByName(matname + "_in");

					StringUtil::StrStreamType matName;
					matName << "@template" << flows.flow[fnum].in_texture;

					if (MaterialManager::getSingleton().resourceExists(matName.str())) {
						MaterialManager::getSingleton().remove(matName.str());
					}

					MaterialPtr shadMat = origMat->clone(matName.str());
					shadMat->load();

					addWorldMaterialTemplate(flows.flow[fnum].in_texture, shadMat);
					LOG_INFO("Flow now defined : %s (template %s_in)", matName.str().c_str(), matname.c_str());
				} else {
					LOG_ERROR("Material not found : %s_in", matname.c_str());
				}


				// OUT
				if (MaterialManager::getSingleton().resourceExists(matname + "_out")) {
					MaterialPtr origMat = MaterialManager::getSingleton().getByName(matname + "_out");

					StringUtil::StrStreamType matName;
					matName << "@template" << flows.flow[fnum].out_texture;

					if (MaterialManager::getSingleton().resourceExists(matName.str())) {
						MaterialManager::getSingleton().remove(matName.str());
					}

					MaterialPtr shadMat = origMat->clone(matName.str());
					shadMat->load();

					addWorldMaterialTemplate(flows.flow[fnum].out_texture, shadMat);
					LOG_INFO("Flow now defined : %s (template %s_in)", matName.str().c_str(), matname.c_str());
				} else {
					LOG_ERROR("Material not found : %s_out", matname.c_str());
				}

			}
		}
	}


	//-----------------------------------------------------------------------
	std::pair<float, float> MaterialService::getWRTextureScale(unsigned int idx) {
		TxtScaleMap::const_iterator it = mTxtScaleMap.find(idx);

		if (it != mTxtScaleMap.end()) {
			return it->second;
		} else {
			// Default to 1,1
			return std::pair<float, float>(1.0f, 1.0f);
		}
	}


	//-----------------------------------------------------------------------
	void MaterialService::setWRTextureScale(unsigned int idx, std::pair<float, float> scale) {
		mTxtScaleMap.insert(make_pair(idx, scale));
	}


	//-----------------------------------------------------------------------
	void MaterialService::createStandardMaterial(unsigned int idx, std::string matName, std::string textureName,
	        std::string resourceGroup) {
		Image tex;
		bool loaded = false; // indicates we were successful finding the texture

		StringVectorPtr texnames = ResourceGroupManager::getSingleton().findResourceNames(resourceGroup, textureName
		        + ".*");

		if (texnames->size() <= 0) {
			// no results, try the localised version
			// prev. path + /language/filename
			String locresname = mConfigService->getLocalisedResourcePath(textureName);

			LOG_INFO("Specified resource (%s) was not found, trying localized version: %s", textureName.c_str(), locresname.c_str());

			texnames = ResourceGroupManager::getSingleton().findResourceNames(resourceGroup, locresname
					        + ".*");
		}

		String txtfile;

		// Let's try the extensions from the extensions vector
		StringVector::iterator it = texnames->begin();

		for (; it != texnames->end(); it++) { // Try loading every given
			try {
				tex.load((*it), resourceGroup);

				TextureManager::getSingleton().loadImage(textureName, resourceGroup, tex, TEX_TYPE_2D, 5, 1.0f);

				txtfile = (*it);

				loaded = true;

				break; // we got it!
			} catch (Ogre::Exception) {
				// Nothing. We are trying more extensions
			}
		}

		if (!loaded)
			LOG_ERROR("Image %s was not found, texture will be invalid!", textureName.c_str());

		// Construct a material out of this texture. We'll just clone the material upstairs to enable lmap-txture combinations
		MaterialPtr shadMat = MaterialManager::getSingleton().create(matName, resourceGroup);

		shadMat->setReceiveShadows(true);

		Pass *shadPass = shadMat->getTechnique(0)->getPass(0);

		shadPass->setAmbient(0.5, 0.5, 0.5);
		shadPass->setDiffuse(1, 1, 1, 1);
		shadPass->setSpecular(1, 1, 1, 1);

		TextureUnitState* tus = createAnimatedTextureState(shadPass, txtfile, resourceGroup, 5);

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


		// standard size
		addWorldMaterialTemplate(idx, shadMat);
	}


	// ---------------------------------------------------------------------
	void MaterialService::createSkyHackMaterial(const Ogre::String& resourceGroup) {
		// First, we'll create the sky materials, named SkyShader and WaterShader
		// The sky material does basically only write the Z value (no color write), and should be rendered prior to any other material
		// This is because we render sky(box/sphere/dome/plane) first, and we want it to be visible through the faces textured by this material
		std::string shaderName("@template249");

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

			addWorldMaterialTemplate(SKY_TEXTURE_ID, SkyShader); // fixed at slot 249
		}
	}


	// ---------------------------------------------------------------------
	void MaterialService::createJorgeMaterial(const Ogre::String& resourceGroup) {
		std::string shaderName("@template0");

		if (!MaterialManager::getSingleton().resourceExists(shaderName)) {
			MaterialPtr shadMat = MaterialManager::getSingleton().create(shaderName, resourceGroup);
			shadMat->setReceiveShadows(true);

			Pass *shadPass = shadMat->getTechnique(0)->getPass(0);

			shadPass->setAmbient(0.5, 0.5, 0.5);
			shadPass->setDiffuse(1, 1, 1, 1);
			shadPass->setSpecular(1, 1, 1, 1);

			// Texture unit state for the main texture...
			// jorge.png is compiled-in - see the RenderService::prepareHardcodedMedia
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
			addWorldMaterialTemplate(0, shadMat); // fixed at slot 0
		}
	}


	//------------------------------------------------------
	void MaterialService::onDBChange(const DatabaseChangeMsg& m) {
		LOG_INFO("MaterialService::onDBChange called.");
		if (m.change == DBC_DROPPING) {
			clear();
			return;
		}

		if (m.change == DBC_LOADING && m.dbtype == DBT_MISSION) {
			// If there is some scene already, clear it
			clear();


			// Initialize materials here:
			loadMaterials(m.db);
		}
	}


	// ---------------------------------------------------------------------
	Ogre::String MaterialService::getMaterialName(int mat_index) {
		std::string path = "";
		if ((mTextures[mat_index].fam != 0) && ((mTextures[mat_index].fam - 1) < (int) mTxlistHeader.fam_count)) {
			path = mFamilies[mTextures[mat_index].fam - 1].name;
			path += "/";
		}

		path += mTextures[mat_index].name; // TODO: Multiplatformness... is it solved by ogre?

		return path;
	}


	//------------------------------------------------------------------------------------
	MaterialPtr MaterialService::getWRMaterialInstance(unsigned int texture, int tag, unsigned int flags) {
		MaterialPtr origMat = getWorldMaterialTemplate(texture);

		bool isSky = (texture == SKY_TEXTURE_ID);

		if (isSky) { // sky polygons have no lmapping
			tag = -1; // override for sky
		} else if (flags != 0) { // if nonzero flags are specified, this is water
			tag = -1; // directly name after the original. This will cause the material to be found
		}

		MaterialPtr shadMat = getWorldMaterialInstance(texture, tag);

		if (shadMat.isNull()) {
			LOG_ERROR("MaterialService: getWRMaterialInstance did not find material for %u[%d] - overriding to Jorge", texture, tag);

			shadMat = getWorldMaterialInstance(JORGE_TEXTURE_ID, tag);

			if (shadMat.isNull()) {
				// TODO: Exception?
				LOG_FATAL("MaterialService: getWRMaterialInstance did not find jorge for 0[%d] - this is Fatal and will probably crash", tag);
			}
		}

		return shadMat;
	}


	//------------------------------------------------------------------------------------
	MaterialService::TextureDimensions2D MaterialService::getTextureDimensions(unsigned int texture) {
		TextureDimensions2D dimensions;

		TextureDimensionMap::iterator it = mTextureDimensionMap.find(texture);

		if (it != mTextureDimensionMap.end())
			return it->second;

		LOG_ERROR("MaterialService::getTextureDimensions: Texture dimensions for %u not registered (returning 64x64)", texture);
		dimensions.first = 64;
		dimensions.second = 64;
		return dimensions;
	}

	//------------------------------------------------------------------------------------
	Ogre::TextureUnitState* MaterialService::createAnimatedTextureState(Pass* pass, const String& baseTextureName, const String& resourceGroup, float fps) {
		//
		// Texture unit state for the main texture...
		TextureUnitState* tus = pass->createTextureUnitState(baseTextureName);

		StringVectorPtr mat_textures;

		// if if was found, then we'll proceed by enumerating all the resources with the same name,
		// but with _NUMBER
		mat_textures = getAnimTextureNames(baseTextureName, resourceGroup);

		// if we have anim. textures:
		if (!mat_textures.isNull() && mat_textures->size() > 1) {
			// convert to String* array
			size_t size = mat_textures->size();

			String* sarray = new String[size];

			size_t s = 0;
			for (StringVector::iterator it = mat_textures->begin(); s < size; ++it, ++s) {
				sarray[s] = *it;
			}

			tus->setAnimatedTextureName(sarray, size, size/fps);

			delete[] sarray;
		}

		return tus;

	}

	//-------------------------- Factory implementation
	std::string MaterialServiceFactory::mName = "MaterialService";

	MaterialServiceFactory::MaterialServiceFactory() :
		ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
	}
	;


	//------------------------------------------------------
	const std::string& MaterialServiceFactory::getName() {
		return mName;
	}


	//------------------------------------------------------
	const uint MaterialServiceFactory::getMask() {
		return SERVICE_RENDERER | SERVICE_DATABASE_LISTENER;
	}
	
	//------------------------------------------------------
	const size_t MaterialServiceFactory::getSID() {
		return MaterialService::SID;
	}

	//------------------------------------------------------
	Service* MaterialServiceFactory::createInstance(ServiceManager* manager) {
		return new MaterialService(manager, mName);
	}

}
