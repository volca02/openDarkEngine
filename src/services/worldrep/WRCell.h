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

#ifndef __WRCELL_H
#define __WRCELL_H

#include "config.h"

#include "FileGroup.h"
#include "LightmapAtlas.h"
#include "WRTypes.h"

#include "material/MaterialService.h"

#include "DarkBspNode.h"

#include <OgreMaterial.h>
#include <OgreSceneNode.h>
#include <OgreStaticFaceGroup.h>

namespace Ogre {
class DarkGeometry;
class DarkSceneManager;
} // namespace Ogre

namespace Opde {
class WorldRepService;

/** Helping BSP vertex struct, used to prepare the vertex data. The
 * VertexDeclaration has to be set up accordingly */
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

/** Encapsulates the reading and interpreting of one Cell in the chunk. Has
 * methods for ogre Mesh generation. And data access */
class OPDELIB_EXPORT WRCell {
private:
    /** The cell number this cell represents */
    int mCellNum;

    WRCellHeader mHeader;

    /** the list of the cell's vertices (count is in the header) */
    Vector3 *mVertices;

    /** the list of the polygon map headers */
    WRPolygon *mFaceMaps;

    /** the list of the face texturing infos */
    WRPolygonTexturing *mFaceInfos;

    /** polygon mapping struct.. pointer to array of indices on each poly index
     * poly_indices[0][0]... etc.*/
    // uint32_t		num_indices;
    uint8_t **mPolyIndices;

    /** Planes forming the cell */
    Plane *mPlanes;

    /** Indicates the fact that the cell data have already been loaded */
    bool mLoaded;

    /** Indicates the fact that the cell portals have been already attached */
    bool mPortalsDone;

    Ogre::BspNode::PlanePortalMap mPortalMap;

    int countBits(uint32_t src);

    /** Inserts a new vertex into the manual object. Calculates all the UV
     * values needed
     * @deprecated For moving towards the geometry by buffers */
    void
    insertTexturedVertex(Ogre::ManualObject *manual, int faceNum,
                         const Vector3 &pos,
                         const std::pair<Ogre::uint, Ogre::uint> &dimensions,
                         Ogre::Vector3 origin);

    // Finds the unwrapping shifts (in multiple of 64.0f)
    void findLightmapShifts(Ogre::Vector2 &tgt, Ogre::Vector2 origin);

    /** The bsp node constructed by this class. Filled with static geometry and
     * otherwise initialized */
    Ogre::BspNode *mBSPNode;

    /** Owner service of this cell */
    WorldRepService *mOwner;

    /** Geometry holder to fill */
    Ogre::DarkGeometry *mLevelGeometry;

    /** Material Service - used for texture UV, atlas/material combinations */
    MaterialServicePtr mMaterialService;

    /** Light Service - used to get the poly's tag, lmap uv transforms, etc */
    LightServicePtr mLightService;

    /// Info about lmaps of the cell. Used for UV remaps, atlas mapping etc.
    LightsForCellPtr mLights;

public:
    /** Default constructor. */
    WRCell(WorldRepService *owner, Ogre::DarkGeometry *targetGeom);

    /** destructor */
    ~WRCell();

    /** Load the cell data from the given chunk.
     * @param _cell_num The cell ID we assign to the cell (for distinction)
     * @param chunk The database chunk to load data from
     * @param lightSize The lightmap pixel size (either 1 or 2)
     * @note lightSize has to be 1 for grayscale lightmaps, 2 for xBGR
     * lightmaps. Otherwise, an assertation takes place */
    void loadFromChunk(unsigned int _cell_num, FilePtr &chunk, int lightSize);

    /** Returns a cell's plane with the specified index */
    const Ogre::Plane &getPlane(int index);

    /** Construct and inserts the static geometry of the portal meshes into the
     * Scene Inserts a new SceneNode -> MovableObject pairs for each of the
     * portals geometry having a graphical representation (e.g. water planes)
     * @param sceneMgr SceneManager which is used for the construction
     * @note Note that Materials \@templateXXXX are expected to exist as those
     * are cloned in the construction process
     */
    void constructPortalMeshes(Ogre::SceneManager *sceneMgr);

    /// creates the geometry for the cell in the DarkGeometry given in
    /// constructor
    void createCellGeometry();

    /** Return the exact vertex count needed to set-up the vertex buffer with
    the cell data.
    * @note The vertex count is not a plain vertex list count, but the count of
    vertices which are counted as the resulting polygons total vertex sum (The
    reason is the vertex here is in fact a Vertex-Normal pair)
    */
    int getVertexCount();

    /** Return the exact count of indices needed to be filled into the static
     * geometry index buffer in order to set up the triangles
     * @note The index count should come up as 3 * triangle_Count
     */
    int getIndexCount();

    /** Return the count of faces this cell will produce.
     * @note I do one FaceGroup == One polygon here
     */
    int getFaceCount();

    /** Attaches all the found portals to the source and destination
     * DarkSceneNodes
     * @param smgr The scene manager to use for portal attachment
     * @return int Number of vertices removed by optimization
     */
    int attachPortals(Ogre::DarkSceneManager *smgr);

    /** Atlases the lightmaps defined in this cell.
     * @note do this before the constructStaticGeometry, otherwise the material
     * cloning will get confused */
    void atlasLightMaps(LightAtlasList *atlasList);

    /** Sets the BspNode we will fill (e.g. leaf node which corresponds to this
     * cell)
     */
    void setBspNode(Ogre::BspNode *tgtNode);

    /** Returns the associated BspNode filled by the other methods.
     * @note The owner attribute is not set on the returned node, this means
     * that YOU should do it.
     * @note This method returns the pointer to a beforehand constructed
     * instance, which is always leaf
     */
    Ogre::BspNode *getBspNode();

    /** Returns the center coordinate for the cell.
     */
    Ogre::Vector3 getCenter();
};
} // namespace Opde

#endif
