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
 *****************************************************************************/

#ifndef LATLAS_H
#define LATLAS_H

#include <vector>
#include <map>
#include "OgreImage.h"
#include "OgreTextureManager.h"
#include "OgreVector2.h"
#include "OgreVector3.h"
#include "OgreHardwarePixelBuffer.h"
#include "ConsoleCommandListener.h"

// maximal W and H of lightmap in pixels
#define ATLAS_maxlm 32
// number of lightmaps in a row (and number of rows)
#define ATLAS_numlm 8
// number of lightmaps in atlas
#define ATLAS_lmcount (ATLAS_numlm*ATLAS_numlm)
// size of lightmap atlas (both W and H)
#define ATLAS_lmsize (ATLAS_maxlm*ATLAS_numlm)

namespace Opde {
	
typedef struct {
	int atlasnum;
	float u,v; // shift of the lmap in the atlas
	float su,sv; // size of the lmap in the atlas
} AtlasInfo;


/** Free space information storage - rectangular area in the lightmap (either used or free to use) */
class FreeSpaceInfo {
	public:
		int	x;
		int	y;
		int	w;
		int	h;

		FreeSpaceInfo() {
			this->x = 0;
			this->y = 0;
			this->w = -1;
			this->h = -1;
		}
	
		FreeSpaceInfo(int x, int y, int w, int h) {
			this->x = x;
			this->y = y;
			this->w = w;
			this->h = h;
		}
		
		bool Fits(int sw, int sh) {
			if ((sw<=w) && (sh<=h))
				return true;
			
			return false;
		}
		
		// test if we can, and if so, Crop Out the sw*sh rect, and insert the rest of the rects (the one that were created during croping) to the list
		// otherwise return false (if size is too big for us)
		
		/** Crops this free space with a rectangular area measured sw*sh. 
		* Used for lightmap space allocation inside the Light Map atlas.
		* If the rectangular area requested fits into the area the instance represents, the instance is removed from the list. 
		* If there is some space left on the bottom or right of the cropping area, a new FreeSpaceInfo is inserted to describe those.
		* \param sw Width of the cropping area
		* \param sh Height of the cropping area
		* \param list a std::vector storing this instance
		* \param order the index of this instance inside the list (to let this instance remove itself from the list)
		* \param dest a pointer to the target FreeSpaceInfo instance we fill with the cropped area if sucesful
		* \return true if sucessful, false otherwise 
		* @see Opde::LightAtlas */
		bool Crop(int sw, int sh, std::vector< FreeSpaceInfo * > &list, int order, FreeSpaceInfo *dest) {
			// first test if we can insert
			if (!Fits(sw,sh))
				return false;
			
			// now insert
			// we can have up to 2 rectangles as a result of cropping operation
			
			list.erase(list.begin()+order);
			
			// bottom will be created?
			if (sh<h) 
				list.push_back(new FreeSpaceInfo(x,y+sh,w,h-sh));
						
			// right will be created?
			if (sw<w) 
				list.push_back(new FreeSpaceInfo(x+sw,y,w-sw,sh));
						
			// ok, we have the original cropped.
			// the removal of us must be done from caller
			
			// create new FreeSpaceInfo describing the target.
			*dest = FreeSpaceInfo(x,y,sw,sh);
			
			return true;
		}
};

/** A structure representing a lightmap pixel */
typedef struct lmpixel {
	unsigned char R,G,B;
	
	/** Converts the RGB values to the A8R8G8B8 format 
	* \return uint32 holding the A8R8G8B8 version of the stored RGB */
	Ogre::uint32 ARGB() {
		return (R << 16) | (G << 8) | B;
	}
	
	lmpixel(unsigned char nR,unsigned char nG,unsigned char nB) {
		R = nR; G = nG; B = nB;
	}
	
	lmpixel() {
		R = 0; G = 0; B = 0;
	}
} lmpixel;


// Forward declaration
class LightAtlas;
	
/** A class representing a switchable lightmap. It holds one static lightmap, which can't be switched, and a set of lightmaps indexed by light number, 
* which can have their'e intensity modulated. The resulting lightmap is recalculated every time an actual chage happens. 
* Please use Ogre::LightAtlasList.setLightIntensity if you want to set an intensity to a certain light. Calling the method here would not refresh the lightmap texture. 
* @see LightAtlasList */
class LightMap {
		// So the atlasser is able to set uv && suv for us
		friend class LightAtlas;
			
		/** Information about the lightmap position in the atlas */
		FreeSpaceInfo position;
	
		/** static lightmap */
		lmpixel* static_lmap;
	
		/** A map of the switchable lightmaps */
		std::map< int, lmpixel* > switchable_lmap;
		
		/** A map of the actual intensities */
		std::map< int, float > intensities;
	
		/** Lightmap's size in pixels */
		unsigned int sx, sy;
	
		/** Atlas holding the lightmap */
		LightAtlas *owner;
	
		/** Repositioning from 0-1 to the Atlas coords */
		Ogre::Vector2 uv; 
	
		/** Size of the lmap in the atlas*/
		Ogre::Vector2 suv;
		
	public:
		/** Constructor - takes the targetting freespaceinfo, size of the lightmap and initializes our buffer with the static lightmap.
		* This class will manage the unallocation of the lighymap as needed, just pass the pointer, it will deallocate the lmap in the destructor */
		LightMap(FreeSpaceInfo& tgt, LightAtlas *owner, unsigned int sx, unsigned int sy, lmpixel *static_lightmap) {
			position = tgt;
			
			static_lmap = static_lightmap;
			
			this->sx = sx;
			this->sy = sy;
			this->owner = owner;
		}
		
		~LightMap() {
			if (static_lmap != NULL)
				delete[] static_lmap;
			
			// TODO: delete the animated lmaps too
		}
	
		/** Gets an owner atlas of this lightmap
		* \return LightAtlas containing this light map */
		const LightAtlas* getOwner() {
			return owner;
		}
		
		/** Converts input UV coords to the Atlas coords */
		Ogre::Vector2 toAtlasCoords(Ogre::Vector2 in_uv) {
			in_uv = in_uv * suv; // dunno why, this one does not work written in_uv *= suv;
			in_uv += uv;
			
			return in_uv;
		}
		
		/** Helping static method. Prepares an RGB version of the given buffer containing v1 or v2 lightmap */
		static lmpixel* convert(char *data, int sx, int sy, int ver);
		
		/** Adds a switchable lightmap with identification id to the lightmap list (has to be of the same size). 
		* \param id the ID of the light the lightmaps belongs to.
		* \param data are the actual values converted to RGB. Unallocation is handled in destructor */
		void AddSwitchableLightmap(int id, lmpixel *data);
		
		/** The main intensity setting function */
		void setLightIntensity(int id, float intensity);
		
		/** Refreshes the texture's pixel buffer with our new settings*/
		void refresh();
		
		/** Returns the atlas index */
		int getAtlasIndex();
};

/** Class holding one set of light maps. Uses a HardwarePixelBufferSharedPtr class for texture storage.
* This class is used for atlasing a light map set into one bigger lightmap texture containing the light maps.
* This accelerates rendering. */
class LightAtlas {
	private:
		/** The count of the stored light maps */
		int count;
	
		/** The category of this atlas - used for texture number - every texture owns it's own atlas or atlases */
		int category;
	
		/** Global index of this atlas in the atlas list */
		int idx;
	
		/** Tecture pointer for this atlas */
		Ogre::TexturePtr ptex;
	
		/** The resulting pixel buffer */
		Ogre::HardwarePixelBufferSharedPtr atlas; 
	
		/** The name of the resulting resource */
		Ogre::StringUtil::StrStreamType name;
	
		/** A vector containing Free Space rectangles left in the Atlas */
		std::vector < FreeSpaceInfo * > freeSpace;
	
		/** Already inserted lightmaps - mainly for dealocation */
		std::vector < LightMap * > lightmaps;
	
		/** Light number to the LightMap set mapping - changing light intensity will iterate throught the set of lmaps, and regenerate */
		std::map<int, std::set<LightMap*> > lights;
		
		/** A buffer which holds a copy of the image data */
		lmpixel *copybuffer; // I keep the data of the lightmap here before I place them to Texture buffer.

		/** return an origin - XY coords for the next free space (for a lightmap sized w*h) */
		FreeSpaceInfo getOrigin(int w, int h);

	public:
		LightAtlas(int category, int idx);
	
		/** \return true if a light map of this size can be inserted into this light map atlas. */
		bool canInsert(int w, int h);

		/** Adds a light map to this atlas. 
		* \param ver the version (and thus a size) of the WR chunk. 
		* \param buf a buffer containing the light map pixel data
		* \param w the width of the light map pixel data
		* \param h the height of the light map pixel data
		* \param destinfo destination structure to hold light map targetting and stuff.
		*/
		//  TODO: Deprecate destinfo
		LightMap* addLightmap(int ver, char *buf, int w, int h, AtlasInfo &destinfo); // TODO: make a size out of the ver

		/** A function returning the category of this Atlas. Used for texture number distinction. 
		* \return int Category of the Atlas (Currently used for texture number) */
		int getCategory() {
			return category;
		}
		
		/** renders the prepared light map buffers into a texture - copies the pixel data to the 'atlas' */
		bool render();
		
		/** Updated the pixel buffer with a new version of the lightmap. This version converts the Vector3 floats to 0-255 range (checks limits). 
		* \warning Must be called after atlas locking, otherwise the program will crash ! */
		void updateLightMapBuffer(FreeSpaceInfo& fsi, lmpixel* rgb);
		
		/** Updated the pixel buffer with a new version of the lightmap. This version converts the Vector3 floats to 0-255 range (checks limits) 
		* \warning Must be called after atlas locking, otherwise the program will crash ! */
		void updateLightMapBuffer(FreeSpaceInfo& fsi, Ogre::Vector3* rgb);
		
		/** Register that animated light ID maps to the LightMap instance */
		void registerAnimLight(int id, LightMap* target);
		
		void setLightIntensity(int id, float intensity);
		
		/** Returns the Light Map Atlas order number */
		int getIndex();
};


/** @brief A holder of a number of the light map atlases. 
*
* Use this class to work with the light map storage and light switching. */
class LightAtlasList : public ConsoleCommandListener {
	private:
		/** A list of the light map atlases */
		std::vector<LightAtlas *> list;
	
		/** ? */
		int index;

	public:
		LightAtlasList();
	
		/** A destructor, unallocates all the previously allocated lightmaps */
		virtual ~LightAtlasList();
		
		/** Adds a light map holding place and a static lightmap */
		LightMap* addLightmap(int ver, int texture, char *buf, int w, int h, AtlasInfo &destinfo);

		/** get's the current count of the atlases stored. 
		* \return int Count of the atlases */
		int getCount();

		/** Prepares the Light Atlases to be used as textures. Calls the LightAtlas.render method
		* @see Ogre::LightAtlas.render() */
		bool render();
	
		void setLightIntensity(int id, float value) {
			std::vector< LightAtlas* >::iterator atlases = list.begin();
			
			for (;atlases != list.end(); atlases++) {
				(*atlases)->setLightIntensity(id, value);
			}
		};
		
		virtual void commandExecuted(std::string command, std::string parameters);
};

} // namespace Ogre
#endif
