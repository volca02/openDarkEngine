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
 *
 *		$Id$
 *
 *****************************************************************************/

#include "ManualBinFileLoader.h"
#include "File.h"
#include "BinFormat.h"
#include "lgcolors.h"
#include <OgreStringConverter.h>
#include <OgreMaterial.h>
#include <OgreMaterialManager.h>
#include <OgreTechnique.h>
#include <OgreSubMesh.h>
#include <OgreHardwareBufferManager.h>
#include <OgreLogManager.h>
#include <OgreSkeletonManager.h>
#include <OgreBone.h>
#include <OgreTextureManager.h>

#include <OgreMeshSerializer.h>
#include <OgreSkeletonSerializer.h>

using namespace std;
using namespace Opde; // For the Opde::File

namespace Ogre {

    /** Helper SubMesh filler class. Receives a pointer to the UV and Vertex arrays, and is filled with triangles */
    class SubMeshFiller {
        public:
            SubMeshFiller(SubMesh* sm, size_t nvert, Vertex* vertices, size_t nnorms, Vertex* normals, size_t nuvs, UVMap* uvs, bool useuvmap) :
                    mSubMesh(sm),
                    mVertices(vertices),
                    mNormals(normals),
                    mUVs(uvs),
                    mUseUV(useuvmap),
                    mBuilt(false),
                    mNumVerts(nvert),
                    mNumNorms(nnorms),
                    mNumUVs(nuvs) {
            };

            ~SubMeshFiller() {};

            void setMaterialName(String& matname) { mMaterialName = matname; mSubMesh->setMaterialName(mMaterialName); };
            bool needsUV() {return mUseUV; };

            void addPolygon(int bone, size_t numverts, uint16_t normal, uint16_t* vidx, uint16_t* lightidx, uint16_t* uvidx);

            void setSkeleton(SkeletonPtr skel) { mSkeleton = skel; };

            void build();

            typedef struct VertexDefinition {
                uint16_t vertex;
                uint16_t normal;
                uint16_t uvidx; // Left zero if the UV's are not used
                int bone;
            };

        protected:
            uint16_t getIndex(int bone, uint16_t vert, uint16_t norm, uint16_t uv);

            /// Global vertex data pointer
            Vertex* mVertices;
            /// Global normals data pointer
            Vertex* mNormals;
            /// Global UV table pointer
            UVMap* mUVs;
            /// used Material name
            String mMaterialName;
            /// Uses UVs
            bool mUseUV;
            /// Already built
            bool mBuilt;
            /// The submesh being filled
            SubMesh* mSubMesh;
            /// Skeleton pointer used to transform initial vertex positions and normals (Has to be non-null prior to build operation)
            SkeletonPtr mSkeleton;

            size_t mNumVerts, mNumNorms, mNumUVs;

            typedef std::vector< uint16_t > IndexList;
            IndexList mIndexList;

            typedef std::vector< VertexDefinition > VertexList;

            VertexList mVertexList;
    };

    bool operator==(const SubMeshFiller::VertexDefinition &a, const SubMeshFiller::VertexDefinition& b) {
        return ((a.vertex == b.vertex) && (a.normal == b.normal) && (a.uvidx == b.uvidx) && (a.bone == b.bone));
    }

    void SubMeshFiller::addPolygon(int bone, size_t numverts, uint16_t normal, uint16_t* vidx, uint16_t* lightidx, uint16_t* uvidx) {
        // For each of the vertices, search for the vertex/normal combination
        // If not found, insert one
        // As we triangulate the polygon, the order is slightly different from simple 0-(n-1)
        uint16_t last_index;
        uint16_t max_index;

        if (mUseUV) {
            /* TODO: Lights!
            last_index = getIndex(vidx[0], lightidx[0], uvidx[0]);
            max_index = getIndex(vidx[numverts-1], lightidx[numverts-1], uvidx[numverts-1]);
            */
            last_index = getIndex(bone, vidx[0], normal, uvidx[0]);
            max_index = getIndex(bone, vidx[numverts-1], normal, uvidx[numverts-1]);
        } else {
            /* see up
            last_index = getIndex(vidx[0], lightidx[0], 0);
            max_index = getIndex(vidx[numverts-1], lightidx[numverts-1], 0);
            */
            last_index = getIndex(bone, vidx[0], normal, 0);
            max_index = getIndex(bone, vidx[numverts-1], normal, 0);
        }

        for (size_t i = 1; i < (numverts-1); i++) {
            uint16_t index;

            if (mUseUV)
                // index = getIndex(vidx[i], lightidx[i], uvidx[i]);
                index = getIndex(bone, vidx[i], normal, uvidx[i]);
            else
                // lights. etc.
                index = getIndex(bone, vidx[i], normal, 0);

            mIndexList.push_back(max_index);
            mIndexList.push_back(last_index);
            mIndexList.push_back(index);

            last_index = index;
        }
    }

    uint16_t SubMeshFiller::getIndex(int bone, uint16_t vert, uint16_t norm, uint16_t uv) {
        // Find the record with the same parameters
        // As I'm a bit lazy, I do this by iterating the whole vector
        // Check the limits!
        if (mNumVerts <= vert)
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Vertex Index out of range!", "SubMeshFiller::getIndex");
        if (mNumNorms <= norm)
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Normal Index out of range!", "SubMeshFiller::getIndex");
        if (mNumUVs <= uv && mUseUV)
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "UV Index out of range!", "SubMeshFiller::getIndex");

        VertexList::const_iterator it = mVertexList.begin();
        uint16_t index = 0;

        VertexDefinition vdef;
        vdef.vertex = vert;
        vdef.normal = norm;
        vdef.uvidx = uv;
        vdef.bone = bone;

        for (; it != mVertexList.end(); ++it, ++index) {
            // If the records match, return index
            if (vdef == (*it))
                return index;
        }

        // Push the vdef in the vert. list
        mVertexList.push_back(vdef);

        // Push the vertex bone binding as well
        return index;
    }

    void SubMeshFiller::build() {
        if (mBuilt)
            return;

        if (mSkeleton.isNull())
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "No skeleton given prior to build!", "SubMeshFiller::build");

        mSubMesh->useSharedVertices = false;

        mSubMesh->vertexData = new VertexData();

        mSubMesh->vertexData->vertexCount = mVertexList.size();

        VertexDeclaration* decl = mSubMesh->vertexData->vertexDeclaration;

        size_t offset = 0;

        // Vertex and normal data
        decl->addElement(0, offset, VET_FLOAT3, VES_POSITION);
        offset += VertexElement::getTypeSize(VET_FLOAT3);

        // Need to understand the lights block first
        decl->addElement(0, offset, VET_FLOAT3, VES_NORMAL);
        offset += VertexElement::getTypeSize(VET_FLOAT3);

        // Build the VertexBuffer and vertex declaration
        HardwareVertexBufferSharedPtr vbuf =
            HardwareBufferManager::getSingleton().createVertexBuffer(
                offset, mSubMesh->vertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);

        size_t elemsize = 6; // 2 * 3

        // Build the buffer. 3 floats position, 3 floats normal
        float* fptr = new float[mSubMesh->vertexData->vertexCount * elemsize];

        float* f = fptr;

        VertexList::const_iterator it = mVertexList.begin();

        size_t vidx = 0;

        for (; it != mVertexList.end(); ++it, ++vidx) {
            // Transform the vertex position and normal, then put into the buffer
            Vector3 pos(mVertices[it->vertex].x, mVertices[it->vertex].y, mVertices[it->vertex].z);
            Vector3 norm(mNormals[it->normal].x, mNormals[it->normal].y, mNormals[it->normal].z);

            Vector3 npos, nnorm;

            // Get the bone!
            Bone* b = mSkeleton->getBone(it->bone);

            if (b == NULL)
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Invalid bone index encountered : " + StringConverter::toString(it->bone), "SubMeshFiller::build");


            Matrix4 tf = b->_getFullTransform();

            // Get the global transform the bone gives us
            // b->getWorldTransforms();

            // Transform the vertices
            npos.x = tf[0][0] * pos.x +
                     tf[0][1] * pos.y +
                     tf[0][2] * pos.z +
                     tf[0][3];

            npos.y = tf[1][0] * pos.x +
                     tf[1][1] * pos.y +
                     tf[1][2] * pos.z +
                     tf[1][3];

            npos.z = tf[2][0] * pos.x +
                     tf[2][1] * pos.y +
                     tf[2][2] * pos.z +
                     tf[2][3];

            nnorm.x = tf[0][0] * norm.x +
                      tf[0][1] * norm.y +
                      tf[0][2] * norm.z;

            nnorm.y = tf[1][0] * norm.x +
                      tf[1][1] * norm.y +
                      tf[1][2] * norm.z;

            nnorm.z = tf[2][0] * norm.x +
                      tf[2][1] * norm.y +
                      tf[2][2] * norm.z;

            f[0] = npos.x;
            f[1] = npos.y;
            f[2] = npos.z;

            // Normal - see vertex decl for comments
            f[3] = nnorm.x;
            f[4] = nnorm.y;
            f[5] = nnorm.z;

            f += elemsize;
        }

        vbuf->writeData(0, vbuf->getSizeInBytes(), fptr, true);

        delete[] fptr;

        VertexBufferBinding* bind = mSubMesh->vertexData->vertexBufferBinding;

        bind->unsetAllBindings();

        bind->setBinding(0, vbuf);

        // Second buffer. I have to use two, otherwise the uv's are screwed because of the skeletal anim.
        if (mUseUV) {
            offset = 0;

            decl->addElement(1, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES);
            offset += VertexElement::getTypeSize(VET_FLOAT2);

            HardwareVertexBufferSharedPtr tuvvbuf =
                HardwareBufferManager::getSingleton().createVertexBuffer(
                    offset, mSubMesh->vertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);

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

        mSubMesh->vertexData->reorganiseBuffers(decl->getAutoOrganisedDeclaration(true, false));

        // Build the index buffer
        // We have that done already, just copy the indices
        HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().
            createIndexBuffer(
                HardwareIndexBuffer::IT_16BIT,
                ibufCount,
                HardwareBuffer::HBU_STATIC_WRITE_ONLY);

        uint16_t* idxptr = new uint16_t[mIndexList.size()];

        uint16_t* idxs = idxptr;

        IndexList::const_iterator iit = mIndexList.begin();

        for (; iit != mIndexList.end(); ++iit) {
            *(idxs++) = *iit;
        }

        /// Upload the index data
        ibuf->writeData(0, ibuf->getSizeInBytes(), idxptr, true);

        delete[] idxptr;

        // Assign bone bindings
        // VertexBoneBindings::const_iterator bit = mBoneBindings.begin();
        it = mVertexList.begin();
        int idx = 0;
        for (; it != mVertexList.end(); ++it, ++idx) {
            // insert a binding
            VertexBoneAssignment vba;
            vba.boneIndex = it->bone;
            vba.vertexIndex = idx;
            vba.weight = 1.0f;

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

    /** Object Mesh loader class. identified by LGMD in the file offset 0. Accepts revision 3, 4 models.
    * This class fills the supplied mesh with a Ogre's version of the Mesh. */
    class ObjectMeshLoader {
        public:
            ObjectMeshLoader(Mesh* mesh, Opde::FilePtr file, unsigned int version) : mMesh(mesh),
                        mFile(file),
                        mVersion(version),
                        mMaterials(NULL),
                        mMaterialsExtra(NULL),
                        mVHots(NULL),
                        mVertices(NULL),
                        mUVs(NULL),
                        mSubObjects(NULL) {

                if ((mVersion != 3) && (mVersion != 4))
                    OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Unsupported object mesh version : " + StringConverter::toString(mVersion),"ObjectMeshLoader::ObjectMeshLoader");
            };

            ~ObjectMeshLoader() {
                // cleanout
                delete[] mMaterials;
                delete[] mMaterialsExtra;
                delete[] mVHots;
                delete[] mVertices;
                delete[] mUVs;
                delete[] mSubObjects;
                delete[] mNormals;

                FillerMap::iterator it = mFillers.begin();

                for (; it != mFillers.end(); ++it) {
                    delete it->second;
                }

                mFillers.clear();
            };

            void load();

        protected:
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

            Matrix3 convertRotation(const float m[9]);

            SubMeshFiller* getFillerForPolygon(ObjPolygon& ply);

            int getMaterialIndex(int slotidx);
            /// Creates a material from the pallete index (solid color material)
            MaterialPtr createPalMaterial(String& matname, int palindex);

            void readVertex(Vertex& vtx);
            MaterialPtr prepareMaterial(String matname, MeshMaterial& mat, MeshMaterialExtra& matext);


            unsigned int mVersion;
            Mesh* mMesh;
            Opde::FilePtr mFile;

            BinHeader mHdr;
            MeshMaterial* mMaterials;
            MeshMaterialExtra* mMaterialsExtra;
            VHotObj* mVHots;
            Vertex* mVertices;
            Vertex* mNormals;
            UVMap* mUVs;
            SubObjectHeader* mSubObjects;

            int mNumUVs;
            int mNumNorms;


            std::map<int, int> mSlotToMatNum; // only v3 uses this. v4 is filled 1:1
            typedef std::map< int, SubMeshFiller* > FillerMap;

            FillerMap mFillers;

            typedef std::map<int, MaterialPtr> OgreMaterials;

            OgreMaterials mOgreMaterials;

            SkeletonPtr mSkeleton;
    };

    //---------------------------------------------------------------
    void ObjectMeshLoader::load() {
        // read all the fields of the BIN header...
        readBinHeader();

        // progress with loading. Calculate the count of the UV records
        mNumUVs = (mHdr.offset_vhots - mHdr.offset_uv) / (sizeof(float) * 2);
        mNumNorms = (mHdr.offset_pgons - mHdr.offset_norms) / (sizeof(float) * 3);

        // Read the materials
        readMaterials();

        // read UV
        readUVs();

        // Read the VHots if present
        readVHot();

        // Read the vertices
        readVertices();

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
        mMesh->_setBounds(AxisAlignedBox(mHdr.bbox_min[0], mHdr.bbox_min[1], mHdr.bbox_min[2],
                mHdr.bbox_max[0],mHdr.bbox_max[1],mHdr.bbox_max[2]));

        // Spherical radius
        mMesh->_setBoundingSphereRadius(mHdr.sphere_rad);

        mMesh->load();

        // DONE!
        mMesh->_updateCompiledBoneAssignments();
    }


    //---------------------------------------------------------------
    void ObjectMeshLoader::readObjects() {
        mFile->seek(mHdr.offset_objs);

        // create the skeleton for further usage
        // name it the same as the mesh
        mSkeleton = Ogre::SkeletonManager::getSingleton().create(mMesh->getName() + String("-Skeleton"), mMesh->getGroup());

        mMesh->_notifySkeleton(mSkeleton);

        // Seek at the beginning of the object
        mSubObjects = new SubObjectHeader[mHdr.num_objs];

        // Load all the subobj headers
        for (int n = 0; n < mHdr.num_objs; ++n ) {
            mFile->read(mSubObjects[n].name, 8);
            mFile->read(&mSubObjects[n].movement, 1);

            // the transform stuff
            mFile->readElem(&mSubObjects[n].trans.parent, 4);
            mFile->readElem(&mSubObjects[n].trans.min_range, 4);
            mFile->readElem(&mSubObjects[n].trans.max_range, 4);

            mFile->readElem(mSubObjects[n].trans.rot, 4, 9);

            readVertex(mSubObjects[n].trans.AxlePoint);

            // the rest of the struct
            mFile->readElem(&mSubObjects[n].child_sub_obj, 2, 12); // Todo: short int == 2 bytes?
        };

        /* Some notes:
        All begins with the root sub-object. Seems to always be 0
        All subobjs have two params. Child, Next. So what happens?
        ...
        Recursion!

        Concept is that every sub-object is a child of some other, or is a root (Only one of that type is per object, and has index 0).

        1) If child is >=0, we have a child object index. Recurse
        2) If next object is >=0, we have a colegue. Continue on the same level
        */

        // The subobject table is loaded. Now load all the subobj's (Recurses. This calls the root)
        loadSubObject(0, -1);

        mSkeleton->_notifyManualBonesDirty();
        mSkeleton->setBindingPose();
    }


    //-------------------------------------------------------------------
    void ObjectMeshLoader::loadSubObject(int obj, int parent) {
        // Bone has to be prepared. It can either be root (parent is -1) or attached (parent>=0)
        int subobj = obj;

        while (subobj >= 0) {
            if (parent >= 0) {
                // Set the bone position and transformation
                const Vertex& pos = mSubObjects[subobj].trans.AxlePoint;
                Vector3 v(pos.x, pos.y, pos.z);

                Quaternion q;

                q.FromRotationMatrix(convertRotation(mSubObjects[subobj].trans.rot));

                // We have a bone that should connect to a parent bone
                Bone* bone = mSkeleton->getBone(parent)->createChild(subobj);

                bone->setPosition(v);
                bone->setOrientation(q);
                bone->needUpdate();

                bone->setManuallyControlled(true);
            } else {
                // We have a root bone!
                Bone* bone = mSkeleton->createBone(subobj); // root bone it is!

                bone->resetOrientation();

                const Vertex& pos = mSubObjects[subobj].trans.AxlePoint;

                if (subobj != 0) { // Only if the root bone is not the subobject 0 (transform seems bogus on that one)
                    Quaternion q;
                    bone->setPosition(Vector3(pos.x, pos.y, pos.z));
                    q.FromRotationMatrix(convertRotation(mSubObjects[subobj].trans.rot));
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

            //next child of the parent to come?
            subobj = mSubObjects[subobj].next_sub_obj;
        }

    }

    //-------------------------------------------------------------------
    void ObjectMeshLoader::loadSubNode(int obj, size_t offset) {
        mFile->seek(mHdr.offset_nodes + offset);

        // read the type
        uint8_t type;
        mFile->read(&type, 1);

        NodeHeader ndhdr;
        NodeRaw     nr;
        NodeCall    nc;
        NodeSplit   ns;

        switch (type) {
            case MD_NODE_HDR:
                // in.read((char *) &ndhdr, sizeof(NodeHeader));
                mFile->read(&ndhdr.subObjectID, 1);
                mFile->read(&ndhdr.object_number, 1);
                mFile->read(&ndhdr.c_unk1, 1);

				if (obj == ndhdr.subObjectID)
					loadSubNode(obj, offset + NODE_HEADER_SIZE); // only skip this node's size
					
                break;

            case MD_NODE_SPLIT:
                readVertex(ns.sphere_center);
                mFile->readElem(&ns.sphere_radius, 4);
                mFile->readElem(&ns.pgon_before_count, 2);
                mFile->readElem(&ns.normal, 2);
                mFile->readElem(&ns.d, 4);
                mFile->readElem(&ns.behind_node, 2);
                mFile->readElem(&ns.front_node, 2);
                mFile->readElem(&ns.pgon_after_count, 2);

                loadPolygons(obj, ns.pgon_before_count + ns.pgon_after_count);


                loadSubNode (obj, ns.behind_node);
                loadSubNode (obj, ns.front_node);


                break;

            case MD_NODE_CALL:
                readVertex(nc.sphere_center);
                mFile->readElem(&nc.sphere_radius, 4);
                mFile->readElem(&nc.pgon_before_count, 2);
                mFile->readElem(&nc.call_node, 2);
                mFile->readElem(&nc.pgon_after_count, 2);

                // TODO: This seems not to work. Seeks past end of file and all that jazz...
                // It sure seems that the call in the title is just a name. If something is missing, do a revision of this
                // loadSubNode (obj, nc.call_node);

                loadPolygons(obj, nc.pgon_before_count + nc.pgon_after_count);


                break;

            case MD_NODE_RAW:
                readVertex(nr.sphere_center);
                mFile->readElem(&nr.sphere_radius, 4);
                mFile->readElem(&nr.pgon_count, 2);

                loadPolygons(obj, nr.pgon_count);

                break;

            default:
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Unknown node type " + StringConverter::toString(type)
                        + " at offset " + StringConverter::toString(offset), "ObjectMeshLoader::loadSubNode");
        };
    }

    //-------------------------------------------------------------------
    void ObjectMeshLoader::loadPolygons(int obj, size_t count) {
        // Step one - Read the polygon index list from the current position
        uint16_t* polys = new uint16_t[count];

        mFile->readElem(polys, 2, count); // These are offsets from the polygon offset in header, not plain index list

        // TODO: Step two - insert the polygons into the subobjects (load a poly and insert each step)
        for (size_t n = 0; n < count; n++) {
            // Load one polygon...
            mFile->seek(mHdr.offset_pgons + polys[n]);

            // Polygon header.
            ObjPolygon op;

            mFile->readElem(&op.index, 2);
            mFile->readElem(&op.data, 2);
            mFile->read(&op.type, 1);
            mFile->read(&op.num_verts, 1);
            mFile->readElem(&op.norm, 2);
            mFile->readElem(&op.d, 4);

            SubMeshFiller* f = getFillerForPolygon(op); // Won't give us a filler that does not need UV's when we have those or oposite

            // Load the indices for all 3 types, as needed
            uint16_t* verts = new uint16_t[op.num_verts];
            mFile->readElem(verts, 2, op.num_verts);

            // TODO: These are lights, not normals, and it will probably be bad to do it like this
            uint16_t* norms = new uint16_t[op.num_verts];
            mFile->readElem(norms, 2, op.num_verts);

            uint16_t* uvs = NULL;

            if (f->needsUV()) {
                uvs = new uint16_t[op.num_verts];
                mFile->readElem(uvs, 2, op.num_verts);
            }

            f->addPolygon(obj, op.num_verts, op.norm, verts, norms, uvs);

            delete[] verts;
            delete[] norms;
            delete[] uvs;
        }

        delete[] polys;
    }

    //-------------------------------------------------------------------
    void ObjectMeshLoader::readBinHeader() {
        mFile->read(mHdr.ObjName, 8);
        mFile->readElem(&mHdr.sphere_rad, 4);
        mFile->readElem(&mHdr.max_poly_rad, 4);

        mFile->readElem(mHdr.bbox_max, 4, 3); // vector
        mFile->readElem(mHdr.bbox_min, 4, 3); // vector
        mFile->readElem(mHdr.parent_cen, 4, 3); // vector

        mFile->readElem(&mHdr.num_pgons, 2);
        mFile->readElem(&mHdr.num_verts, 2);
        mFile->readElem(&mHdr.num_parms, 2);

        mFile->read(&mHdr.num_mats, 1);
        mFile->read(&mHdr.num_vcalls, 1);
        mFile->read(&mHdr.num_vhots, 1);
        mFile->read(&mHdr.num_objs, 1);

        mFile->readElem(&mHdr.offset_objs, 4);
        mFile->readElem(&mHdr.offset_mats, 4);
        mFile->readElem(&mHdr.offset_uv, 4);
        mFile->readElem(&mHdr.offset_vhots, 4);
        mFile->readElem(&mHdr.offset_verts, 4);
        mFile->readElem(&mHdr.offset_light, 4);
        mFile->readElem(&mHdr.offset_norms, 4);
        mFile->readElem(&mHdr.offset_pgons, 4);
        mFile->readElem(&mHdr.offset_nodes, 4);
        mFile->readElem(&mHdr.model_size, 4);

        // version 4 addons
        if (mVersion == 4) {
            mFile->readElem(&mHdr.mat_flags, 4);
            mFile->readElem(&mHdr.offset_new1, 4);
            mFile->readElem(&mHdr.offset_new2, 4);
        } else { // no mat flags, init to some non-colliding values
            mHdr.mat_flags = 0;
            mHdr.offset_new1 = 0;
            mHdr.offset_new2 = 0;
        }
    }

    //-------------------------------------------------------------------
    void ObjectMeshLoader::readMaterials() {
        mFile->seek(mHdr.offset_mats);

        // Allocate the materials
        mMaterials = new MeshMaterial[mHdr.num_mats];

        int n;

        // for each of the materials, load the struct
        for (n = 0; n < mHdr.num_mats; n++) {
            // load a single material
            mFile->read(mMaterials[n].name, 16);
            mFile->read(&mMaterials[n].type, 1);
            mFile->read(&mMaterials[n].slot_num, 1);

            // Material type variable part. 8 bytes total
            if (mMaterials[n].type == MD_MAT_COLOR) {
                mFile->read(mMaterials[n].colour, 4);
                mFile->readElem(&mMaterials[n].ipal_index, 4);
            } else if (mMaterials[n].type == MD_MAT_TMAP) {
                mFile->readElem(&mMaterials[n].handle, 4);
                mFile->readElem(&mMaterials[n].uvscale, 4);
            } else
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,"Unknown Material type : " + (int)(mMaterials[n].type), "ManualBinFileLoader::readBinHeader");
        }

        // construct anyway
        mMaterialsExtra = new MeshMaterialExtra[mHdr.num_mats];

        // if we need extended material attributes (this is fishy)

        for (int n = 0; n < mHdr.num_mats; n++) {
            if ( mHdr.mat_flags & MD_MAT_TRANS || mHdr.mat_flags & MD_MAT_ILLUM ) {
                mFile->readElem(&mMaterialsExtra[n].illum, 4);
                mFile->readElem(&mMaterialsExtra[n].trans, 4);
            } else {
                mMaterialsExtra[n].illum = 0;
                mMaterialsExtra[n].trans = 0;
            }

        }

        // Prepare the material slot mapping, if used
        // This means, slot index will point to material index
        for (int n = 0; n < mHdr.num_mats; n++) {
            //if (mVersion == 3) {
                mSlotToMatNum.insert(make_pair(mMaterials[n].slot_num, n));
            //} else
            //    mSlotToMatNum.insert(make_pair(n,n)); // plain mapping used
        }

        // Now prepare the raw material index to Ogre Material reference mapping to be used through the conversion
        for (int n = 0; n < mHdr.num_mats; n++) {
            MaterialPtr mat = prepareMaterial(mMesh->getName() + String("/") + String(mMaterials[n].name), mMaterials[n], mMaterialsExtra[n]);
            mOgreMaterials.insert(make_pair(n,mat));
        }
    }

    //-------------------------------------------------------------------
    void ObjectMeshLoader::readVHot() {
        // seek and load all the VHot, if present
        if (mHdr.num_vhots > 0) {
            mVHots = new VHotObj[mHdr.num_vhots];

            mFile->seek(mHdr.offset_vhots);

            for (int n = 0; n < mHdr.num_vhots; n++) {
                mFile->readElem(&mVHots[n].index, 4);
                readVertex(mVHots[n].point);
            }
        }
    }

    //-------------------------------------------------------------------
    void ObjectMeshLoader::readUVs() {
        // read
        if (mNumUVs > 0) { // can be zero if all the model is color only (No TXT)
            mUVs = new UVMap[mNumUVs];

            // seek to the offset
            mFile->seek(mHdr.offset_uv);

            for (int n = 0; n < mNumUVs; n++) {
                mFile->readElem(&mUVs[n].u, 4);
                mFile->readElem(&mUVs[n].v, 4);
            }
        }
    }

    //-------------------------------------------------------------------
    void ObjectMeshLoader::readVertices() {
        if (mHdr.num_verts > 0) {
            mVertices = new Vertex[mHdr.num_verts];

            mFile->seek(mHdr.offset_verts);

            for (int n = 0; n < mHdr.num_verts; n++)
                readVertex(mVertices[n]);

        } else
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Number of vertices is zero!", "ObjectMeshLoader::readVertices");
    }

    //-------------------------------------------------------------------
    void ObjectMeshLoader::readNormals() {
        if (mNumNorms > 0) {
            mNormals = new Vertex[mNumNorms];

            mFile->seek(mHdr.offset_norms);

            for (int n = 0; n < mNumNorms; n++)
                readVertex(mNormals[n]);

        } else
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,"Number of normals is zero!", "ObjectMeshLoader::readVertices");
    }

    //-------------------------------------------------------------------
    Matrix3 ObjectMeshLoader::convertRotation(const float m[9]) {
        // Convert to the Ogre::Matrix3
        Matrix3 mat(
            m[0], m[3], m[6],
            m[1], m[4], m[7],
            m[2], m[5], m[8]
        );

        return mat;
    }


    //-------------------------------------------------------------------
    void ObjectMeshLoader::readVertex(Vertex& vtx) {
        mFile->readElem(&vtx.x, 4);
        mFile->readElem(&vtx.y, 4);
        mFile->readElem(&vtx.z, 4);
    }

    //-------------------------------------------------------------------
    SubMeshFiller* ObjectMeshLoader::getFillerForPolygon(ObjPolygon& ply) {
        int type = ply.type & 0x07;
        int color_mode = ply.type & 0x60; // used only for MD_PGON_SOLID or MD_PGON_WIRE (WIRE is just a guess)

        FillerMap::iterator it = mFillers.end();
        String matName = "";
        int fillerIdx = -1;
        bool use_uvs = true;
        // TODO: MD_PGON_WIRE

        // Depending on the type, find the right filler, or construct one if not yet constructed
        if (type == MD_PGON_TMAP) {
            // Get the material index from the index (can be slot idx...)
            int matidx = getMaterialIndex(ply.data);

            // Color mode is ignored. We search the filler table simply by the material index
            it = mFillers.find(matidx);
            fillerIdx = matidx;
            matName = mOgreMaterials[matidx]->getName();
            // mMesh->getName() + "/" + mMaterials[matidx].name;


        } else if (type == MD_PGON_SOLID) {
            use_uvs = false;

            // Solid color polygon. This means we need to see if we use Material or Color table index
            if (color_mode == MD_PGON_SOLID_COLOR_PAL) {
                // Dynamically created material. We allocate negative numbers for these fillers
                // Color mode is ignored. We search the filler table simply by the material index

                matName = mMesh->getName() + "/Color" + StringConverter::toString(ply.index);

                // TODO: See if the material already existed or not...
                // Generate the solid material...
                MaterialPtr material = createPalMaterial(matName, ply.index);

                it = mFillers.find(-ply.index);
                fillerIdx = -ply.index;

                mOgreMaterials.insert(make_pair(-ply.index,material));

            } else if (color_mode == MD_PGON_SOLID_COLOR_VCOLOR) {

                int matidx = getMaterialIndex(ply.data);

                it = mFillers.find(matidx);
                fillerIdx = matidx;
                matName = mMesh->getName() + "/" + mMaterials[matidx].name;
            } else
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Unrecognized color_mode for polygon", "ObjectMeshLoader::getFillerForPolygon");
        } else
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("Unknown or invalid polygon type: ") + StringConverter::toString(ply.type) +
                " (" + StringConverter::toString(type) + " - " + StringConverter::toString(color_mode) + ")", "ObjectMeshLoader::getFillerForPolygon");


        if (it == mFillers.end()) {
            SubMesh* sm = mMesh->createSubMesh("SubMesh" + StringConverter::toString(fillerIdx));

            SubMeshFiller* f = new SubMeshFiller(sm, mHdr.num_verts, mVertices, mNumNorms, mNormals, mNumUVs, mUVs, use_uvs);

            // Set the material for the submesh
            f->setMaterialName(matName);


            mFillers.insert(make_pair(fillerIdx, f));


            return f;
        } else
            return it->second;
    }

    //-------------------------------------------------------------------
    int ObjectMeshLoader::getMaterialIndex(int slotidx) {
        std::map<int, int>::iterator it = mSlotToMatNum.find(slotidx);

        if (it != mSlotToMatNum.end()) {
            return it->second;
        } else
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,String("Unknown material slot index : ") + StringConverter::toString(slotidx), "ObjectMeshLoader::getMaterialIndex");
    }

    //-------------------------------------------------------------------
    MaterialPtr ObjectMeshLoader::prepareMaterial(String matname, MeshMaterial& mat, MeshMaterialExtra& matext) {
        // Look if the material is already defined or not (This enables anyone to replace the material without modifying anything)
        if (MaterialManager::getSingleton().resourceExists(matname)) {
            MaterialPtr fmat = MaterialManager::getSingleton().getByName(matname);
            return fmat;
        }

        // We'll create a material given the mat and matext structures
        MaterialPtr omat = MaterialManager::getSingleton().create(matname, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

        // fill it with the values given
        Pass *pass = omat->getTechnique(0)->getPass(0);

        if (mat.type == MD_MAT_TMAP) {

            // Texture unit state for the main texture...
            TextureUnitState* tus;

			// TODO: This ugly code should be in some special service. Name it ToolsService (getObjectTextureFileName, etc) and should handle language setting!
            String txtname = String("txt16/") + String(mat.name);
            // First, let's look into txt16 dir, then into the txt dir. I try to
             try {

                    TextureManager::getSingleton().load(txtname,
                                                ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D);
             } catch (Exception &e) {
                    // Error loading from txt16...
                    txtname = String("txt/") + String(mat.name);
                }

			// TODO: This is bad. Should be fixed. Looks awful. See ss2 model loading...
			pass->setAlphaRejectSettings(CMPF_EQUAL, 255); // Alpha rejection. 
			
			// Some basic lightning settings
            tus = pass->createTextureUnitState(txtname);

            tus->setTextureAddressingMode(TextureUnitState::TAM_WRAP);
            tus->setTextureCoordSet(0);
            tus->setTextureFiltering(TFO_TRILINEAR);
            tus->setColourOperation(LBO_REPLACE);


            // If the transparency is used
            if (( mHdr.mat_flags & MD_MAT_TRANS) && (matext.trans > 0)) {
                // set at least the transparency value for the material
                tus->setColourOperation(LBO_ALPHA_BLEND);
                tus->setAlphaOperation(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, 1 - matext.trans);
                pass->setDepthWriteEnabled(false);
            } else {
                // Set replace on all first layer textures for now
                tus->setColourOperation(LBO_REPLACE);
            }
            
            // Illumination of the material. Converted to ambient lightning here
            if (( mHdr.mat_flags & MD_MAT_ILLUM) && (matext.illum > 0)) {
                // set the illumination (converted to ambient lightning)
                pass->setAmbient(matext.illum, matext.illum, matext.illum);
            }

            omat->setCullingMode(CULL_ANTICLOCKWISE);
            
            omat->setLightingEnabled(true);
            
            
        } else if (mat.type == MD_MAT_COLOR) {
            // Fill in a color-only material
            TextureUnitState* tus = pass->createTextureUnitState();

			ColourValue matcolor(mat.colour[2] / 255.0, mat.colour[1] / 255.0, mat.colour[0] / 255.0);
			
            // set the material color
            tus->setColourOperationEx(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, matcolor);

            if (( mHdr.mat_flags & MD_MAT_TRANS ) && (matext.trans > 0)) {
                // tus->setColourOperation(LBO_ALPHA_BLEND);
                tus->setAlphaOperation(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, 1 - matext.trans);
            }
            
            // Illumination of a color based material. Hmm. Dunno if this is right, but i simply multiply the color with the illumination
            if (( mHdr.mat_flags & MD_MAT_ILLUM) && (matext.illum > 0)) {
                // set the illumination (converted to diffuse lightning)
                pass->setAmbient(matcolor.r * matext.illum, matcolor.g * matext.illum, matcolor.b * matext.illum);
            }
            
            omat->setCullingMode(CULL_ANTICLOCKWISE);
			omat->setLightingEnabled(true);
        } else
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("Invalid material type : ") + StringConverter::toString(mat.type), "ObjectMeshLoader::prepareMaterial");

		omat->load();

		return omat;
    }

    //-------------------------------------------------------------------
    MaterialPtr ObjectMeshLoader::createPalMaterial(String& matname, int palindex) {
        if (MaterialManager::getSingleton().resourceExists(matname)) {
            MaterialPtr fmat = MaterialManager::getSingleton().getByName(matname);
            return fmat;
        }

        assert(palindex >= 0 && palindex <= 255);

        // We'll create a material given the mat and matext structures
        MaterialPtr omat = MaterialManager::getSingleton().create(matname, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

        // fill it with the values given
        Pass *pass = omat->getTechnique(0)->getPass(0);

        // Fill in a color-only material
        TextureUnitState* tus = pass->createTextureUnitState();

        // set the material color from the lg system color table
        tus->setColourOperationEx(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT,
                ColourValue(
                    lg_system_colors[palindex][0],
                    lg_system_colors[palindex][1],
                    lg_system_colors[palindex][2]
                    )
            );

        // tus->setColourOperation(LBO_REPLACE);

        omat->load();

        return omat;
    }

    /*-----------------------------------------------------------------*/
	/*--------------------- ManualBinFileLoader -----------------------*/
	/*-----------------------------------------------------------------*/
    ManualBinFileLoader::ManualBinFileLoader() : ManualResourceLoader() {
    }

    //-------------------------------------------------------------------
    ManualBinFileLoader::~ManualBinFileLoader() {
    }

    //-------------------------------------------------------------------
    void ManualBinFileLoader::loadResource(Resource* resource) {
        // Cast to mesh, and fill
        Mesh* m = static_cast<Mesh*>(resource);

        // Fill. Find the file to be loaded by the name, and load it
        String name = m->getName();
        String group = m->getGroup();

        // Get the real filename from the nameValuePairList
        // That means: truncate to the last dot, append .bin to the filename
        size_t dot_pos = name.find_last_of(".");

        String basename = name;
        if (dot_pos != String::npos){
            basename = name.substr(0, dot_pos);
        }

        basename += ".bin";

        //Open the file, and detect the mesh type (Model/AI)
        Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource(basename, m->getGroup(), true, resource);

        FilePtr fin = new OgreFile(stream);

		// This here (the mf wrap) prevents the direct use of read on bad file offsets. Don't use normally. Just a fix for zzip segv
        MemoryFile* mf = new MemoryFile(basename, File::FILE_R);
        mf->initFromFile(*fin, fin->size()); // read the whole contents into the mem. file

        FilePtr f = mf; // wrap into mem. file



        char _hdr[5];
        _hdr[4] = 0;

        uint32_t version;

        // Look at the 4 byte header
        f->read(_hdr, 4); // read the LGMM/LGMD header
        f->read(&version, 4); // read the version int

        String header(_hdr);

        if (header == "LGMM") {
            // AI mesh. Not yet supported
        } else if (header == "LGMD") {
            // model. Supported for good. Load!
            ObjectMeshLoader ldr(m, f, version);
            try {
                ldr.load(); // that's all. Will do what's needed
            } catch (Opde::FileException &e) {
                LogManager::getSingleton().logMessage("An exception happened while loading the mesh " + basename + " : " + e.getDetails());
            }
        } else
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("Unknown BIN model format : '") + header + "'", "ManualBinFileLoader::loadResource");
    }
}


