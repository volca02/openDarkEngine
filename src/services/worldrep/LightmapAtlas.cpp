/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005 Filip Volejnik <f.volejnik@centrum.cz>
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
 *****************************************************************************/
 
#include "LightmapAtlas.h"
#include "OgreStringConverter.h"
#include "ConsoleBackend.h"

// #define framelm

using namespace Ogre;

namespace Opde {
	
	Vector3 operator*(float a, lmpixel b) {
		return Vector3(a * b.R,a * b.G,a * b.B);
	}
	
	// ---------------- LightAtlas Methods ----------------------
	/*
		A single texture, containing many lightmaps. used in the texturelist construction
	*/
	LightAtlas::LightAtlas(int category, int idx) {
		name << "@lightmap" << idx; // so we can find the atlas by number in the returned AtlasInfo
		this->category = category;  // category can be used for texture number f.e. - e.g. to stop mixing the texture<>lmap in a big scale (we drop down the number of resulting combinations)
		
		this->idx = idx;
		
		count = 0;
	
		// initialise the free space - initially whole lmap
		freeSpace.push_back(new FreeSpaceInfo(0,0,ATLAS_lmsize,ATLAS_lmsize));
		
		
		ptex = TextureManager::getSingleton().createManual(
			name.str(),ResourceGroupManager::getSingleton().getWorldResourceGroupName(), 
			TEX_TYPE_2D, ATLAS_lmsize, ATLAS_lmsize, 0, PF_A8R8G8B8, 
			TU_DYNAMIC_WRITE_ONLY);
		
		atlas = ptex->getBuffer(0, 0);
		
		// Erase the lmap atlas pixel buffer
		atlas->lock(HardwareBuffer::HBL_DISCARD);
		const PixelBox &pb = atlas->getCurrentLock();
		
		for (int y = 0; y < ATLAS_lmsize; y++) {
			uint32 *data = static_cast<uint32*>(pb.data) + y*pb.rowPitch;
			
			for (int x = 0; x < ATLAS_lmsize; x++) 
				data[x] = 0;
			
		}
		
		atlas->unlock();
	}
	
	bool LightAtlas::canInsert(int w, int h) {
		// no free space
		if (freeSpace.size() == 0) return false;
		
		int last = freeSpace.size();
		// so, we should also do the actual work here, no need to search twice
		
		// iterate through free spaces
		for (int i = 0; i < last; i++) {
			if (freeSpace.at(i)->Fits(w,h))
				return true;
		}
	
		return false;
	}
	
	// get origin for the new lightmap inserted
	FreeSpaceInfo LightAtlas::getOrigin(int w, int h) {
		int last = freeSpace.size();
		// so, we should also do the actual work here, no need to search twice
		
		// iterate through free spaces
		for (int i = 0; i < last; i++) {
			FreeSpaceInfo dest;
			
			if (freeSpace.at(i)->Crop(w, h, freeSpace, i, &dest)) 
				return dest;
		}
		
		// TODO: Assertation?!
		return FreeSpaceInfo(0,0,-1,-1); // this is an error!
	}
	
	
	LightMap* LightAtlas::addLightmap(int ver, char *buf, int w, int h, AtlasInfo &destinfo) {
		if (!canInsert(w,h)) return NULL;
	
		// get the origin for our lmap
		FreeSpaceInfo origin = getOrigin(w,h);
		
		lmpixel *converted_data = LightMap::convert(buf, w, h, ver);
		
		LightMap* light_map = new LightMap(origin, this, w, h, converted_data);
	
		// calculate some important UV conversion data
		// +0.5? to display only the inner transition of lmap texture, the outer goes to black color
		light_map->uv.x = ((float)origin.x + 0.5 ) / ATLAS_lmsize;
		light_map->uv.y = ((float)origin.y + 0.5 ) / ATLAS_lmsize;
		
		// size conversion to atlas coords
		light_map->suv.x = ((float)w - 1)/ATLAS_lmsize;
		light_map->suv.y = ((float)h - 1)/ATLAS_lmsize;
	
		// finally increment the count of stored lightmaps
		count++;
		lightmaps.push_back(light_map); 
		
		return light_map;
	}
	
	bool LightAtlas::render() {
		atlas->lock(HardwareBuffer::HBL_DISCARD);
		
		std::vector< LightMap * >::const_iterator lmaps_it = lightmaps.begin();
		
		for (; lmaps_it != lightmaps.end(); lmaps_it++)
			(*lmaps_it)->refresh();
		
		atlas->unlock();
		
		return true;
	}
	
	void LightAtlas::updateLightMapBuffer(FreeSpaceInfo& fsi, lmpixel* rgb) {
		// atlas->lock(HardwareBuffer::HBL_DISCARD);
		
		const PixelBox &pb = atlas->getCurrentLock();
		
		for (int y = 0; y < fsi.h; y++) {
			uint32 *data = static_cast<uint32*>(pb.data) + (y + fsi.y)*pb.rowPitch;
			
			for (int x = 0; x < fsi.w; x++)
				data[x + fsi.x] = rgb[x + y * fsi.w].ARGB(); // Write a A8R8G8B8 conversion of the lmpixel
		}
					
		// atlas->unlock();
	}
	
	void LightAtlas::updateLightMapBuffer(FreeSpaceInfo& fsi, Vector3* rgb) {
		// atlas->lock(HardwareBuffer::HBL_DISCARD);
		
		const PixelBox &pb = atlas->getCurrentLock();
		
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
					
		//atlas->unlock();
	}
	
	void LightAtlas::registerAnimLight(int id, LightMap* target) {
		// Try to find the set, if not make a new one...
		std::map< int, std::set<LightMap*> >::iterator light_it = lights.find(id);
		
		if (light_it != lights.end()) {
			light_it->second.insert(target);
		} else {
			std::set<LightMap *> new_set;
			new_set.insert(target);
			lights.insert(std::make_pair(id, new_set));
		}
	}
	
	void LightAtlas::setLightIntensity(int id, float intensity) {
		// iterate throught the map lights, and call the setLightIntensity on all registered lightmaps
		std::map< int, std::set<LightMap*> >::iterator light_it = lights.find(id);
		
		if (light_it != lights.end()) {
			atlas->lock(HardwareBuffer::HBL_DISCARD);
						
			std::set<LightMap *>::const_iterator lmaps_it = light_it->second.begin();
			
			for (;lmaps_it != light_it->second.end(); lmaps_it++) 
				(*lmaps_it)->setLightIntensity(id, intensity);
			
			atlas->unlock();
		}
	}
	
	int LightAtlas::getIndex() {
		return idx;
	}
	
	// ---------------- LightAtlasList Methods ----------------------
	LightAtlasList::LightAtlasList() {
		Opde::ConsoleBackend::getInstance()->registerCommandListener(std::string("light"), dynamic_cast<ConsoleCommandListener*>(this));
	}
	
	LightAtlasList::~LightAtlasList() {
		int last = list.size();
			
		// iterate through existing Atlases, and see if any of them accepts our lightmap
		for (int i = 0; i < last; i++) 
			delete list.at(i);
		
		list.clear();
	}
	
	
	LightMap* LightAtlasList::addLightmap(int ver, int texture, char *buf, int w, int h, AtlasInfo &destinfo) {
		if (list.size() == 0) 
			list.push_back(new LightAtlas(texture, 0));
		
		int last = list.size();
		
		LightMap *result;
		
		// iterate through existing Atlases, and see if any of them accepts our lightmap
		for (int i = 0; i < last; i++) {
			if (list.at(i)->getCategory() == texture) // so we don't mix up the different categories (e.g. texture numbers)
				if (result = list.at(i)->addLightmap(ver, buf, w, h, destinfo)) 
					return result;
		}
		
		// add new atlas to list if none of them accepted the lmap
		list.push_back(new LightAtlas(texture, last));
		
		return list.at(last)->addLightmap(ver, buf, w, h, destinfo);
		
		// now we have a lightAtlas which we can insert into. 
		// TODO: add a useful info in destinfo too please (for light switching)
	}
	
	int LightAtlasList::getCount() {
		return list.size();
	}

	bool LightAtlasList::render() {
		for (std::vector<LightAtlas *>::size_type i=0; i < list.size(); i++)
			if (!list.at(i)->render()) return false;
				
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
				
				Opde::ConsoleBackend::getInstance()->putMessage(std::string("Doing light change"));
				
				int light = StringConverter::parseInt(s_light);
				float intensity = StringConverter::parseReal(s_intensity);
				
				// apply
				setLightIntensity(light, intensity);
			}
		}
	}
	
// ------------------------------- Lightmap class
	void LightMap::refresh() {
		// First we calculate the new version of the lmap. Then we post it to the atlas
		// Float version of lmap
		Vector3* f_lmap = new Vector3[sx * sy];
					
		// copy the static lmap...
		for (unsigned int i = 0; i < sx * sy; i++) {
			f_lmap[i].x = static_lmap[i].R;
			f_lmap[i].y = static_lmap[i].G;
			f_lmap[i].z = static_lmap[i].B;
		}
		
		
		std::map<int, lmpixel *>::const_iterator lmap_it = switchable_lmap.begin();
		
		for  (;lmap_it != switchable_lmap.end(); lmap_it++) {
			lmpixel* act_lmap = lmap_it->second;
			
			int light_id = lmap_it->first;
			
			float intensity = 0;
			
			std::map<int, float>::iterator intens_it = intensities.find(light_id);
		
			// Just to be sure we get the intensity 
			if (intens_it == intensities.end())  {
				intensities.insert(std::make_pair(light_id, 0));
			} else {
				intensity = intens_it->second;
			}
			
			if (intensity > 0) {
				for (unsigned int i = 0; i < sx * sy; i++) {
					f_lmap[i] += intensity * act_lmap[i]; 
				}
			}
		}
		
		owner->updateLightMapBuffer(position, f_lmap);
		
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
		switchable_lmap.insert(std::make_pair(id, data));
		intensities.insert(std::make_pair(id, float(1)));
		
		// register us
		owner->registerAnimLight(id, this);
	}
	
	void LightMap::setLightIntensity(int id, float intensity) {
		std::map<int, float>::iterator intens = intensities.find(id);
		
		if (intens != intensities.end()) {
			// only if the value changed
			if (intens->second != intensity) {
				intens->second = intensity;
			
				refresh();
			}
		}
	}
	
	int LightMap::getAtlasIndex() {
		return owner->getIndex();
	}
} // namespace ogre

