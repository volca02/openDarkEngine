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


#ifndef __wrcell_h
#define __wrcell_h

#include "WRTypes.h"
#include "FileGroup.h"
#include "LightmapAtlas.h"
#include "OgrePortal.h"
#include "OgreSceneNode.h"
#include "OgreDarkSceneNode.h"
#include "OgreBspNode.h"
#include <OgreMaterial.h>
#include <OgreStaticFaceGroup.h>

namespace Opde {
    class WorldRepService;

	const unsigned short int SKY_TEXTURE = 249;

	/** Helping BSP vertex struct, used to prepare the vertex data. The VertexDeclaration has to be set up accordingly */
	typedef struct BspVertex {
			/** Vertex position */
			float position[3];
			/** Vertex normal vector */
			float normal[3];
			/** Colour of the vertex - ?*/
			// int colour; // TODO: Can I safely remove this one?
			/** Texturing UV */
			float texcoords[2];
			/** Lightmapping UV */
			float lightmap[2];
	} BspVertex;


	/** Encapsulates the reading and interpreting of one Cell in the chunk. Has methods for ogre Mesh generation. And data access */
	class WRCell {
		private:
			/** The cell number this cell represents */
			int cellNum;

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

			/** Planes forming the cell */
			Ogre::Plane *planes;

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

			/** Indicates the fact that the lightmaps have already been atlased */
			bool atlased;

			/** Indicates the fact that the cell data have already been loaded */
			bool loaded;

			/** Indicates the fact that the cell portals have been already attached */
			bool portalsDone;

			Ogre::BspNode::PlanePortalMap mPortalMap;

			int countBits(uint32_t src);

			/** Returns a prepared material pointer for combination texture/atlasnum
			* @note fills the dimensions parameter with the texture dimensions */
			const Ogre::MaterialPtr getMaterial(unsigned int texture, unsigned int atlasnum, std::pair< Ogre::uint, Ogre::uint > &dimensions, unsigned int flags);

			/** Returns a prepared material name for combination texture/atlasnum
			* @note fills the dimensions parameter with the texture dimensions */
			const Ogre::String getMaterialName(unsigned int texture, unsigned int atlasnum, std::pair< Ogre::uint, Ogre::uint > &dimensions, unsigned int flags);

			/** Inserts a new vertex into the manual object. Calculates all the UV values needed
			* @deprecated For moving towards the geometry by buffers */
			void insertTexturedVertex(Ogre::ManualObject *manual, int faceNum, wr_coord_t pos, Ogre::Vector2 displacement, 
				std::pair< Ogre::uint, Ogre::uint > dimensions, Ogre::Vector3 origin);

			/** Constructs a BSPVertex out of our data */
			void constructBspVertex(int faceNum, wr_coord_t pos, Ogre::Vector2 displacement, std::pair< Ogre::uint, Ogre::uint > dimensions, BspVertex *vtx);

			/** Calculates the Lightmap center in texture space, using Bounding coordinates as the base. */
			Ogre::Vector2 calcLightmapDisplacement(int polyNum);


			/** The bsp node constructed by this class. Filled with static geometry and otherwise initialized */
			Ogre::BspNode* bspNode;

			/** Owner service of this cell */
			WorldRepService* mOwner;

		public:
			/** Default constructor. */
			WRCell(WorldRepService* owner);
			~WRCell();

			/** Load the cell data from the given chunk.
			* @param _cell_num The cell ID we assign to the cell (for distinction)
			* @param chunk The database chunk to load data from
			* @param lightSize The lightmap pixel size (either 1 or 2)
			* @note lightSize has to be 1 for grayscale lightmaps, 2 for xBGR lightmaps. Otherwise, an assertation takes place */
			void loadFromChunk(unsigned int _cell_num, FilePtr& chunk, int lightSize);

			/** Returns a cell's plane with the specified index */
			const Ogre::Plane& getPlane(int index);

			/** Construct and inserts the static geometry of the portal meshes into the Scene
			* Inserts a new SceneNode -> MovableObject pairs for each of the portals geometry having a graphical representation (e.g. water planes)
			* @param sceneMgr SceneManager which is used for the construction
			* @note Note that Materials \@templateXXXX are expected to exist as those are cloned in the construction process
			*/
			void constructPortalMeshes(Ogre::SceneManager *sceneMgr);


			/// Creates a scene node with all non-portal geometry attached as a mesh
			Ogre::SceneNode* createSceneNode(Ogre::SceneManager *sceneMgr);

			/** Return the exact vertex count needed to set-up the vertex buffer with the cell data.
			* @note The vertex count is not a plain vertex list count, but the count of vertices which are counted as the resulting polygons
				total vertex sum (The reason is the vertex here is in fact a Vertex-Normal pair)
			*/
			int getVertexCount();

			/** Return the exact count of indices needed to be filled into the static geometry index buffer in order to set up the triangles
			* @note The index count should come up as 3 * triangle_Count
			*/
			int getIndexCount();

			/** Return the count of faces this cell will produce.
			* @note I do one FaceGroup == One polygon here
			*/
			int getFaceCount();


			/** Update the Vertex/Index buffers with polygonal data this cell contains. Also insert face information into a set of StaticFaceGroup
			* descriptors and update the accompanying BspNode to contain start/count of those
			* @param vertexPtr A pointer to a place which will accept the vertex data
			* @param indexPtr A pointer to a place which will accept the index data
			* @param facePtr A pointer to a place which will accept our face groups
			* @param startVertex an absolute index of the first vertex this cell will produce (For index absolutization)
			* @param startFace an absolute index of the first face this cell will produce
			* @param startIndex an absolute index of the first tri-index this cell will produce
			* @param node A BspNode class instance that will get happy if it receives the StaticFaceGroup index and count
			* @return count of face groups produced
			* @note Please note that the vertex buffers usualy need to be locked prior to writing to them
			* @note the written index,vertex and face counts must not be greater than the getXXXXCount calls returned
			* @note The Light maps have to be atlased before
			*/
			int buildStaticGeometry(BspVertex* vertexPtr, unsigned int* indexPtr, Ogre::StaticFaceGroup* facePtr,
						int startVertex, int startIndex, int startFace);

			/** Attaches all the found portals to the source and destination DarkSceneNodes
			* @param cellList The cell list which the method uses to set source and destination BspNodes
			* @return int Number of vertices removed by optimization
			*/
			int attachPortals(WRCell** cellList);

			/** Atlases the lightmaps defined in this cell.
			* @note do this before the constructStaticGeometry, otherwise the material cloning will get confused */
			void atlasLightMaps(LightAtlasList* atlasList);

			/** Sets the BspNode we will fill (e.g. leaf node which corresponds to this cell)
			*/
			void setBspNode(Ogre::BspNode* tgtNode);

			/** Returns the associated BspNode filled by the other methods.
			* @note The owner attribute is not set on the returned node, this means that YOU should do it.
			* @note This method returns the pointer to a beforehand constructed instance, which is always leaf
			*/
			Ogre::BspNode* getBspNode();

			/** Returns the center coordinate for the cell.
			*/
			Ogre::Vector3 getCenter();
	};
}

#endif
