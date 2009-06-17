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
 *	  $Id$
 *
 *****************************************************************************/

#include "ServiceCommon.h"
#include "LightService.h"
#include "OpdeServiceManager.h"

using namespace std;
using namespace Ogre;

namespace Opde {
	/*----------------------------------------------------*/
	/*-------------------- LightService ------------------*/
	/*----------------------------------------------------*/
	LightService::LightService(ServiceManager *manager, const std::string& name) :
		Service(manager, name), mLightPixelSize(0) {

		mAtlasList = new LightAtlasList();
	}


	//------------------------------------------------------
	LightService::~LightService() {
		clear();
		
		delete mAtlasList;
	}


	//------------------------------------------------------
	void LightService::setLightPixelSize(size_t size) {
		mLightPixelSize = size;
	}


	//------------------------------------------------------
	void LightService::build() {
		atlasLightMaps();

		mAtlasList->render();
	}


	//------------------------------------------------------
	void LightService::clear() {
		// TODO: mAtlasList->clear();

		// TODO: To be totally clean we would have to release all the created lights!
		// for now, this is done in WR - it calls clearScene
		mLights.clear();
	}

	//------------------------------------------------------
	bool LightService::init() {
		// nothing so far...
		mRenderService = GET_SERVICE(RenderService);
		mSceneMgr = mRenderService->getSceneManager();

		return true;
	}

	//------------------------------------------------------
	LightsForCellPtr LightService::_loadLightDefinitionsForCell(size_t cellID, const FilePtr& tag,
	        size_t num_anim_lights, size_t num_textured, WRPolygonTexturing* face_infos) {
		assert(mLightPixelSize != 0);

		// create a new cell, load the definitions for it
		LightsForCellPtr lfc(new LightsForCell(tag, num_anim_lights, num_textured, mLightPixelSize, face_infos));

		mLightsForCell.insert(make_pair(cellID, lfc));

		return lfc;
	}


	//------------------------------------------------------
	void LightService::_loadTableFromTagFile(const FilePtr& tag) {
		// two counts - static lights, dynamic lights
		uint32_t nstatic, ndynamic;
		tag->read(&nstatic, sizeof(uint32_t));
		tag->read(&ndynamic, sizeof(uint32_t));

		mStaticLightCount = nstatic;
		mDynamicLightCount = ndynamic;

		// the arrays have to be empty, already
		assert(mLights.getSize() == 0);

		// now load, and create lights as we go
		mLights.grow(mStaticLightCount + mDynamicLightCount);

		for (size_t i = 0; i < mDynamicLightCount + mStaticLightCount; ++i) {
			LightTableEntry tentry(tag, mLightPixelSize != 1);

			// produce light according to the entry
			// the static lights do not traverse scene in order to calculate visibility
			// and are instead manually attached to BSP leaves. See the end of this method
			mLights[i] = _produceLight(tentry, i, i >= mStaticLightCount);
		}

		// Now we have to iterate over the cells and attach lights to all affected
		CellLightInfoMap::iterator it = mLightsForCell.begin();

		for (; it != mLightsForCell.end(); ++it) {
			// get the BSP leaf from SceneMgr
			BspNode* leaf = static_cast<DarkSceneManager*>(mSceneMgr)->getBspLeaf(it->first);

			const LightsForCellPtr& lfc = it->second;

			// for all lights in the cell
			for (size_t i = 0; i < lfc->light_count; ++i) {
				size_t lid = lfc->light_indices[i];

				if (lid < mStaticLightCount)
					mLights[lid]->affectsCell(leaf);
			}
		}
	}


	//------------------------------------------------------
	DarkLight* LightService::getLightForID(int id) {
		// TODO: Code
		return NULL;
	}


	//------------------------------------------------------
	const WRLightInfo& LightService::getLightInfo(size_t cellID, size_t faceID) {
		// find the LFC pointer for the cell
		return mLightsForCell[cellID]->getLightInfo(faceID);
	}


	//------------------------------------------------------
	size_t LightService::getAtlasForCellPolygon(size_t cellID, size_t faceID) {
		//
		return mLightsForCell[cellID]->getAtlasForPolygon(faceID);
	}


	//------------------------------------------------------------------------------------
	void LightService::atlasLightMaps() {
		// atlas all the cells
		CellLightInfoMap::iterator it = mLightsForCell.begin();

		for (; it != mLightsForCell.end(); ++it) {
			//atlas each
			it->second->atlasLightMaps(mAtlasList);
		}
	}


	//------------------------------------------------------------------------------------
	DarkLight* LightService::_produceLight(const LightTableEntry& entry, size_t id, bool dynamic) {
		String lname = String("Light") + (dynamic ? 'D' : 'S') + StringConverter::toString(id);

		DarkLight* l =  static_cast<DarkLight*>(mSceneMgr->createLight(lname));

		// set the parameters
		l->setPosition(entry.pos.x, entry.pos.y, entry.pos.z);

		if (entry.cone_inner < 0) {
			l->setType(Light::LT_POINT);
		} else {
			l->setType(Light::LT_SPOTLIGHT);
			l->setSpotlightInnerAngle(Radian(Math::ACos(entry.cone_inner)));
			l->setSpotlightOuterAngle(Radian(Math::ACos(entry.cone_outer)));
			l->setDirection(entry.rot.x, entry.rot.y, entry.rot.z);
		}

		if (entry.radius > 0) {
			l->setAttenuation(entry.radius, 1.0, 0.0, 0.0); // lights with radius - constant brightness over radius
		} else {
			Vector3 color(entry.brightness.x, entry.brightness.y, entry.brightness.z);

			// The range * 10 is just a test, but seems to be working quite nice
			l->setAttenuation(color.length() * 10, 0.0, 1.0, 0.0); // linear falloff for radius-less lights
		}

		l->setDiffuseColour(entry.brightness.x, entry.brightness.y, entry.brightness.z);
		l->setSpecularColour(entry.brightness.x, entry.brightness.y, entry.brightness.z);

		l->setIsDynamic(dynamic);

		static_cast<DarkSceneManager*>(mSceneMgr)->queueLightForUpdate(l);

		return l;
	}

	/*----------------------------------------------------*/
	/*------------------ LightsForCell -------------------*/
	/*----------------------------------------------------*/
	LightsForCell::LightsForCell(const FilePtr& file, size_t num_anim_lights, size_t num_textured, size_t light_size,
	        WRPolygonTexturing* face_infos) :
			mLightSize(light_size), 
			mNumTextured(num_textured), 
			mNumAnimLights(num_anim_lights),
		        mFaceInfos(face_infos),
			mAtlased(false) {


		// load the light id's array
		anim_map = new int16_t[mNumAnimLights];
		file->read(anim_map, sizeof(int16_t) * mNumAnimLights);

		//8. load the lightmap descriptors
		lm_infos = new WRLightInfo[mNumTextured];
		file->read(lm_infos, sizeof(WRLightInfo) * mNumTextured);

		//9. load the lightmaps
		// alloc the space for all the pointers
		lmaps = new uint8_t**[mNumTextured];
		lmcounts = new uint8_t[mNumTextured];

		size_t i;
		for (i = 0; i < mNumTextured; i++) {
			// Calculate the number of lmaps
			int lmcount = countBits(lm_infos[i].animflags) + 1;

			lmcounts[i] = lmcount;
			//10. allocate space for all the lmap pointers (1+Anim) for actual face
			lmaps[i] = new uint8_t*[lmcount];

			int lmsize = lm_infos[i].lx * lm_infos[i].ly * mLightSize;


			// for each lmap, put the ptr to data
			int lmap;


			// this stores the pointer to the anim lmaps too (all of them)
			for (lmap = 0; lmap < lmcount; lmap++) {
				// 11. Read one lightmap
				lmaps[i][lmap] = new uint8_t[lmsize];
				file->read(lmaps[i][lmap], lmsize);
			}
		}

		// list of object lights (anim+static) affecting this cell
		// _uint32 unk;
		// chunk->read(&unk, sizeof(_uint32));

		file->read(&light_count, sizeof(uint32_t));

		// 12. Light object indexes
		light_indices = new uint16_t[light_count];
		file->read(&(light_indices[0]), sizeof(uint16_t) * light_count);
	}


	//------------------------------------------------------
	LightsForCell::~LightsForCell() {
		delete[] anim_map;

		delete[] lm_infos;

		for (size_t i = 0; i < mNumTextured; i++) {

			for (size_t l = 0; l < lmcounts[i]; l++)
				delete[] lmaps[i][l];

			delete[] lmaps[i];
		}

		delete[] lmaps;
		delete[] lmcounts;

		delete[] light_indices;

		delete[] lightMaps;
	}


	//------------------------------------------------------
	void LightsForCell::atlasLightMaps(LightAtlasList* atlas) {
		assert(!mAtlased);

		int ver = mLightSize - 1;


		// Array of lmap references
		lightMaps = new LightMap*[mNumTextured];

		for (size_t face = 0; face < mNumTextured; ++face) {
			AtlasInfo info;
			LightMap *lmap = atlas->addLightmap(ver, mFaceInfos[face].txt, (char *) lmaps[face][0], lm_infos[face].lx,
			        lm_infos[face].ly, info);

			lightMaps[face] = lmap;

			// Let's iterate through the animated lmaps
			// we have anim_map (array of light id's), cell->header->anim_lights contains the count of them.
			int bit_idx = 1, lmap_order = 1;

			for (size_t anim_l = 0; anim_l < mNumAnimLights; anim_l++) {
				if ((lm_infos[face].animflags & bit_idx) > 0) {
					// There is a anim lmap for this light and face
					lmpixel *converted = LightMap::convert((char *) lmaps[face][lmap_order], lm_infos[face].lx,
					        lm_infos[face].ly, ver);

					lmap->AddSwitchableLightmap(anim_map[anim_l], converted);

					lmap_order++;
				}

				bit_idx <<= 1;
			}

		} // for each face

		mAtlased = true;
	}


	//------------------------------------------------------
	const WRLightInfo& LightsForCell::getLightInfo(size_t face_id) {
		assert(face_id < mNumTextured);

		return lm_infos[face_id];
	}


	//------------------------------------------------------
	size_t LightsForCell::getAtlasForPolygon(size_t face_id) {
		assert(mAtlased);

		assert(face_id < mNumTextured);

		return lightMaps[face_id]->getAtlasIndex();
	}


	//------------------------------------------------------------------------------------
	int LightsForCell::countBits(uint32_t src) {
		// Contributed by TNH (Telliamed):
		// Found this trick in some code by Sean Barrett [TNH]
		int count = src;
		count = (count & 0x55555555) + ((count >> 1) & 0x55555555); // max 2
		count = (count & 0x33333333) + ((count >> 2) & 0x33333333); // max 4
		count = (count + (count >> 4)) & 0x0f0f0f0f; // max 8 per 4, now 8 bits
		count = (count + (count >> 8)); // max 16 per 8 bits
		count = (count + (count >> 16)); // max 32 per 8 bits
		return count & 0xff;
	}

	//------------------------------------------------------------------------------------
	Ogre::Vector2 LightsForCell::mapUV(size_t face_id, const Ogre::Vector2& original) {
		assert(face_id < mNumTextured);

		return lightMaps[face_id]->toAtlasCoords(original);
	}


	//-------------------------- Factory implementation
	std::string LightServiceFactory::mName = "LightService";

	LightServiceFactory::LightServiceFactory() :
		ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
	}
	;

	const std::string& LightServiceFactory::getName() {
		return mName;
	}

	const uint LightServiceFactory::getMask() {
		return SERVICE_RENDERER;
	}

	Service* LightServiceFactory::createInstance(ServiceManager* manager) {
		return new LightService(manager, mName);
	}

}
