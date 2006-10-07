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

#ifndef __wrcell_h
#define __wrcell_h

#include "WRTypes.h"
#include "DarkDatabase.h"
#include "LightmapAtlas.h"
#include "OgrePortal.h"
#include "OgreSceneNode.h"
#include "OgreDarkSceneNode.h"

namespace Opde {
	
	const unsigned short int SKY_TEXTURE = 249;
	
	/** Encapsulates the reading and interpreting of one Cell in the chunk. Has methods for ogre Mesh generation. And data access */
	class WRCell {
		private:
			wr_cell_hdr_t	header;
		
			/** the list of the cell's vertices (count is in the header) */
			wr_coord_t	*vertices;
			
			/** the list of the polygon map headers */
			wr_polygon_t	*face_maps;
			
			/** the list of the face texturing infos */
			wr_polygon_texturing_t	*face_infos;
			
			/** polygon mapping struct.. pointer to array of indices on each poly index  poly_indices[0][0]... etc.*/
			// uint32_t		num_indices;
			uint8_t		**poly_indices;
			
			/** Plane descriptors */
			wr_plane_t	*planes;
			
			/** animated lights map (e.g. bit to object number mapping) - count is to be found in the header */
			int16_t		*anim_map; // index by bit num, and ya get the object number the animated lightmap belongs to
			
			wr_light_info_t *lm_infos;
			
			// Lightmaps:
			// The count of lightmaps per face index
			uint8_t* lmcounts;
			uint8_t* **lmaps; // poly number, lmap number --> pointer to data (may be 2 bytes per pixel)
			
			// objects that are in this leaf when loaded (we may skip this if we add it some other way in system) (Maybe it's only a light list affecting our cell)
			uint32_t		obj_count;
			uint16_t		*obj_indices; // the object index number
			
			/* Array referencing the lightmaps inserted into the atlas */ 
			LightMap** 	lightMaps;
			
			/** Size of the lightmap element */
			int mLightSize;
			
			int countBits(uint32_t src);
			
			/** Returns a prepared material pointer for combination texture/atlasnum
			* @note fills the dimensions parameter with the texture dimensions */
			Ogre::String getMaterialName(unsigned int texture, unsigned int atlasnum, std::pair< Ogre::uint, Ogre::uint > &dimensions);
			
			/** Inserts a new vertex into the manual object. Calculates all the UV values needed */
			void insertTexturedVertex(Ogre::ManualObject *manual, int faceNum, wr_coord_t pos, Ogre::Vector2 displacement, std::pair< Ogre::uint, Ogre::uint > dimensions);
			
			/** Calculates the Lightmap center in texture space, using Bounding coordinates as the base. */
			Ogre::Vector2 calcLightmapDisplacement(int polyNum);
		public:
			WRCell();
			~WRCell();
		
			void loadFromChunk(DarkDatabaseChunk *chunk, int lightSize);
		
			/** Returns a cell's plane with the specified index */
			const wr_plane_t getPlane(int index);
		
			/** Construct and inserts the static geometry of this cell into the SceneNode. 
			* Inserts a new MovableObject into the specified SceneNode, which contains the cell's static geometry.
			* @param node SceneNode onto which the geometry is attached 
			* @param sceneMgr SceneManager which is used for the construction
			* @param atlasList LightAtlasList into which the lightmaps are inserted
			* @note Note that Materials @templateXXXX are expected to exist as those are cloned in the construction process */
			void constructStaticGeometry(int cellNum, Ogre::SceneNode *node, Ogre::SceneManager *sceneMgr, LightAtlasList* atlasList);
		
			/** Attaches all the found portals to the source and destination DarkSceneNodes
			* @param nodeList The cell list which the method uses to set source and destination SceneNodes 
			* @param thisNode The SceneNode representing this cell */
			void attachPortals(std::vector< Ogre::DarkSceneNode *> nodeList, Ogre::DarkSceneNode *thisNode);
		
			/** Atlases the lightmaps defined in this cell.
			* @note do this before the constructStaticGeometry, otherwise the material cloning will get confused */
			void atlasLightMaps(LightAtlasList* atlasList);
	};
}

#endif
