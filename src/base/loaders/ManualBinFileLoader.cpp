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
 *
 *	  $Id$
 *
 *****************************************************************************/

#include "config.h"

#include "BinFormat.h"
#include "File.h"
#include "FileCompat.h"
#include "ManualBinFileLoader.h"
#include "LGPalette.h"
#include "format.h"
#include "logger.h"

// #include "MaterialService.h"

#include <OgreBone.h>
#include <OgreHardwareBufferManager.h>
#include <OgreLogManager.h>
#include <OgreMaterial.h>
#include <OgreMaterialManager.h>
#include <OgreSkeleton.h>
#include <OgreSkeletonManager.h>
#include <OgreStringConverter.h>
#include <OgreSubMesh.h>
#include <OgreTechnique.h>
#include <OgreTextureManager.h>

#include <OgreColourValue.h>
#include <OgreMeshSerializer.h>
#include <OgreSkeletonSerializer.h>

using namespace std;

namespace Opde {

namespace {

static Vertex unpack_normal(uint32_t src) {
    Vertex res;
    /* The Vector is organized as follows

    bit 32 -> bit 0

    XXXX XXXX | XXYY YYYY || YYYY ZZZZ | ZZZZ ZZ00

    each of those are fixed point signed numbers (10 bit)
    */

    res.z = (int16_t)((int16_t)(src & 0x0FFC) << 4) / 16384.0f;
    res.y = (int16_t)((src >> 6) & 0x0FFC0) / 16384.0f;
    res.x = (int16_t)((src >> 16) & 0x0FFC0) / 16384.0f;

    return res;
}

} // namespace

/** Class that loads a .CAL file and produces a ogre's skeleton instance
 * There are certain differences between .CAL and ogre's skeleton concept.
 * For example the whole skeleton in the CAL file is consisted of Torsos and
 * bones. A torso concept does not exist in ogre, but can be simulated by a set
 * of bones from the torso's root to the attachment points. Then there is the
 * fact that dark adresses the bones as joints, and uses a number (not a string)
 * as a unique identifier
 *
 * As ogre expects all the vertices in the transformed positions (something Dark
 * does not do neither for LGMM nor LGMD), this class also is a transformation
 * source for the vertices based on the blended positions of joints.
 */
class CalSkeletonLoader {
public:
    CalSkeletonLoader(const std::string &name, const std::string &group,
                      Ogre::Resource *resource)
        : mFileName(name), mGroup(group), mResource(resource), mFile(NULL),
          mTorsos(), mLimbs(), mSkeleton(){};

    ~CalSkeletonLoader() {}

    /// does all the loading work, creates an Ogre::Skeleton
    void load();

    /// Retuns the resulting skeleton
    Ogre::SkeletonPtr &getSkeleton() { return mSkeleton; };

protected:
    /// Reads the header from the .CAL file
    void readHeader(void);

    /// Reads the torsos from the .CAL file
    void readTorsos(void);

    /// Reads the limbs from the .CAL file
    void readLimbs(void);

    /// Converts Vertex to Ogre::Vector3
    static Vector3 toVector(const Vertex &v) { return Vector3(v.x, v.y, v.z); };

    /// the cal file name (incl. the .cal extension)
    std::string mFileName;

    /// the resource group used for the generated skeleton
    std::string mGroup;

    /// Resource we're loading the skeleton for
    Ogre::Resource *mResource;

    /// the file used for reading
    FilePtr mFile;

    /// header of the cal file
    CalHdr mHeader;

    /// the torso array prepared for processing
    std::vector<CalTorso> mTorsos;

    /// the limb array prepared for processing
    std::vector<CalLimb> mLimbs;

    /// our target skeleton instance
    Ogre::SkeletonPtr mSkeleton;
};

void CalSkeletonLoader::load() {
    // quite simple, really
    Ogre::DataStreamPtr stream =
        Ogre::ResourceGroupManager::getSingleton().openResource(
            mFileName, mGroup, true, mResource);

    mFile = FilePtr(new OgreFile(stream));

    // ok, got it open
    // read the header
    readHeader();

    if (mHeader.version != 1)
        OPDE_EXCEPT(format("Cal file has version other than 1 : ", mFileName));

    // read the torsos
    readTorsos();

    // limbs now
    readLimbs();

    // ----------- Now, we build the result

    // Create a new skeleton to fill
    mSkeleton = Ogre::SkeletonManager::getSingleton().create(
        mFileName + "-skeleton", mGroup);

    // construct the Bones
    // Torsos first
    // the root torso we expect as first
    if (mTorsos[0].parent != -1)
        OPDE_EXCEPT(
            format("Cal file expected to have torsos in order of creation : ",
                   mFileName));

    // the really ROOT bone
    // TODO: Map file for named joints (bones), as an optional addition
    // Bone* root =
    mSkeleton->createBone(mTorsos[0].root);

    for (int i = 0; i < mHeader.num_torsos; ++i) {
        // get the torso's root bone to attach to:
        Ogre::Bone *trbone = mSkeleton->getBone(mTorsos[i].root);

        for (int f = 0; f < mTorsos[i].fixed_count; ++f) {
            // here we go. one bone at time
            trbone->createChild(mTorsos[i].fixed_joints[f],
                                toVector(mTorsos[i].fixed_joint_diff_coord[f]),
                                Quaternion::IDENTITY);
        }
    }

    // Torsos are processed. Now limbs
    for (int i = 0; i < mHeader.num_limbs; ++i) {
        // get the attachment bone
        Ogre::Bone *atb = mSkeleton->getBone(mLimbs[i].attachment_joint);

        for (int s = 0; s < mLimbs[i].num_segments; ++s) {
            Ogre::Bone *nb =
                atb->createChild(mLimbs[i].segments[s],
                                 mLimbs[i].lengths[s] *
                                     toVector(mLimbs[i].segment_diff_coord[s]),
                                 Quaternion::IDENTITY);
            atb = nb; // now, the new bone becomes the parent for the next bone
        }
    }
    // Voila, All done. I said it's quite simple
}

void CalSkeletonLoader::readHeader(void) {
    *mFile >> mHeader;
}

void CalSkeletonLoader::readTorsos(void) {
    // sanity checks
    if (mHeader.num_torsos < 1)
        OPDE_EXCEPT(format("Cal file has zero torsos : ", mFileName));

    // hard to imagine a body with 512 torsos... a cartepillar? :)
    if (mHeader.num_torsos > 512)
        OPDE_EXCEPT(format("Cal file has more than 512 torsos : ", mFileName));

    mTorsos.resize(mHeader.num_torsos);
    *mFile >> mTorsos;
}

void CalSkeletonLoader::readLimbs(void) {
    if (mHeader.num_limbs > 512)
        OPDE_EXCEPT(format("Cal file has more than 512 limbs : ", mFileName));

    mLimbs.resize(mHeader.num_limbs);
    *mFile >> mLimbs;
}

/** Helper SubMesh filler class. Receives a pointer to the UV and Vertex arrays,
 * and is filled with triangles */
class SubMeshFiller {
public:
    SubMeshFiller(Ogre::SubMesh *sm, const std::vector<Vertex> &vertices,
                  const std::vector<Vertex> &normals,
                  const std::vector<ObjLight> &lights,
                  const std::vector<UVMap> &uvs, bool useuvmap)
        : mVertices(vertices),
          mNormals(normals),
          mLights(lights),
          mUVs(uvs),
          mUseUV(useuvmap),
          mBuilt(false),
          mSubMesh(sm),
          mSkeleton(),
          mColour(),
          mTrans(0.0f){};

    ~SubMeshFiller(){};

    void setMaterialName(const std::string &matname) {
        mMaterialName = matname;
        mSubMesh->setMaterialName(mMaterialName);
    };

    bool needsUV() { return mUseUV; };

    void addPolygon(int bone, size_t numverts, uint16_t normal,
                    const std::vector<uint16_t> &vidx,
                    const std::vector<uint16_t> &lightidx,
                    const std::vector<uint16_t> &uvidx);

    /// For AI meshes. All 3 coords index vert, norm and uv at once
    void addTriangle(uint16_t a, uint16_t bone_a,
                     uint16_t b, uint16_t bone_b,
                     uint16_t c, uint16_t bone_c);

    void setSkeleton(const Ogre::SkeletonPtr &skel) { mSkeleton = skel; };

    void setColour(const Ogre::ColourValue &cv) { mColour = cv; };

    void setTransparency(float trans) { mTrans = trans; };

    void build();

    struct VertexDefinition {
        uint16_t vertex;
        uint16_t normal;
        uint16_t light;
        uint16_t uvidx; // Left zero if the UV's are not used
        int bone;
    };

private:
    uint16_t getIndex(int bone, uint16_t vert, uint16_t norm, uint16_t light,
                      uint16_t uv);

    /// Unpacks the 30 bit normal from light struct (at index idx)
    Vector3 getUnpackedNormal(uint16_t idx);

    /// Global vertex data pointer
    const std::vector<Vertex> &mVertices;
    /// Global normals data pointer
    const std::vector<Vertex> &mNormals;
    // Lights (~normals)
    std::vector<ObjLight> mLights;
    /// Global UV table pointer
    const std::vector<UVMap> &mUVs;
    /// used Material name
    std::string mMaterialName;
    /// Uses UVs
    bool mUseUV;
    /// Already built
    bool mBuilt;
    /// The submesh being filled
    Ogre::SubMesh *mSubMesh;
    /// Skeleton pointer used to transform initial vertex positions and normals
    /// (Has to be non-null prior to build operation)
    Ogre::SkeletonPtr mSkeleton;


    typedef std::vector<uint16_t> IndexList;
    IndexList mIndexList;

    typedef std::vector<VertexDefinition> VertexList;

    VertexList mVertexList;

    Ogre::ColourValue mColour;

    float mTrans;
};

bool operator==(const SubMeshFiller::VertexDefinition &a,
                const SubMeshFiller::VertexDefinition &b) {
    return ((a.vertex == b.vertex) && (a.normal == b.normal) &&
            (a.uvidx == b.uvidx) && (a.bone == b.bone) && (a.light == b.light));
}

void SubMeshFiller::addPolygon(int bone, size_t numverts, uint16_t normal,
                               const std::vector<uint16_t> &vidx,
                               const std::vector<uint16_t> &lightidx,
                               const std::vector<uint16_t> &uvidx)
{
    // For each of the vertices, search for the vertex/normal combination
    // If not found, insert one
    // As we triangulate the polygon, the order is slightly different from
    // simple 0-(n-1)
    uint16_t last_index;
    uint16_t max_index;

    if (mUseUV) {
        last_index = getIndex(bone, vidx[0], normal, lightidx[0], uvidx[0]);
        max_index = getIndex(bone, vidx[numverts - 1], normal,
                             lightidx[numverts - 1], uvidx[numverts - 1]);
    } else {
        last_index = getIndex(bone, vidx[0], normal, lightidx[0], 0);
        max_index = getIndex(bone, vidx[numverts - 1], normal,
                             lightidx[numverts - 1], 0);
    }

    for (size_t i = 1; i < (numverts - 1); i++) {
        uint16_t index;

        if (mUseUV)
            index = getIndex(bone, vidx[i], normal, lightidx[i], uvidx[i]);
        else
            index = getIndex(bone, vidx[i], normal, lightidx[i], 0);

        mIndexList.push_back(max_index);
        mIndexList.push_back(index);
        mIndexList.push_back(last_index);

        last_index = index;
    }
}

void SubMeshFiller::addTriangle(uint16_t a, uint16_t bone_a, uint16_t b,
                                uint16_t bone_b, uint16_t c, uint16_t bone_c)
{

    uint16_t idxa = getIndex(bone_a, a, a, 0, a);
    uint16_t idxb = getIndex(bone_b, b, b, 0, b);
    uint16_t idxc = getIndex(bone_c, c, c, 0, c);

    mIndexList.push_back(idxa);
    mIndexList.push_back(idxb);
    mIndexList.push_back(idxc);
}

uint16_t SubMeshFiller::getIndex(int bone, uint16_t vert, uint16_t norm,
                                 uint16_t light, uint16_t uv)
{
    // Find the record with the same parameters
    // As I'm a bit lazy, I do this by iterating the whole vector
    // Check the limits!
    if (vert >= mVertices.size())
        OPDE_EXCEPT("Vertex Index out of range!");

    // TODO: What takes precedence? Light's or normal's index?
    if (mLights.empty()) {
        if (norm >= mNormals.size())
            OPDE_EXCEPT("Normal Index out of range!");
    } else {
        if (light >= mLights.size())
            OPDE_EXCEPT("Light Index out of range!");
    }

    if (uv >= mUVs.size() && mUseUV)
        OPDE_EXCEPT("UV Index out of range!");

    VertexDefinition vdef;
    vdef.vertex = vert;

    if (mLights.empty()) {
        vdef.normal = norm;
        vdef.light = 0;
    } else {
        vdef.normal = 0;
        vdef.light = light;
    }

    vdef.uvidx = uv;
    vdef.bone  = bone;

    uint16_t index = 0;
    for (const auto &vert : mVertexList) {
        // If the records match, return index
        if (vdef == vert) return index;
        ++index;
    }

    // Push the vdef in the vert. list
    mVertexList.push_back(vdef);

    // Push the vertex bone binding as well
    return mVertexList.size() - 1;
}

Vector3 SubMeshFiller::getUnpackedNormal(uint16_t idx) {
    // get the ref to light struct at idx
    if (idx >= mLights.size())
        OPDE_EXCEPT("Light Index out of range!");

    uint32_t src = mLights[idx].packed_normal;
    auto v = unpack_normal(src);
    return {v.x, v.y, v.z};
}

void SubMeshFiller::build() {
    if (mBuilt)
        return;

    // Ambient col is self illum times transparency
    Ogre::RGBA diffuseCol =
        Ogre::ColourValue(mColour.r, mColour.g, mColour.b, 1.0f - mTrans)
            .getAsRGBA();

    if (!mSkeleton)
        OPDE_EXCEPT("No skeleton given prior to build!");

    mSubMesh->operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;

    mSubMesh->useSharedVertices = false;

    mSubMesh->vertexData = new Ogre::VertexData();

    mSubMesh->vertexData->vertexCount = mVertexList.size();

    mSubMesh->setBuildEdgesEnabled(true);

    Ogre::VertexDeclaration *decl = mSubMesh->vertexData->vertexDeclaration;

    size_t offset = 0;

    // Vertex and normal data
    decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

    decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

    // Diffuse vertex colour - used for tinting/alpha transparency
    decl->addElement(0, offset, Ogre::VET_COLOUR, Ogre::VES_DIFFUSE);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_COLOUR);

    // Build the VertexBuffer and vertex declaration
    Ogre::HardwareVertexBufferSharedPtr vbuf =
        Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
            offset, mSubMesh->vertexData->vertexCount,
            Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);

    size_t elemsize = 2 * 3 + 1;

    // Build the buffer. 3 floats position, 3 floats normal
    float *fptr = new float[mSubMesh->vertexData->vertexCount * elemsize];

    float *f = fptr;

    VertexList::const_iterator it = mVertexList.begin();

    size_t vidx = 0;

    for (; it != mVertexList.end(); ++it, ++vidx) {
        // Transform the vertex position and normal, then put into the buffer
        Vector3 pos(mVertices[it->vertex].x, mVertices[it->vertex].y,
                    mVertices[it->vertex].z);
        Vector3 norm;

        if (!mLights.empty()) {
            norm = getUnpackedNormal(it->light);
        } else {
            norm = Vector3(mNormals[it->normal].x,
                           mNormals[it->normal].y,
                           mNormals[it->normal].z);
        }

        Vector3 npos, nnorm;

        // Get the bone!
        Ogre::Bone *b = mSkeleton->getBone(it->bone);

        if (b == NULL)
            OPDE_EXCEPT(format("Invalid bone index encountered : ", it->bone));

        Ogre::Matrix4 tf = b->_getFullTransform();

        // Get the global transform the bone gives us
        // b->getWorldTransforms();

        // Transform the vertices
        npos.x =
            tf[0][0] * pos.x + tf[0][1] * pos.y + tf[0][2] * pos.z + tf[0][3];

        npos.y =
            tf[1][0] * pos.x + tf[1][1] * pos.y + tf[1][2] * pos.z + tf[1][3];

        npos.z =
            tf[2][0] * pos.x + tf[2][1] * pos.y + tf[2][2] * pos.z + tf[2][3];

        nnorm.x = tf[0][0] * norm.x + tf[0][1] * norm.y + tf[0][2] * norm.z;

        nnorm.y = tf[1][0] * norm.x + tf[1][1] * norm.y + tf[1][2] * norm.z;

        nnorm.z = tf[2][0] * norm.x + tf[2][1] * norm.y + tf[2][2] * norm.z;

        f[0] = npos.x;
        f[1] = npos.y;
        f[2] = npos.z;

        // Normal - see vertex decl for comments
        f[3] = nnorm.x;
        f[4] = nnorm.y;
        f[5] = nnorm.z;

        // Diffuse colour - 32 bit
        *((Ogre::RGBA *)&f[6]) = diffuseCol;

        f += elemsize;
    }

    vbuf->writeData(0, vbuf->getSizeInBytes(), fptr, true);

    delete[] fptr;

    Ogre::VertexBufferBinding *bind = mSubMesh->vertexData->vertexBufferBinding;

    bind->unsetAllBindings();

    bind->setBinding(0, vbuf);

    // Second buffer. I have to use two, otherwise the uv's are screwed because
    // of the skeletal anim.
    if (mUseUV) {
        offset = 0;

        decl->addElement(1, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
        offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);

        Ogre::HardwareVertexBufferSharedPtr tuvvbuf =
            Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
                offset, mSubMesh->vertexData->vertexCount,
                Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);

        elemsize = 2;

        fptr = new float[elemsize * mSubMesh->vertexData->vertexCount];
        f = fptr;

        it = mVertexList.begin();

        for (; it != mVertexList.end(); ++it) {
            *(f++) = mUVs[it->uvidx].u;
            *(f++) = mUVs[it->uvidx].v;
        }

        tuvvbuf->writeData(0, tuvvbuf->getSizeInBytes(), fptr, true);

        bind->setBinding(1, tuvvbuf);

        delete[] fptr;
    }

    size_t ibufCount = mIndexList.size();

    // skeletalAnimation yep, vertexAnimation Nope, vertexAnimationNormals nope
    mSubMesh->vertexData->reorganiseBuffers(
        decl->getAutoOrganisedDeclaration(true, false, false));

    // Build the index buffer
    // We have that done already, just copy the indices
    Ogre::HardwareIndexBufferSharedPtr ibuf =
        Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
            Ogre::HardwareIndexBuffer::IT_16BIT, ibufCount,
            Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);

    /// Upload the index data
    ibuf->writeData(0, ibuf->getSizeInBytes(), mIndexList.data(), true);

    // Assign bone bindings
    // VertexBoneBindings::const_iterator bit = mBoneBindings.begin();
    it = mVertexList.begin();
    int idx = 0;
    for (; it != mVertexList.end(); ++it, ++idx) {
        // insert a binding
        Ogre::VertexBoneAssignment vba;
        vba.boneIndex = it->bone;
        vba.vertexIndex = idx;
        vba.weight =
            1.0f; // TODO: Stretchy vertices will have this split to two parts

        mSubMesh->addBoneAssignment(vba);
    }

    /// Set parameters of the submesh
    mSubMesh->indexData->indexBuffer = ibuf;
    mSubMesh->indexData->indexCount = ibufCount;
    mSubMesh->indexData->indexStart = 0;

    mSubMesh->_compileBoneAssignments();

    mBuilt = true;
}

/* Loader classes. Not for public use */

/// Abstract parent for bin file loaders. Handles the common parts of model
/// loading
class DarkBINFileLoader {
public:
    DarkBINFileLoader(Ogre::Mesh *mesh, const Opde::FilePtr &file,
                      unsigned int version)
        : mVersion(version), mMesh(mesh), mFile(file){};

protected:
    unsigned int mVersion;
    Ogre::Mesh *mMesh;
    Opde::FilePtr mFile;

    typedef std::map<int, Ogre::MaterialPtr> OgreMaterials;
    typedef std::map<int, std::unique_ptr<SubMeshFiller>> FillerMap;
};

/** Object Mesh loader class. identified by LGMD in the file offset 0. Accepts
 * revision 3, 4 models. This class fills the supplied mesh with a Ogre's
 * version of the Mesh. */
class ObjectMeshLoader : public DarkBINFileLoader {
public:
    ObjectMeshLoader(Ogre::Mesh *mesh, const Opde::FilePtr &file,
                     unsigned int version)
        : DarkBINFileLoader(mesh, file, version), mMaterials(),
          mMaterialsExtra(), mVHots(), mVertices(), mNormals(),
          mLights(), mUVs(), mSubObjects(), mNumUVs(0),
          mNumNorms(0), mNumLights(0), mMaxSlot(0) {

        if ((mVersion != 3) && (mVersion != 4))
            OPDE_EXCEPT(format("Unsupported object mesh version : ", mVersion));
    };

    ~ObjectMeshLoader() {
        // cleanout
        mMaterials.clear();
        mMaterialsExtra.clear();
        mVHots.clear();
        mVertices.clear();
        mUVs.clear();
        mSubObjects.clear();
        mNormals.clear();
        mLights.clear();
        mFillers.clear();
    };

    void load();

private:
    void readObjects();
    void loadSubObject(int obj, int parent);
    void loadSubNode(int obj, size_t offset);
    void loadPolygons(int obj, size_t count);
    void readBinHeader();
    void readMaterials();
    void readVHot();
    void readUVs();
    void readVertices();
    void readNormals();
    void readLights();

    Ogre::Matrix3 convertRotation(const float m[9]);

    SubMeshFiller *getFillerForPolygon(ObjPolygon &ply);

    int getMaterialIndex(int slotidx);
    /// Creates a material from the pallete index (solid color material)
    Ogre::MaterialPtr createPalMaterial(const std::string &matname,
                                        int palindex);

    Ogre::MaterialPtr prepareMaterial(const std::string &matname,
                                      MeshMaterial &mat,
                                      MeshMaterialExtra &matext);

    BinHeader mHdr;
    std::vector<MeshMaterial> mMaterials;
    std::vector<MeshMaterialExtra> mMaterialsExtra;
    std::vector<VHotObj> mVHots;
    std::vector<Vertex> mVertices;
    std::vector<Vertex> mNormals;
    std::vector<ObjLight> mLights;

    std::vector<UVMap> mUVs;
    std::vector<SubObjectHeader> mSubObjects;

    int mNumUVs;
    int mNumNorms;
    int mNumLights;

    std::map<int, int> mSlotToMatNum; // only v3 uses this. v4 is filled 1:1

    FillerMap mFillers;

    OgreMaterials mOgreMaterials;

    Ogre::SkeletonPtr mSkeleton;

    //			MaterialServicePtr mMaterialService;

    // maximal slot ID, used for extra materials, which are not unmapped to
    // slots
    unsigned int mMaxSlot;
};

class AIMeshLoader : public DarkBINFileLoader {
public:
    AIMeshLoader(Ogre::Mesh *mesh, const Opde::FilePtr &file,
                 unsigned int version);
    ~AIMeshLoader();

    void load();

protected:
    void readHeader();
    void readMaterials();
    void readMappers();
    void readJoints();
    void readTriangles();
    void readVertices();
    void readNormals();

    void readUVs();

    Ogre::MaterialPtr prepareMaterial(const std::string &matname, AIMaterial &mat);

    SubMeshFiller *getFillerForSlot(int slot);

    /// LGMM header (offset 0x08 in file)
    AIMeshHeader mHeader;

    /// joint remappings (?)
    std::vector<uint8_t> mJointsIn, mJointsOut;

    /// source of material definitions
    std::vector<AIMaterial> mMaterials;

    /// remaps .BIN joints to .CAL joints (in bin, i'd call those chunks in
    /// fact, not joints)
    std::vector<AIMapper> mMappers;

    /// geometry chunks per mapper
    std::vector<AIJointInfo> mJoints;

    /// vertices
    std::vector<Vertex> mVertices;

    /// triangle normals
    std::vector<Vertex> mTriNormals;

    /// vertex normals
    std::vector<Vertex> mNormals;

    /// uv's (stored as Vertex in file, but we use the same submesh filler
    /// class)
    std::vector<UVMap> mUVs;

    /// AI triangles
    std::vector<AITriangle> mTriangles;

    /// materials per slot ID
    OgreMaterials mOgreMaterials;

    /// Fillers for polys
    FillerMap mFillers;

    /// Loader for the skeleton
    std::unique_ptr<CalSkeletonLoader> mCalLoader;

    /// simple mapper for vertex->joint
    std::vector<uint8_t> mVertexJointMap;
};

/*-----------------------------------------------------------------*/
/*--------------------- ObjectMeshLoader    -----------------------*/
/*-----------------------------------------------------------------*/
void ObjectMeshLoader::load() {
    // read all the fields of the BIN header...
    readBinHeader();

    // progress with loading. Calculate the count of the UV records
    mNumUVs = (mHdr.offset_vhots - mHdr.offset_uv) / (sizeof(float) * 2);
    mNumNorms = (mHdr.offset_pgons - mHdr.offset_norms) / (sizeof(float) * 3);
    mNumLights = (mHdr.offset_norms - mHdr.offset_light) / ObjLight_Size;

    // Read the materials
    readMaterials();

    // read UV
    readUVs();

    // Read the VHots if present
    readVHot();

    // Read the vertices
    readVertices();

    // Read the light vectors
    readLights();

    // Read the normals
    readNormals();

    // Everything is ready. Can proceed with the filling
    readObjects();

    // Final step. Build the submeshes
    FillerMap::iterator it = mFillers.begin();

    for (; it != mFillers.end(); ++it) {
        it->second->setSkeleton(mSkeleton);
        it->second->build();
    }

    // Cubic bounds

    /* TODO: some meshes violate the rules min<max, so temporarily commented
     mMesh->_setBounds(AxisAlignedBox(mHdr.bbox_min[0], mHdr.bbox_min[1],
     mHdr.bbox_min[2], mHdr.bbox_max[0],mHdr.bbox_max[1],mHdr.bbox_max[2]));*/

    // Spherical radius
    mMesh->_setBoundingSphereRadius(mHdr.sphere_rad);

    mMesh->prepareForShadowVolume();
    mMesh->buildEdgeList();

    // mMesh->load();

    // DONE!
    mMesh->_updateCompiledBoneAssignments();
}

//---------------------------------------------------------------
void ObjectMeshLoader::readObjects() {
    // create the skeleton for further usage
    // name it the same as the mesh
    mSkeleton = Ogre::SkeletonManager::getSingleton().create(
            format(mMesh->getName(), "-Skeleton"), mMesh->getGroup());

    mMesh->_notifySkeleton(mSkeleton);

    // Seek at the beginning of the objects
    // Load all the subobj headers
    mFile->seek(mHdr.offset_objs);
    mSubObjects.resize(mHdr.num_objs);
    *mFile >> mSubObjects;

    /* Some notes:
    All begins with the root sub-object. Seems to always be 0
    All subobjs have two params. Child, Next. So what happens?
    ...
    Recursion!

    Concept is that every sub-object is a child of some other, or is a root
    (Only one of that type is per object, and has index 0).

    1) If child is >=0, we have a child object index. Recurse
    2) If next object is >=0, we have a colegue. Continue on the same level
    */

    // The subobject table is loaded. Now load all the subobj's (Recurses. This
    // calls the root)
    loadSubObject(0, -1);

    mSkeleton->_notifyManualBonesDirty();
    mSkeleton->setBindingPose();
}

//-------------------------------------------------------------------
void ObjectMeshLoader::loadSubObject(int obj, int parent) {
    // Bone has to be prepared. It can either be root (parent is -1) or attached
    // (parent>=0)
    int subobj = obj;

    while (subobj >= 0) {
        if (parent >= 0) {
            // Set the bone position and transformation
            const Vertex &pos = mSubObjects[subobj].trans.axle_point;
            Vector3 v(pos.x, pos.y, pos.z);

            Quaternion q;

            q.FromRotationMatrix(
                convertRotation(mSubObjects[subobj].trans.rot));

            // We have a bone that should connect to a parent bone
            Ogre::Bone *bone = mSkeleton->getBone(parent)->createChild(subobj);

            bone->setPosition(v);
            bone->setOrientation(q);
            bone->needUpdate();

            bone->setManuallyControlled(true);
        } else {
            // We have a root bone!
            Ogre::Bone *bone = mSkeleton->createBone(subobj); // root bone it is

            bone->resetOrientation();

            const Vertex &pos = mSubObjects[subobj].trans.axle_point;

            if (subobj != 0) { // Only if the root bone is not the subobject 0
                               // (transform seems bogus on that one)
                Quaternion q;
                bone->setPosition(Vector3(pos.x, pos.y, pos.z));
                q.FromRotationMatrix(
                    convertRotation(mSubObjects[subobj].trans.rot));
                bone->setOrientation(q);
            }

            bone->setManuallyControlled(true);
        }

        if (mSubObjects[subobj].child_sub_obj >= 0) { // children to come
            // Load the children as well
            loadSubObject(mSubObjects[subobj].child_sub_obj, subobj);
        }

        // load the geometry of this sub-object
        // We expect a reference to a root Node of the subobj
        loadSubNode(subobj, mSubObjects[subobj].node_start);

        // next child of the parent to come?
        subobj = mSubObjects[subobj].next_sub_obj;
    }
}

//-------------------------------------------------------------------
void ObjectMeshLoader::loadSubNode(int obj, size_t offset) {
    mFile->seek(mHdr.offset_nodes + offset);

    // read the type
    uint8_t type;
    mFile->read(&type, 1);

    if (type == MD_NODE_HDR) {
        NodeHeader ndhdr;
        *mFile >> ndhdr;

        if (obj == ndhdr.subObjectID) {
            // only skip this node's size
            loadSubNode(obj, offset + NodeHeader::SIZE);
        }
    } else if (type == MD_NODE_SPLIT) {
        NodeSplit ns;
        *mFile >> ns;

        loadPolygons(obj, ns.pgon_before_count + ns.pgon_after_count);

        loadSubNode(obj, ns.behind_node);
        loadSubNode(obj, ns.front_node);
    } else if (type == MD_NODE_CALL) {
        NodeCall nc;
        *mFile >> nc;

        // TODO: This seems not to work. Seeks past end of file and all that
        // jazz... It sure seems that the call in the title is just a name. If
        // something is missing, do a revision of this loadSubNode (obj,
        // nc.call_node);

        loadPolygons(obj, nc.pgon_before_count + nc.pgon_after_count);
    } else if (type == MD_NODE_RAW) {
        NodeRaw nr;
        *mFile >> nr;

        loadPolygons(obj, nr.pgon_count);
    } else {
        OPDE_EXCEPT(format("Unknown node type ", type,
                           " at offset ", offset));
    };
}

//-------------------------------------------------------------------
void ObjectMeshLoader::loadPolygons(int obj, size_t count) {
    // Step one - Read the polygon index list from the current position
    std::vector<uint16_t> polys(count);

    // These are offsets from the polygon
    // offset in header, not plain index list
    *mFile >> polys;

    // TODO: Step two - insert the polygons into the subobjects (load a poly and
    // insert each step)
    for (size_t n = 0; n < count; n++) {
        // Load one polygon...
        mFile->seek(mHdr.offset_pgons + polys[n]);

        // Polygon header.
        ObjPolygon op;
        *mFile >> op;

        // Won't give us a filler that does not need UV's when we have those or
        // oposite
        SubMeshFiller *f = getFillerForPolygon(op);

        // Load the indices for all 3 types, as needed
        std::vector<uint16_t> verts(op.num_verts);
        *mFile >> verts;

        // TODO: These are lights, not normals, and it will probably be bad to
        // do it like this
        std::vector<uint16_t> norms(op.num_verts);
        *mFile >> norms;

        std::vector<uint16_t> uvs;

        if (f->needsUV()) {
            uvs.resize(op.num_verts);
            *mFile >> uvs;
        }

        f->addPolygon(obj, op.num_verts, op.norm, verts, norms, uvs);
    }
}

//-------------------------------------------------------------------
void ObjectMeshLoader::readBinHeader() {
    mHdr.read(*mFile, mVersion);
}

//-------------------------------------------------------------------
void ObjectMeshLoader::readMaterials() {
    mFile->seek(mHdr.offset_mats);

    mMaxSlot = 0;

    // Allocate the materials
    mMaterials.resize(mHdr.num_mats);

    // for each of the materials, load the struct
    for (auto &mat : mMaterials) {
        // load a single material
        mFile->read(mat.name, 16);

        *mFile >> mat.type >> mat.slot_num;

        mMaxSlot = std::max(mMaxSlot, (unsigned int)mat.slot_num);

        // Material type variable part. 8 bytes total
        if (mat.type == MD_MAT_COLOR) {
            mFile->read(mat.colour, 4);
            *mFile >> mat.ipal_index;
        } else if (mat.type == MD_MAT_TMAP) {
            *mFile >> mat.handle >> mat.uvscale;
        } else
            OPDE_EXCEPT(format("Unknown Material type : ", mat.type));
    }

    // construct anyway
    mMaterialsExtra.resize(mHdr.num_mats);

    for (auto &mext : mMaterialsExtra) {
        mext.illum = 0;
        mext.trans = 0;
    }

    // we only know how to handle 8 byte slot records
    if (mVersion > 3) {
        mFile->seek(mHdr.offset_mat_extra);

        // bytes to skip per record (extra bytes beyond what we read)
        int extralen = (mHdr.size_mat_extra - 0x08);

        // in debug, reveal that there is a problem with mat. extra size...
        assert(extralen >= 0);

        if (extralen >= 0) {
            // if we need extended material attributes
            for (auto &mext : mMaterialsExtra) {
                *mFile >> mext.trans >> mext.illum;

                if (extralen > 0)
                    mFile->seek(extralen, File::FSEEK_CUR);
            }
        }
    }

    // Prepare the material slot mapping, if used
    // This means, slot index will point to material index
    for (int n = 0; n < mHdr.num_mats; n++) {
        // if (mVersion == 3) {
        mSlotToMatNum.insert(make_pair(mMaterials[n].slot_num, n));
        //} else
        //    mSlotToMatNum.insert(make_pair(n,n)); // plain mapping used
    }

    // Now prepare the raw material index to Ogre Material reference mapping to
    // be used through the conversion
    for (int n = 0; n < mHdr.num_mats; n++) {
        Ogre::MaterialPtr mat = prepareMaterial(
                format(mMesh->getName(), "/", mMaterials[n].name),
                mMaterials[n], mMaterialsExtra[n]);

        mOgreMaterials.emplace(n, mat);
    }
}

//-------------------------------------------------------------------
void ObjectMeshLoader::readVHot() {
    // seek and load all the VHot, if present
    if (mHdr.num_vhots > 0) {
        mVHots.resize(mHdr.num_vhots);
        mFile->seek(mHdr.offset_vhots);
        *mFile >> mVHots;
    }
}

//-------------------------------------------------------------------
void ObjectMeshLoader::readUVs() {
    // read
    if (mNumUVs > 0) { // can be zero if all the model is color only (No TXT)
        mUVs.resize(mNumUVs);
        // seek to the offset
        mFile->seek(mHdr.offset_uv);
        *mFile >> mUVs;
    }
}

//-------------------------------------------------------------------
void ObjectMeshLoader::readVertices() {
    if (mHdr.num_verts > 0) {
        mVertices.resize(mHdr.num_verts);
        mFile->seek(mHdr.offset_verts);
        *mFile >> mVertices;
    } else
        OPDE_EXCEPT("Number of vertices is zero!");
}

//-------------------------------------------------------------------
void ObjectMeshLoader::readNormals() {
    if (mNumNorms > 0) {
        mNormals.resize(mNumNorms);
        mFile->seek(mHdr.offset_norms);
        *mFile >> mNormals;
    } else
        OPDE_EXCEPT("Number of normals is zero!");
}

//-------------------------------------------------------------------
void ObjectMeshLoader::readLights() {
    if (mNumLights > 0) {
        mLights.resize(mNumLights);
        mFile->seek(mHdr.offset_light);
        *mFile >> mLights;
    } else
        OPDE_EXCEPT("Number of normals is zero!");
}

//-------------------------------------------------------------------
Ogre::Matrix3 ObjectMeshLoader::convertRotation(const float m[9]) {
    // Convert to the Ogre::Matrix3
    Ogre::Matrix3 mat(m[0], m[3], m[6], m[1], m[4], m[7], m[2], m[5], m[8]);
    return mat;
}

//-------------------------------------------------------------------
SubMeshFiller *ObjectMeshLoader::getFillerForPolygon(ObjPolygon &ply) {
    int type = ply.type & 0x07;
    int color_mode = ply.type & 0x60; // used only for MD_PGON_SOLID or
                                      // MD_PGON_WIRE (WIRE is just a guess)

    FillerMap::iterator it = mFillers.end();
    std::string matName = "";
    int fillerIdx = -1;
    bool use_uvs = true;

    // TODO: MD_PGON_WIRE

    // Depending on the type, find the right filler, or construct one if not yet
    // constructed
    if (type == MD_PGON_TMAP) {
        // Get the material index from the index (can be slot idx...)
        int matidx = getMaterialIndex(ply.data);

        // Color mode is ignored. We search the filler table simply by the
        // material index
        it = mFillers.find(matidx);
        fillerIdx = matidx;
        matName = mOgreMaterials[matidx]->getName();
        // mMesh->getName() + "/" + mMaterials[matidx].name;

    } else if (type == MD_PGON_SOLID) {
        use_uvs = false;

        // Solid color polygon. This means we need to see if we use Material or
        // Color table index
        if (color_mode == MD_PGON_SOLID_COLOR_PAL) {
            // Dynamically created material. We allocate negative numbers for
            // these fillers Color mode is ignored. We search the filler table
            // simply by the material index

            matName = format(mMesh->getName(), "/Color", ply.index);

            // TODO: See if the material already existed or not...
            // Generate the solid material...
            Ogre::MaterialPtr material = createPalMaterial(matName, ply.index);

            it = mFillers.find(-ply.index);
            fillerIdx = -ply.index;

            mOgreMaterials.insert(make_pair(fillerIdx, material));

        } else if (color_mode == MD_PGON_SOLID_COLOR_VCOLOR) {
            int matidx = getMaterialIndex(ply.data);

            it = mFillers.find(matidx);
            fillerIdx = matidx;
            matName = mMesh->getName() + "/" + mMaterials[matidx].name;
        } else
            OPDE_EXCEPT("Unrecognized color_mode for polygon");
    } else
        OPDE_EXCEPT(format("Unknown or invalid polygon type: ", ply.type, " (",
                           type, " - ", color_mode, ")"));

    // Not found yet. Create one now
    if (it == mFillers.end()) {
        Ogre::SubMesh *sm = mMesh->createSubMesh(
                format("SubMesh", fillerIdx));

        SubMeshFiller *f = new SubMeshFiller(sm, mVertices,mNormals,
                                             mLights, mUVs, use_uvs);

        // Set the material for the submesh
        f->setMaterialName(matName);

        // Set extra parameters - transfer transparency, colour to
        // SubmeshFillers only if it is an original material
        if (fillerIdx >= 0 && fillerIdx < mMaterials.size()) {
            const MeshMaterial &mm = mMaterials[fillerIdx];
            const MeshMaterialExtra &me = mMaterialsExtra[fillerIdx];

            if ((mHdr.mat_flags & MD_MAT_TRANS) && (me.trans > 0))
                f->setTransparency(me.trans);

            if (mm.type == MD_MAT_COLOR)
                f->setColour(Ogre::ColourValue(mm.colour[2] / 255.0f,
                                               mm.colour[1] / 255.0f,
                                               mm.colour[0] / 255.0f));
        }

        mFillers.insert(make_pair(fillerIdx, f));

        return f;
    } else
        return it->second.get();
}

//-------------------------------------------------------------------
int ObjectMeshLoader::getMaterialIndex(int slotidx) {
    std::map<int, int>::iterator it = mSlotToMatNum.find(slotidx);

    if (it != mSlotToMatNum.end()) {
        return it->second;
    } else
        OPDE_EXCEPT(format("Unknown material slot index : ", slotidx));
}

//-------------------------------------------------------------------
Ogre::MaterialPtr ObjectMeshLoader::prepareMaterial(const std::string &matname,
                                                    MeshMaterial &mat,
                                                    MeshMaterialExtra &matext)
{
    // Look if the material is already defined or not (This enables anyone to
    // replace the material without modifying anything)
    if (Ogre::MaterialManager::getSingleton().resourceExists(matname)) {
        Ogre::MaterialPtr fmat =
            Ogre::MaterialManager::getSingleton().getByName(matname);
        return fmat;
    }

    // We'll create a material given the mat and matext structures
    Ogre::MaterialPtr omat = Ogre::MaterialManager::getSingleton().create(
        matname, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    omat->setLightingEnabled(true);
    omat->setReceiveShadows(false);

    // fill it with the values given
    Ogre::Pass *pass = omat->getTechnique(0)->getPass(0);
    pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);

    // Defaults:
    // Ambient is one. It is controlled by mission ambient setting...
    if ((mHdr.mat_flags & MD_MAT_TRANS) && (matext.trans > 0.01f)) {
        float a = 1 - matext.trans;
        pass->setAmbient(a, a, a);
        pass->setDiffuse(a, a, a, a);
    } else {
        pass->setAmbient(1, 1, 1);
        pass->setDiffuse(1, 1, 1, 1);
    }

    pass->setSpecular(0, 0, 0, 0);
    pass->setCullingMode(Ogre::CULL_CLOCKWISE);

    if (mat.type == MD_MAT_TMAP) {
        // TODO: This ugly code should be in some special service. Name it
        // ToolsService (getObjectTextureFileName, etc) and should handle
        // language setting!
        std::string txtname = format("txt16/", mat.name);

        if (!Ogre::ResourceGroupManager::getSingleton().resourceExists(
                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                format("txt16/", mat.name))) {
            // Not in txt16, will be in txt then...
            txtname = format("txt/", mat.name);

            if (!Ogre::ResourceGroupManager::getSingleton().resourceExists(
                    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                    txtname)) {
                OPDE_EXCEPT(format(
                    "Can't find texture in txt16 or txt folder: ", mat.name));
            }
        }

        pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
        pass->setAlphaRejectSettings(Ogre::CMPF_GREATER, 128); // Alpha rejection.
        // default depth bias
        pass->setDepthBias(0.01, 0.01);

        // Texture unit state for the main texture...
        Ogre::TextureUnitState *tus = pass->createTextureUnitState(txtname);
        // The model textures are not animated like this. They probably use a
        // texture swapping tweq or something tus =
        // mMaterialService->createAnimatedTextureState(pass, txtname,
        // ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 5);

        tus->setTextureAddressingMode(Ogre::TextureUnitState::TAM_WRAP);
        tus->setTextureCoordSet(0);
        tus->setTextureFiltering(
            Ogre::TFO_ANISOTROPIC); // TODO: Should be controlled by global setting

        // Based on vertex alpha and texture alpha
        tus->setAlphaOperation(Ogre::LBX_MODULATE, Ogre::LBS_DIFFUSE,
                               Ogre::LBS_TEXTURE);

    } else if (mat.type == MD_MAT_COLOR) {
        pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
        pass->setAmbient(Ogre::ColourValue::White); // controlled by vertex col...
        pass->setDiffuse(Ogre::ColourValue::White);

        // if transparent, we have to specify this in an empty tus
        if ((mHdr.mat_flags & MD_MAT_TRANS) && (matext.trans > 0)) {
            Ogre::TextureUnitState *tus = pass->createTextureUnitState();
            tus->setAlphaOperation(Ogre::LBX_SOURCE1, Ogre::LBS_DIFFUSE);
        }
    } else {
        OPDE_EXCEPT(format("Invalid material type : ", mat.type));
    }

    // Illumination of the material. Converted to ambient lightning here
    if ((mHdr.mat_flags & MD_MAT_ILLUM) && (matext.illum > 0)) {
        // set the illumination
        pass->setSelfIllumination(matext.illum, matext.illum, matext.illum);
    }

    omat->setShadingMode(Ogre::SO_GOURAUD);
    omat->load();

    return omat;
}

//-------------------------------------------------------------------
Ogre::MaterialPtr
ObjectMeshLoader::createPalMaterial(const std::string &matname, int palindex)
{
    if (Ogre::MaterialManager::getSingleton().resourceExists(matname)) {
        Ogre::MaterialPtr fmat =
            Ogre::MaterialManager::getSingleton().getByName(matname);
        return fmat;
    }

    assert(palindex >= 0 && palindex <= 255);

    // We'll create a material given the mat and matext structures
    Ogre::MaterialPtr omat = Ogre::MaterialManager::getSingleton().create(
        matname, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    // fill it with the values given
    Ogre::Pass *pass = omat->getTechnique(0)->getPass(0);
    pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);

    // Fill in a color-only material
    Ogre::TextureUnitState *tus = pass->createTextureUnitState();

    // set the material color from the lg system color table
    tus->setColourOperationEx(
        Ogre::LBX_SOURCE1, Ogre::LBS_MANUAL, Ogre::LBS_CURRENT,
        Ogre::ColourValue(sLGPalette[palindex].red, sLGPalette[palindex].green,
                          sLGPalette[palindex].blue));

    // tus->setColourOperation(LBO_REPLACE);

    omat->load();

    return omat;
}

/*-----------------------------------------------------------------*/
/*--------------------- AIMeshLoader        -----------------------*/
/*-----------------------------------------------------------------*/
AIMeshLoader::AIMeshLoader(Ogre::Mesh *mesh, const Opde::FilePtr &file,
                           unsigned int version)
    : DarkBINFileLoader(mesh, file, version),
      mJointsIn(),
      mJointsOut(),
      mMaterials(),
      mMappers(),
      mJoints(),
      mVertices(),
      mNormals(),
      mUVs(),
      mTriangles(),
      mCalLoader(),
      mVertexJointMap()
{
    if (mVersion > 2 || mVersion < 1)
        OPDE_EXCEPT(format("File has version outside (0,1) range, and thus "
                           "cannot be handled ", mMesh->getName()));
};

AIMeshLoader::~AIMeshLoader() {
    mJointsIn.clear();
    mJointsOut.clear();
    mMaterials.clear();
    mMappers.clear();
    mJoints.clear();
    mVertices.clear();
    mNormals.clear();
    mUVs.clear();
    mTriangles.clear();
    mVertexJointMap.clear();
    mFillers.clear();
}

void AIMeshLoader::load() {
    // For the AI mesh to be usable, we need the cal file.
    // The cal file has the same name as the BIN file, but .CAL extension
    std::string name = mMesh->getName();
    std::string group = mMesh->getGroup();

    // Get the real filename from the nameValuePairList
    // That means: truncate to the last dot, append .bin to the filename
    size_t dot_pos = name.find_last_of(".");

    std::string basename = name;
    if (dot_pos != std::string::npos) {
        basename = name.substr(0, dot_pos);
    }

    basename += ".cal";

    // load the skeleton
    mCalLoader.reset(new CalSkeletonLoader(basename, group, mMesh));

    // if this fails, at least we'll be clean on the mesh side
    // because the exception won't have to be handled for cleaning
    mCalLoader->load();

    // load
    // 1. the header
    readHeader();

    // 2. the .BIN joint ID remapping struct (I suppose this swaps .BIN joint
    // id's somehow)
    mFile->seek(mHeader.offset_joint_remap, File::FSEEK_BEG);

    mJointsIn.resize(mHeader.num_joints);
    mJointsOut.resize(mHeader.num_joints);

    *mFile >> mJointsIn;
    *mFile >> mJointsOut;

    // 3. the Joint remap info. BIN joint to .cal joint mapping, probably other
    // data as well
    readMappers();

    // 4. the materials
    readMaterials();

    // 5. Joints - triangles per .BIN joint, vertices per .BIN joint (I suppose
    // there can be some weigting done)
    readJoints();

    // 6. Polygons
    readTriangles();

    // 7. Vertices
    readVertices();

    // 8. Normals
    readNormals();

    // 9. UV's
    readUVs();

    // 10. Weights (tbd sometime further)

    // interpret.

    // pass 1 of joint mappings. Build vertex -> .CAL joint mapping info
    mVertexJointMap.assign(mHeader.num_vertices, 0);


    for (unsigned int j = 0; j < mHeader.num_joints; ++j) {
        // for every joint
        // get the real joint id
        unsigned int mapper = mJoints[j].mapper_id;

        assert(mapper < mHeader.num_mappers);

        unsigned int joint = mMappers[mapper].joint;

        for (short v = 0; v < mJoints[j].num_vertices; ++v) {
            uint16_t vtx = mJoints[j].start_vertex + v;

            mVertexJointMap[vtx] = joint;
        }
    }

    // pass 2 of joint mappings. Fill the submesh builders with vertices
    for (uint8_t j = 0; j < mHeader.num_joints; ++j) {
        // for every joint
        // get the real joint id
        // unsigned int mapper = mJoints[j].mapper_id;
        // assert(mapper < mHeader.num_mappers);
        // unsigned int joint = mMappers[mapper].joint;

        for (short v = 0; v < mJoints[j].num_polys; ++v) {
            AITriangle &tri = mTriangles[mJoints[j].start_poly + v];

            SubMeshFiller *f =
                getFillerForSlot(tri.mat); // maybe slots are not used after all

            if (!f)
                OPDE_EXCEPT("Filler not found for slot!");

            f->addTriangle(tri.vert[0], mVertexJointMap[tri.vert[0]],
                           tri.vert[1], mVertexJointMap[tri.vert[1]],
                           tri.vert[2], mVertexJointMap[tri.vert[2]]);
        }
    }

    FillerMap::iterator it = mFillers.begin();

    for (; it != mFillers.end(); ++it) {
        it->second->build();
    }

    // DONE!
    // mMesh->load();

    // DONE!
    mMesh->_updateCompiledBoneAssignments();
    // cleanup?
}

void AIMeshLoader::readHeader() {
    mHeader.read(*mFile);
}

void AIMeshLoader::readMaterials() {
    // repeat for all materials
    if (mHeader.num_mats < 1) // TODO: This could be fatal
        OPDE_EXCEPT(format("File contains no materials ", mMesh->getName()));

    mFile->seek(mHeader.offset_mats, File::FSEEK_BEG);
    mMaterials.resize(mHeader.num_mats);

    int i = 0;
    for (auto &mat : mMaterials) {
        mat.read(*mFile, mVersion);

        // prepare this as Ogre's material
        Ogre::MaterialPtr omat =
            prepareMaterial(format(mMesh->getName(), "/", mat.name), mat);

        // mOgreMaterials.insert(make_pair(mMaterials[i].slot_num, mat));
        mOgreMaterials.insert(make_pair(i, omat)); // mMaterials[i].slot_num
        ++i;
    }
}

void AIMeshLoader::readMappers() {
    if (mHeader.num_mappers < 1)
        OPDE_EXCEPT(format("File contains no mappers " + mMesh->getName()));

    mFile->seek(mHeader.offset_mappers, File::FSEEK_BEG);
    mMappers.resize(mHeader.num_mappers);
    *mFile >> mMappers;
}

void AIMeshLoader::readJoints() {
    if (mHeader.num_joints < 1)
        OPDE_EXCEPT(format("File contains no joints ", mMesh->getName()));

    mFile->seek(mHeader.offset_joints, File::FSEEK_BEG);
    mJoints.resize(mHeader.num_joints);
    *mFile >> mJoints;
}

void AIMeshLoader::readTriangles() {
    mFile->seek(mHeader.offset_poly, File::FSEEK_BEG);

    if (mHeader.num_polys < 1)
        OPDE_EXCEPT(format("File contains no polygons ", mMesh->getName()));

    mTriangles.resize(mHeader.num_polys);
    *mFile >> mTriangles;
}

void AIMeshLoader::readVertices() {
    mFile->seek(mHeader.offset_vert, File::FSEEK_BEG);

    if (mHeader.num_vertices < 1)
        OPDE_EXCEPT(format("File contains no vertices ", mMesh->getName()));

    mVertices.resize(mHeader.num_vertices);
    *mFile >> mVertices;
}

void AIMeshLoader::readNormals() {
    mFile->seek(mHeader.offset_norm, File::FSEEK_BEG);

    if (mHeader.num_vertices < 1) // TODO: This could be fatal
        OPDE_EXCEPT(format("File contains no normals ", mMesh->getName()));

    size_t num_normals = (mHeader.offset_vert - mHeader.offset_norm) / 12;

    mTriNormals.resize(num_normals);
    *mFile >> mTriNormals;
}

void AIMeshLoader::readUVs() {
    mFile->seek(mHeader.offset_uvmap, File::FSEEK_BEG);

    if (mHeader.num_vertices < 1) // TODO: This could be fatal
        OPDE_EXCEPT(format("File contains no uv's ", mMesh->getName()));

    // this packs both UV and a packed normal that we preffer

    mUVs.resize(mHeader.num_vertices);
    mNormals.resize(mHeader.num_vertices);

    for (size_t i = 0; i < mHeader.num_vertices; ++i) {
        uint32_t packed_n;
        *mFile >> mUVs[i];
        *mFile >> packed_n;
        mNormals[i] = unpack_normal(packed_n);
    }
}

Ogre::MaterialPtr AIMeshLoader::prepareMaterial(const std::string &matname,
                                                AIMaterial &mat)
{
    // Look if the material is already defined or not (This enables anyone to
    // replace the material without modifying anything)
    if (Ogre::MaterialManager::getSingleton().resourceExists(matname)) {
        Ogre::MaterialPtr fmat =
            Ogre::MaterialManager::getSingleton().getByName(matname);
        return fmat;
    }

    // We'll create a material given the mat and matext structures
    Ogre::MaterialPtr omat = Ogre::MaterialManager::getSingleton().create(
        matname, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    omat->setLightingEnabled(true);
    omat->setReceiveShadows(false);

    // fill it with the values given
    Ogre::Pass *pass = omat->getTechnique(0)->getPass(0);
    pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);

    // Defaults:
    // Ambient is one. It is controlled by mission ambient setting...
    pass->setAmbient(1, 1, 1);
    pass->setDiffuse(1, 1, 1, 1);
    pass->setSpecular(0, 0, 0, 0);
    pass->setCullingMode(Ogre::CULL_CLOCKWISE);

    if (mat.type == MD_MAT_TMAP) {
        // Texture unit state for the main texture...
        Ogre::TextureUnitState *tus;

        // TODO: This ugly code should be in some special service. Name it
        // ToolsService (getObjectTextureFileName, etc) and should handle
        // language setting!
        std::string txtname = format("txt16/", mat.name);
        // First, let's look into txt16 dir, then into the txt dir. I try to
        try {
            Ogre::TextureManager::getSingleton().load(
                txtname, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                Ogre::TEX_TYPE_2D);
        } catch (const Ogre::Exception &) {
            // Error loading from txt16...
            txtname = format("txt/", mat.name);
        }

        pass->setAlphaRejectSettings(Ogre::CMPF_GREATER, 128); // Alpha rejection.

        // Some basic lightning settings
        tus = pass->createTextureUnitState(txtname);

        tus->setTextureAddressingMode(Ogre::TextureUnitState::TAM_WRAP);
        tus->setTextureCoordSet(0);
        tus->setTextureFiltering(Ogre::TFO_BILINEAR);

        // If the transparency is used
        if ((mat.ext_flags & MD_MAT_TRANS) && (mat.trans > 0)) {
            // set at least the transparency value for the material
            pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
            pass->setDepthWriteEnabled(false);
            tus->setColourOperation(Ogre::LBO_ALPHA_BLEND);
            pass->setAlphaRejectFunction(
                Ogre::CMPF_ALWAYS_PASS); // Alpha rejection reset. Does not live
                                         // good with the following:
            tus->setAlphaOperation(Ogre::LBX_SOURCE1, Ogre::LBS_MANUAL,
                                   Ogre::LBS_CURRENT, 1 - mat.trans);
        }

        // Illumination of the material. Converted to ambient lightning here
        if ((mat.ext_flags & MD_MAT_ILLUM) && (mat.illum > 0)) {
            // set the illumination
            pass->setSelfIllumination(mat.illum, mat.illum, mat.illum);
        }

        // omat->setCullingMode(CULL_ANTICLOCKWISE);

    } else if (mat.type == MD_MAT_COLOR) {
        // Fill in a color-only material
        Ogre::TextureUnitState *tus = pass->createTextureUnitState();

        // TODO: Code. We don't know where the color is, yet!
        Ogre::ColourValue matcolor(1, 1, 1);

        // set the material color
        // tus->setColourOperationEx(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT,
        // matcolor);
        pass->setDiffuse(matcolor);

        if ((mat.ext_flags & MD_MAT_TRANS) && (mat.trans > 0)) {
            // tus->setColourOperation(LBO_ALPHA_BLEND);
            tus->setAlphaOperation(Ogre::LBX_SOURCE1, Ogre::LBS_MANUAL,
                                   Ogre::LBS_CURRENT, 1 - mat.trans);
        }

        // Illumination of a color based material. Hmm. Dunno if this is right,
        // but i simply multiply the color with the illumination
        if ((mat.ext_flags & MD_MAT_ILLUM) && (mat.illum > 0)) {
            // set the illumination
            pass->setSelfIllumination(matcolor.r * mat.illum,
                                      matcolor.g * mat.illum,
                                      matcolor.b * mat.illum);
        }

    } else
        OPDE_EXCEPT(format("Invalid material type : ", mat.type));

    omat->setShadingMode(Ogre::SO_GOURAUD);
    omat->load();

    return omat;
}

SubMeshFiller *AIMeshLoader::getFillerForSlot(int slot) {
    FillerMap::iterator it = mFillers.find(slot);

    if (it != mFillers.end()) {
        return it->second.get();
    } else {
        // find the material name
        OgreMaterials::iterator mit = mOgreMaterials.find(slot);

        if (mit != mOgreMaterials.end()) {
            Ogre::SubMesh *sm = mMesh->createSubMesh(format("SubMesh", slot));

            static const std::vector<ObjLight> mEmptyLights;

            SubMeshFiller *f = new SubMeshFiller(
                sm, mVertices, mNormals, mEmptyLights, mUVs, true);

            // Set the material for the submesh
            f->setMaterialName(mit->second->getName());

            f->setSkeleton(mCalLoader->getSkeleton());

            mFillers.emplace(slot, f);

            return f;
        } else {
            OPDE_EXCEPT(format("Slot is not occupied by material ", slot,
                               " file ", mMesh->getName()));
        }
    }
}

/*-----------------------------------------------------------------*/
/*--------------------- ManualBinFileLoader -----------------------*/
/*-----------------------------------------------------------------*/
ManualBinFileLoader::ManualBinFileLoader() : ManualResourceLoader() {}

//-------------------------------------------------------------------
ManualBinFileLoader::~ManualBinFileLoader() {}

//-------------------------------------------------------------------
void ManualBinFileLoader::loadResource(Ogre::Resource *resource) {
    // Cast to mesh, and fill
    Ogre::Mesh *m = static_cast<Ogre::Mesh *>(resource);

    // Fill. Find the file to be loaded by the name, and load it
    std::string name = m->getName();
    std::string group = m->getGroup();

    // Get the real filename from the nameValuePairList
    // That means: truncate to the last dot, append .bin to the filename
    size_t dot_pos = name.find_last_of(".");

    std::string basename = name;
    if (dot_pos != std::string::npos) {
        basename = name.substr(0, dot_pos);
    }

    basename += ".bin";

    // Open the file, and detect the mesh type (Model/AI)
    Ogre::DataStreamPtr stream =
        Ogre::ResourceGroupManager::getSingleton().openResource(
            basename, m->getGroup(), true, resource);

    FilePtr fin = FilePtr(new OgreFile(stream));

    // This here (the mf wrap) prevents the direct use of read on bad file
    // offsets. Don't use normally. Just a fix for zzip segv
    FilePtr f = FilePtr(new MemoryFile(basename, File::FILE_R));
    static_pointer_cast<MemoryFile>(f)->initFromFile(
        *fin, fin->size()); // read the whole contents into the mem. file

    char _hdr[5];
    _hdr[4] = 0;

    uint32_t version;

    // Look at the 4 byte header
    f->read(_hdr, 4);     // read the LGMM/LGMD header
    f->read(&version, 4); // read the version int

    std::string header(_hdr);

    if (header == "LGMM") {
        // AI mesh
        AIMeshLoader ldr(m, f, version);

        try {
            ldr.load();
        } catch (const Opde::BasicException &e) {
            LOG_ERROR("An exception happened while loading the mesh %s : %s ",
                      basename.c_str(), e.getDetails().c_str());
        }
    } else if (header == "LGMD") {
        // Object model mesh
        ObjectMeshLoader ldr(m, f, version);
        try {
            ldr.load(); // that's all. Will do what's needed
        } catch (const Opde::BasicException &e) {
            LOG_ERROR("An exception happened while loading the mesh %s : %s ",
                      basename.c_str(), e.getDetails().c_str());
        }
    } else
        OPDE_EXCEPT(format("Unknown BIN model format : '", header, "'"));
}

} // namespace Opde
