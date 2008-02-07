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


#include "compat.h"
#include "integers.h"
#include "LightmapAtlas.h"
#include "OgreStringConverter.h"
#include "ConsoleBackend.h"
#include "WRCommon.h"
// #define framelm
#include "OpdeException.h"

using namespace Ogre;

namespace Opde {

	Vector3 operator*(float a, lmpixel b) {
		return Vector3(a * b.R,a * b.G,a * b.B);
	}

	// ---------------- LightAtlas Methods ----------------------
	/*
		A single texture, containing many lightmaps. used in the texturelist construction
	*/
	LightAtlas::LightAtlas(int idx) {
		mName << "@lightmap" << idx; // so we can find the atlas by number in the returned AtlasInfo

		this->mIdx = idx;

		mCount = 0;

		// initialise the free space - initially whole lmap
		mFreeSpace = new FreeSpaceInfo(0,0,ATLAS_WIDTH,ATLAS_HEIGHT);

		// I Place the lightmaps into a separate resource group for easy unloading
		mTex = TextureManager::getSingleton().createManual(
			mName.str(), TEMPTEXTURE_RESOURCE_GROUP,
			TEX_TYPE_2D, ATLAS_WIDTH, ATLAS_HEIGHT, 0, PF_X8R8G8B8,
			TU_DYNAMIC_WRITE_ONLY);

		mAtlas = mTex->getBuffer(0, 0);

		// Erase the lmap atlas pixel buffer
		mAtlas->lock(HardwareBuffer::HBL_DISCARD);
		const PixelBox &pb = mAtlas->getCurrentLock();

		for (int y = 0; y < ATLAS_HEIGHT; y++) {
			uint32 *data = static_cast<uint32*>(pb.data) + y*pb.rowPitch;

			for (int x = 0; x < ATLAS_WIDTH; x++)
				data[x] = 0;
		}

		mAtlas->unlock();
	}


	LightAtlas::~LightAtlas() {
		// TODO: for_each(lightmaps.begin(), lightmaps.end(), delete lmap);
		delete mFreeSpace;

        TextureManager::getSingleton().remove(mName.str());

        // delete all the lmaps.
        std::vector< LightMap* >::iterator it = mLightmaps.begin();

        while (it != mLightmaps.end()) {
            delete (*it);
            ++it;
        }

        mLightmaps.clear();
	}

	bool LightAtlas::addLightMap(LightMap *lmap) {
		std::pair<int, int> dim = lmap->getDimensions();

		// get the origin for our lmap
		FreeSpaceInfo* area = mFreeSpace->allocate(dim.first, dim.second);

		if (area == NULL) // didn't fit
			return false;

		// calculate some important UV conversion data
		// +0.5? to display only the inner transition of lmap texture, the outer goes to black color
		lmap->mUV.x = ((float)area->x + 0.5 ) / ATLAS_WIDTH;
		lmap->mUV.y = ((float)area->y + 0.5 ) / ATLAS_HEIGHT;

		// size conversion to atlas coords
		lmap->mSizeUV.x = ((float)dim.first - 1)/ATLAS_WIDTH;
		lmap->mSizeUV.y = ((float)dim.second - 1)/ATLAS_HEIGHT;

		// finally increment the count of stored lightmaps
		mCount++;
		mLightmaps.push_back(lmap);

		lmap->setPlacement(this, area);

		return true;
	}

	bool LightAtlas::render() {
		mAtlas->lock(HardwareBuffer::HBL_DISCARD);

		std::vector< LightMap * >::const_iterator lmaps_it = mLightmaps.begin();

		for (; lmaps_it != mLightmaps.end(); ++lmaps_it)
			(*lmaps_it)->refresh();

		mAtlas->unlock();

		return true;
	}

	void LightAtlas::updateLightMapBuffer(FreeSpaceInfo& fsi, lmpixel* rgb) {
		const PixelBox &pb = mAtlas->getCurrentLock();

		for (int y = 0; y < fsi.h; y++) {
			uint32 *data = static_cast<uint32*>(pb.data) + (y + fsi.y)*pb.rowPitch;

			for (int x = 0; x < fsi.w; x++)
				data[x + fsi.x] = rgb[x + y * fsi.w].ARGB(); // Write a A8R8G8B8 conversion of the lmpixel
		}
	}

	void LightAtlas::updateLightMapBuffer(FreeSpaceInfo& fsi, Ogre::Vector3* rgb) {
		const PixelBox &pb = mAtlas->getCurrentLock();

		for (int y = 0; y < fsi.h; y++) {
			uint32 *data = static_cast<uint32*>(pb.data) + (y + fsi.y)*pb.rowPitch;

			for (int x = 0; x < fsi.w; x++) {
				Vector3 pixel = rgb[x + (y * fsi.w)];

				uint32 R, G, B;

				R = (uint32) pixel.x;
				G = (uint32) pixel.y;
				B = (uint32) pixel.z;

				R = (R > 255) ? 255 : ((R < 0)? 0 : R);
				G = (G > 255) ? 255 : ((G < 0)? 0 : G);
				B = (B > 255) ? 255 : ((B < 0)? 0 : B);

				uint32 ARGB = (R << 16) | (G << 8) | B;

				// Write a A8R8G8B8 conversion of the lmpixel
				data[x + fsi.x] = ARGB;
			}
		}
	}

	void LightAtlas::registerAnimLight(int id, LightMap* target) {
		// Try to find the set, if not make a new one...
		std::map< int, std::set<LightMap*> >::iterator light_it = mLights.find(id);

		if (light_it != mLights.end()) {
			light_it->second.insert(target);
		} else {
			std::set<LightMap *> new_set;
			new_set.insert(target);
			mLights.insert(std::make_pair(id, new_set));
		}
	}

	void LightAtlas::setLightIntensity(int id, float intensity) {
		// iterate throught the map lights, and call the setLightIntensity on all registered lightmaps
		std::map< int, std::set<LightMap*> >::iterator light_it = mLights.find(id);

		if (light_it != mLights.end()) {
			mAtlas->lock(HardwareBuffer::HBL_DISCARD);

			std::set<LightMap *>::const_iterator lmaps_it = light_it->second.begin();

			for (;lmaps_it != light_it->second.end(); lmaps_it++)
				(*lmaps_it)->setLightIntensity(id, intensity);

			mAtlas->unlock();
		}
	}

	int LightAtlas::getIndex() {
		return mIdx;
	}


	int LightAtlas::getUnusedArea() {
		return mFreeSpace->getLeafArea();
	}

	// ---------------- LightAtlasList Methods ----------------------
	LightAtlasList::LightAtlasList() {
		Opde::ConsoleBackend::getSingleton().registerCommandListener(std::string("light"), dynamic_cast<ConsoleCommandListener*>(this));
		Opde::ConsoleBackend::getSingleton().setCommandHint(std::string("light"), "Switch a light intensity: light LIGHT_NUMBER INTENSITY(0-1)");
		Opde::ConsoleBackend::getSingleton().registerCommandListener(std::string("lmeff"), dynamic_cast<ConsoleCommandListener*>(this));
		Opde::ConsoleBackend::getSingleton().setCommandHint(std::string("lmeff"), "lightmap atlas efficiency calculator");
		// TODO: short description of command could be useful (registerCommandListener(command, desc, classptr))

		mSplitCount = 0;
	}

	LightAtlasList::~LightAtlasList() {
		std::vector< LightAtlas* >::iterator it = mList.begin();

		for (; it != mList.end(); ++it)
			delete *it;

		mList.clear();
	}

	bool LightAtlasList::placeLightMap(LightMap* lmap) {
		if (mList.size() == 0)
			mList.push_back(new LightAtlas(0));

		int last = mList.size();

		// iterate through existing Atlases, and see if any of them accepts our lightmap
		for (int i = 0; i < last; i++) {
				if (mList.at(i)->addLightMap(lmap))
					return false;
		}

		// add new atlas to list if none of them accepted the lmap
		LightAtlas* la = new LightAtlas(last);
		mList.push_back(la);

		la->addLightMap(lmap);
		return true;

	}

	LightMap* LightAtlasList::addLightmap(int ver, int texture, char *buf, int w, int h, AtlasInfo &destinfo) {
		// convert the pixel representation
		lmpixel *cdata = LightMap::convert(buf, w, h, ver);

		// construct the lmap
		LightMap* lmap = new LightMap(w, h, cdata);

		// Try to insert, will find existing if already there
		std::pair<TextureLightMapQueue::iterator, bool> lmm;

		// Insert the lightmap to the queue
		lmm = mLightMapQueue.insert(
		    TextureLightMapQueue::value_type(texture, std::vector<LightMap*>())
		  );


		lmm.first->second.push_back(lmap);

		return lmap;
	}

	int LightAtlasList::getCount() {
		return mList.size();
	}

	bool LightAtlasList::render() {
		/*
		TODO: The performance of the renderer highly depends on the atlasing used.
		* The reason is that each material is processed separately. As each atlas produces one texture, the same-material based lightmaps should be in the same atlases.
		* There should be as little atlasses as possible for the same reason.
		- If atlasing takes the lightmaps as they go, a low number of atlases is produced, but the lightmap categories are highly mixed
		- If the lightmaps are put into atlases per thei're texture number, the result is good, but the redundacy is killing memory a bit (90% unused pixel space common)
		The solution is to atlas the lightmaps after all those are inserted (there is no guarantee that those will come in material index order), And doing so in a material order
		*/

		// Step 1. Atlas the queue
		TextureLightMapQueue::iterator ti = mLightMapQueue.begin();

		// for all materials
		for (; ti != mLightMapQueue.end(); ti++) {
			// sort the lmap vector by the area of the lmaps, and insert those all
			std::sort(ti->second.begin(), ti->second.end(), lightMapLess());

			bool first = true;

			// find a nice warm place for all those little lightmaps
			for (std::vector< LightMap *>::iterator it = ti->second.begin(); it < ti->second.end(); it++, first = false) {
				if (placeLightMap(*it) && !first)
					mSplitCount++;
			}
		}

		// Step2. Render
		for (std::vector<LightAtlas *>::iterator it = mList.begin(); it != mList.end(); ++it)
			if (!(*it)->render())
				OPDE_EXCEPT("Could not render the lightmaps!","LightAtlasList::render()");


		return true;
	}

	void LightAtlasList::commandExecuted(std::string command, std::string parameters) {
		if (command == "light") {
			// Split the parameters
			size_t space_pos = parameters.find(' ');

			if (space_pos != std::string::npos) {
				// First substring to command, second to params
				std::string s_light = parameters.substr(0,space_pos);
				std::string s_intensity = parameters.substr(space_pos+1, parameters.length() - (space_pos + 1));

				// TODO: Use logger for this.
				Opde::ConsoleBackend::getSingleton().putMessage(std::string("Doing light change"));

				int light = StringConverter::parseInt(s_light);
				float intensity = StringConverter::parseReal(s_intensity);

				// apply
				setLightIntensity(light, intensity);
			}
		} else if (command == "lmeff") {
			// calculate the lightmap coverage efficiency - per atlas and global one (average)
			// LOG_ERROR("Lmeff not implemented yet");
			int total_pixels = 0;
			int unused_pixels = 0;

			int last = mList.size();

			// iterate through existing Atlases, and see if any of them accepts our lightmap
			for (int i = 0; i < last; i++) {
				unused_pixels += mList.at(i)->getUnusedArea();
				total_pixels += ATLAS_WIDTH * ATLAS_HEIGHT;
			}

			float percentage = 0;

			if (total_pixels > 0)
				percentage = 100.0f * unused_pixels / total_pixels;

			LOG_INFO("Total unused light map atlasses pixels: %d of %d (%f%%). %d atlases total used. Total splits: %d", unused_pixels, total_pixels, percentage, last, mSplitCount);

		} else LOG_ERROR("Command %s not understood by LightAtlasList", command.c_str());
	}

	bool LightAtlasList::lightMapLess::operator() (const LightMap* a, const LightMap* b) const {
		std::pair<int, int> s1, s2;

		s1 = a->getDimensions();
		s2 = b->getDimensions();

		int area1 = s1.first * s1.second;
		int area2 = s2.first * s2.second;

		return (area1 < area2);
	}

	// ------------------------------- Lightmap class
	void LightMap::refresh() {
		// First we calculate the new version of the lmap. Then we post it to the atlas
		// Float version of lmap
		// TODO: Int will be sufficient and quicker
		Vector3* f_lmap = new Vector3[mSizeX * mSizeY];

		// copy the static lmap...
		for (unsigned int i = 0; i < mSizeX * mSizeY; i++) {
			f_lmap[i].x = mStaticLmap[i].R;
			f_lmap[i].y = mStaticLmap[i].G;
			f_lmap[i].z = mStaticLmap[i].B;
		}


		ObjectToLightMap::const_iterator lmap_it = mSwitchableLmaps.begin();

		for  (;lmap_it != mSwitchableLmaps.end(); ++lmap_it) {
			lmpixel* act_lmap = lmap_it->second;

			int light_id = lmap_it->first;

			float intensity = 0;

			std::map<int, float>::iterator intens_it = mIntensities.find(light_id);

			// Just to be sure we get the intensity
			if (intens_it == mIntensities.end())  {
				mIntensities.insert(std::make_pair(light_id, 0.0f));
			} else {
				intensity = intens_it->second;
			}

			if (intensity > 0) {
				for (unsigned int i = 0; i < mSizeX * mSizeY; i++) {
					f_lmap[i] += intensity * act_lmap[i];
				}
			}
		}

		mOwner->updateLightMapBuffer(*mPosition, f_lmap);

		// Get rid of the helping array
		delete[] f_lmap;
	}

	lmpixel* LightMap::convert(char *data, int sx, int sy, int ver) {
		lmpixel *result = new lmpixel[sx * sy];

		// old version - grayscale
		if (ver == 0) {
			for (int i = 0; i < (sx * sy); i++) {
				result[i].R = data[i];
				result[i].G = data[i];
				result[i].B = data[i];
			}
		} else {
			uint16* buf_ptr = (uint16*) data;

			for (int i = 0; i < (sx * sy); i++) {
				unsigned int xBGR = buf_ptr[i];

				result[i].R = (xBGR & 0x0001f) << 3;
				result[i].G = ((xBGR >> 5) & 0x0001f) << 3;
				result[i].B = ((xBGR >> 10) & 0x0001f) << 3;
			}
		}

		return result;
	}

	void LightMap::AddSwitchableLightmap(int id, lmpixel *data) {
		mSwitchableLmaps.insert(std::make_pair(id, data));
		mIntensities[id] = 0.0f;
	}

	void LightMap::setLightIntensity(int id, float intensity) {
		std::map<int, float>::iterator intens = mIntensities.find(id);

		if (intens != mIntensities.end()) {
			// only if the value changed
			if (intens->second != intensity) {
				intens->second = intensity;

				refresh();
			}
		}
	}

	int LightMap::getAtlasIndex() {
		return mOwner->getIndex();
	}


	void LightMap::setPlacement(LightAtlas* _owner, FreeSpaceInfo* tgt) {
		mOwner = _owner;
		mPosition = tgt;

		// register as a light related lightmap
		std::map<int, float>::iterator it = mIntensities.begin();

		for (; it != mIntensities.end(); it++) {
			mOwner->registerAnimLight(it->first, this);
		}
	}

	std::pair<int, int> LightMap::getDimensions() const {
		return std::pair<int,int>(mSizeX, mSizeY);
	}
} // namespace ogre

