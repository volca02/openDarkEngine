/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 *Software Foundation; either version 2 of the License, or (at your option) any
 *later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 *details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 *along with this program; if not, write to the Free Software Foundation, Inc.,
 *59 Temple Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 * http://www.gnu.org/copyleft/lesser.txt.
 *
 *
 *	$Id$
 *
 *****************************************************************************/

#ifndef __DARKGEOMETRY_H
#define __DARKGEOMETRY_H

#include "config.h"
#include <OgreHardwareBuffer.h>
#include <OgreRenderOperation.h>

#include "DarkBspPrerequisites.h"
#include "OgreRenderable.h"

namespace Ogre {
// Foward decls.
class DarkSubGeometry;
class DarkFragment;

/// Class that tries to optimally store level geometry of Dark for rendering
class OPDELIB_EXPORT DarkGeometry {
public:
    /// Constructor
    DarkGeometry(const String &name, 
                 size_t cellCount,
                 uint8 defaultRenderQueueID);

    /// Destructor
    ~DarkGeometry();

    /// Notifies the DarkGeometry to update it's contents based on visible areas
    void updateFromCamera(const DarkCamera *cam);

    /// Queues all the viable DarkSubGeometries for rendering
    void queueForRendering(RenderQueue *queue);

    /// Indicates that all the geometry was put in, and a build should be done
    void build(void);

    /// Creates a new cell's fragment (single-material cell geometry part)
    DarkFragment *createFragment(size_t cellID, const MaterialPtr &mat);

    /// retrieves an already existing cell fragment
    DarkFragment *getFragment(size_t cellID, const MaterialPtr &mat);

protected:
    /** Retrieves a pointer to sub-geometry by it's material
     * @return Either existing or newly created sub-geometry, or NULL if
     * createIfNotFound is false and the Sub-Geometry was not found
     */
    DarkSubGeometry *getSubGeometryForMaterial(const MaterialPtr &mat,
                                               bool createIfNotFound = false);

    /// Name of this DarkGeometry
    String mName;

    /// default render queue id used for the sub-geometries
    uint8 mDefaultRenderQueueID;

    /// Indicates that the geometry was built
    bool mBuilt;

    /// Maps a material name to subgeometry
    typedef std::map<String, DarkSubGeometry *> DarkSubGeometryMap;

    /// List of sub-geometries
    typedef std::list<DarkSubGeometry *> DarkSubGeometryList;

    /// Map of all sub geometries, per material. Master list
    DarkSubGeometryMap mSubGeometryMap;

    /// List of subgeometries to be queued (visible parts)
    DarkSubGeometryList mVisibleSubGeometries;

    /// Total count of the cells the static geometry contains
    size_t mCellCount;

    /// central list of dynamic lights for the rendering
    LightList mLightList;
};

/// Vertex buffer allocation info. Linked-list impl.
struct DarkBufferAllocation {
    DarkBufferAllocation *next;
    DarkBufferAllocation *last;

    uint32 pos;
    uint32 size;
    bool free;
};

// Forward decl.
class DarkSubGeometry;

typedef struct {
    float pos[3];
    float norm[3];
    float txtcoord[2];
    float lmcoord[2];
} DarkVertex;

/// Geometry builder for a fragment. Is used temporarily to build geometry from
/// vertex definitions
class OPDELIB_EXPORT DarkFragmentBuilder {
public:
    /// defines a vertex. Returns index to the vertex list holding the
    /// definition of this vertex
    size_t vertex(const Vector3 &pos, const Vector3 &norm, const Vector2 &txt0,
                  const Vector2 &txt1);

    /// pushes in a new index
    void index(size_t index);

    /// Vertex definition for a single unique vertex
    typedef struct {
        size_t pos_idx;
        size_t norm_idx;
        size_t txt0_idx;
        size_t txt1_idx;
    } VertexDefinition;

    /** builds the queued vertex and index data into the fragment/sub-geometry
     * pair
     * @param vdest A pointer to memory to fill with the vertices (VBO)
     * @param ilist The target ilist to hold indices
     * @note both parameters have to be big enough to hold the vertex index data
     * in the counts given by the getVertexCount/getIndexCount routines
     */
    void build(DarkVertex *vdest, uint32 *ilist);

    /// Count of the held vertices
    uint32 getVertexCount(void);

    /// Count of the held indices
    uint32 getIndexCount(void);

protected:
    typedef std::vector<VertexDefinition> VertexDefinitionQueue;
    typedef std::vector<uint32> IndexQueue;

    typedef std::vector<Vector3> CoordList;
    typedef std::vector<Vector2> TxtCoordList;

    /** Unmapper for Vector3 coordinates
     * Will return index the coord is to be found at
     * @param list the list to unmap on
     * @param coord the texture coord to unmap
     */
    static size_t indexCoord(CoordList &list, const Vector3 &coord);

    /** Unmapper for texture coordinates
     * Will return index the texture coord is to be found at
     * @param list the list to unmap on
     * @param coord the texture coord to unmap
     */
    static size_t indexTxtCoord(TxtCoordList &list, const Vector2 &coord);

    /// Vector of indexed vertices
    VertexDefinitionQueue mVertexQueue;

    /// Vector of indices for the target triangles
    IndexQueue mIndexQueue;

    /// List of vertices
    CoordList mVertices;

    /// List of normals
    CoordList mNormals;

    /// First set of texture coordinates
    TxtCoordList mTxtCoords0;

    /// Second set of texture coordinates
    TxtCoordList mTxtCoords1;
};

/// A fragment of a world's geometry. Contains info about one distinct material
/// of one cell
class OPDELIB_EXPORT DarkFragment {
    friend class DarkSubGeometry;

public:
    /// Constructor. Given are Vertex array and Normal Array pointers
    DarkFragment(DarkSubGeometry *owner);

    /// Destructor. Deallocated the allocation
    ~DarkFragment(void);

    /// defines a vertex in the fragment
    size_t vertex(const Vector3 &pos, const Vector3 &normal,
                  const Vector2 &txt1, const Vector2 &txt2);

    /// pushes in a new index (should be called 3x per triangle, although FAN
    /// might be an interesting option as well)
    void index(size_t idx);

    /// writes the relocated indices into the buffer, returns the count of
    /// indices writtens
    size_t cacheIndices(void *bufPtr, bool use16Bit);

protected:
    /// builds itself into a buffer. After this call, the vertex/normal
    /// definitions are not needed anymore
    void build(void);

    /// Moves itself into a new buffer (no vertex recalculations are done)
    void move(DarkSubGeometry *newOwner);

    /// gets the vertex count from the builder (or self if built)
    uint32 getVertexCount(void);

    /// gets the index count from the builder (or self if built)
    uint32 getIndexCount(void);

    /// Current owner of this fragment
    DarkSubGeometry *mOwner;

    /// Allocation info - which part of VBO this fragment uses
    DarkBufferAllocation *mCurrent;

    /// Builder for the buffers
    DarkFragmentBuilder *mBuilder;

    /// Flag indicating that the geometry was already built
    bool mBuilt;

    /// Count of indices of this fragment
    uint32 mIdxCount;

    /// Count of vertices of this fragment
    uint32 mVtxCount;

    /// The indices themself
    uint32 *mIndexList;
};

/** A geometry container for a single material. The geometry of the level is
 * split into these per material it is rendered with */
class __attribute__ ((visibility("hidden"))) DarkSubGeometry : public Renderable {
    friend class DarkGeometry;
    friend class DarkFragment;

public:
    /** Dark SubGeometry constructor
     * @param material The material reference that is used to render this
     * sub-geometry
     * @param cellCount The specified cell count to be used (cell == BSP leaf)
     * @param renderQueueID The ID of the render queue this sub-geometry should
     * be rendered in
     * @param centralLightList a reference to the central light list of the
     * DarkGeometry (so that it is only build once per update for all subgeoms)
     */
    DarkSubGeometry(const MaterialPtr &material, size_t cellCount,
                    uint8 renderQueueID, LightList &centralLightList);
    ~DarkSubGeometry();

    virtual const MaterialPtr &getMaterial(void) const;

    /// Sets the renderQueueGroup id this subgeometry should be rendered in
    inline void setRenderQueueID(uint8 queueID) { mRenderQueueID = queueID; };

    /// getter for the reder queue group id
    inline uint8 getRenderQueueID() const { return mRenderQueueID; };

    bool getUses16BitIndices(void) const { return m16BitIndices; };

protected:
    /** Updates self to be ready for rendering by the given camera
     * @return size_t Count of indices prepared - sort-of size of the index
     * buffer */
    size_t updateFromCamera(const DarkCamera *cam);

    /// Builds the geometry, prepares it to be rendered
    void build(void);

    /// requests a new fragment to hold a cells geometry
    DarkFragment *createFragment(size_t cellID);

    /// requests a new fragment to hold a cells geometry
    DarkFragment *getFragment(size_t cellID);

    /// Allocates a space in the VBO
    DarkBufferAllocation *allocateVBOSpace(size_t size);

    /// Frees a space in the vbo previously occupied
    void freeVBOSpace(DarkBufferAllocation *alloc);

    /// locks the VBO to be able to write into specified location
    DarkVertex *lock(DarkBufferAllocation *alloc,
                     HardwareBuffer::LockOptions lockOptions);

    /// Unlocks the VBO again
    void unlock(void);

    // --- Mandatory impl. for Renderables ---
    /// Produces a renderOperation of this DarkSubGeometry
    virtual void getRenderOperation(RenderOperation &op);

    /// Returns an identity transform
    virtual void getWorldTransforms(Matrix4 *xform) const;

    /// Returns an identity quaternion
    virtual const Quaternion &getWorldOrientation(void) const;

    /// Returns a Zero Vector3
    virtual const Vector3 &getWorldPosition(void) const;

    /// Returns zero
    virtual Real getSquaredViewDepth(const Camera *cam) const;

    /** Returns an empty light list
    @todo per-vertex lightning for first 8(?) dynamic lights)
    */
    virtual const LightList &getLights(void) const;

private:
    /// indicates that 16 bit indices are in use
    bool m16BitIndices;

    /// Material used by this instance
    const MaterialPtr mMaterial;

    /// render queue this geometry uses
    uint8 mRenderQueueID;

    /// Our empty light list (can be used later on to let dynamic lights work)
    LightList mLightList;

    /// Our render operation
    RenderOperation mRenderOp;

    /// Our vertex buffer
    HardwareVertexBufferSharedPtr mVertexBuffer;

    /// Indicates this geometry was already built and is ready for rendering
    bool mBuilt;

    /// Count of indices for this sub-geom.
    uint32 mIdxCount;

    /// Count of vertices for this sub-geom.
    uint32 mVtxCount;

    /// Allocation info storage
    DarkBufferAllocation *mAllocationList;

    /// Endpoint pointer for the allocation
    DarkBufferAllocation *mAllocationEnd;

    /// Map of fragments this SubGeometry holds
    DarkFragment **mFragmentList;

    /// Total count of the cells the static geometry contains
    size_t mCellCount;
};

}; // namespace Ogre

#endif // __DARKGEOMETRY_H
