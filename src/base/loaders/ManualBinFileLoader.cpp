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
 *	  $Id$
 *
 *****************************************************************************/

#include "config.h"

#include "ManualBinFileLoader.h"
#include "File.h"
#include "BinFormat.h"
#include "lgcolors.h"

#include "MaterialService.h"

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

	/** Class that loads a .CAL file and produces a ogre's skeleton instance
	* There are certain differences between .CAL and ogre's skeleton concept.
	* For example the whole skeleton in the CAL file is consisted of Torsos and bones.
	* A torso concept does not exist in ogre, but can be simulated by a set of bones from the torso's root to the attachment points.
	* Then there is the fact that dark adresses the bones as joints, and uses a number (not a string) as a unique identifier
	*
	* As ogre expects all the vertices in the transformed positions (something Dark does not do neither for LGMM nor LGMD), this class also
	* is a transformation source for the vertices based on the blended positions of joints.
	*/
	class CalSkeletonLoader {
		public:
			CalSkeletonLoader(const std::string& name, const std::string& group, Resource* resource) :
				mFileName(name),
				mGroup(group),
                mResource(resource),
				mFile(NULL),
				mTorsos(NULL),
                mLimbs(NULL),
				mSkeleton(NULL) {};

			~CalSkeletonLoader() {
				delete[] mTorsos;
				delete[] mLimbs;
			}

			/// does all the loading work, creates an Ogre::Skeleton
			void load();

			/// Retuns the resulting skeleton
			Ogre::SkeletonPtr& getSkeleton() { return mSkeleton; };

		protected:
			/// Reads the header from the .CAL file
			void readHeader(void);

			/// Reads the torsos from the .CAL file
			void readTorsos(void);

			/// Reads the limbs from the .CAL file
			void readLimbs(void);

			/// Converts Vertex to Ogre::Vector3
			static Vector3 toVector(const Vertex& v) { return Vector3(v.x, v.y, v.z); };

			/// the cal file name (incl. the .cal extension)
			String mFileName;

			/// the resource group used for the generated skeleton
			String mGroup;

			/// Resource we're loading the skeleton for
			Ogre::Resource *mResource;

			/// the file used for reading
			FilePtr mFile;

			/// header of the cal file
			CalHdr mHeader;

			/// the torso array prepared for processing
			CalTorso *mTorsos;

			/// the limb array prepared for processing
			CalLimb *mLimbs;

			/// our target skeleton instance
			SkeletonPtr mSkeleton;
	};

	void CalSkeletonLoader::load() {
		// quite simple, really
		Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource(mFileName, mGroup, true, mResource);

        mFile = new OgreFile(stream);

		// ok, got it open
		// read the header
		readHeader();

		if (mHeader.Version != 1)
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("Cal file has version other than 1 : ") + mFileName, "CalSkeletonLoader::load");

		// read the torsos
		readTorsos();

		// limbs now
		readLimbs();

		// ----------- Now, we build the result

		// Create a new skeleton to fill
		mSkeleton = Ogre::SkeletonManager::getSingleton().create(mFileName + "-skeleton", mGroup);

		// construct the Bones
		// Torsos first
		// the root torso we expect as first
		if (mTorsos[0].parent != -1)
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("Cal file expected to have torsos in order of creation : ") + mFileName, "CalSkeletonLoader::load");

		// the really ROOT bone
		// TODO: Map file for named joints (bones), as an optional addition
		// Bone* root = 
		mSkeleton->createBone(mTorsos[0].root);

		for (int i = 0; i < mHeader.num_torsos; ++i) {
			// get the torso's root bone to attach to:
			Bone* trbone = mSkeleton->getBone(mTorsos[i].root);

			for (int f = 0; f < mTorsos[i].fixed_count; ++f) {
				// here we go. one bone at time
				trbone->createChild(mTorsos[i].fixed_joints[f], toVector(mTorsos[i].fixed_joint_diff_coord[f]), Quaternion::IDENTITY);
			}
		}

		// Torsos are processed. Now limbs
		for (int i = 0; i < mHeader.num_limbs; ++i) {
			// get the attachment bone
			Bone* atb = mSkeleton->getBone(mLimbs[i].attachment_joint);

			for (int s = 0; s < mLimbs[i].num_segments; ++s) {
				Bone* nb = atb->createChild(mLimbs[i].segments[s], mLimbs[i].lengths[s] * toVector(mLimbs[i].segment_diff_coord[s]), Quaternion::IDENTITY);
				atb = nb; // now, the new bone becomes the parent for the next bone
			}
		}

		// Voila, All done. I said it's quite simple
	}

	void CalSkeletonLoader::readHeader(void) {
		mFile->readElem(&mHeader.Version, 4);
		mFile->readElem(&mHeader.num_torsos, 4);
		mFile->readElem(&mHeader.num_limbs, 4);
	}

	void CalSkeletonLoader::readTorsos(void) {
		// sanity checks
		if (mHeader.num_torsos < 1)
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("Cal file has zero torsos : ") + mFileName, "CalSkeletonLoader::readTorsos");

		// hard to imagine a body with 512 torsos... a cartepillar? :)
		if (mHeader.num_torsos > 512)
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("Cal file has more than 512 torsos : ") + mFileName, "CalSkeletonLoader::readTorsos");

		mTorsos = new CalTorso[mHeader.num_torsos];

		for (int32_t i = 0; i < mHeader.num_torsos; ++i) {
			mFile->readElem(&mTorsos[i].root, sizeof(uint32_t));
			mFile->readElem(&mTorsos[i].parent, sizeof(int32_t));
			mFile->readElem(&mTorsos[i].fixed_count, sizeof(int32_t));
			mFile->readElem(&mTorsos[i].fixed_joints, sizeof(uint32_t), 16);
			mFile->readElem(&mTorsos[i].fixed_joint_diff_coord, sizeof(float), 3 * 16); // 3 <-> x,y,z
		}
	}

	void CalSkeletonLoader::readLimbs(void) {
		if (mHeader.num_limbs > 512)
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("Cal file has more than 512 limbs : ") + mFileName, "CalSkeletonLoader::readLimbs");

		mLimbs = new CalLimb[mHeader.num_limbs];

		for (int32_t i = 0; i < mHeader.num_limbs; ++i) {
			mFile->readElem(&mLimbs[i].torso_index, sizeof(int32_t));
			mFile->readElem(&mLimbs[i].junk1, sizeof(int32_t));
			mFile->readElem(&mLimbs[i].num_segments, sizeof(int32_t));
			mFile->readElem(&mLimbs[i].attachment_joint, sizeof(uint16_t));
			mFile->readElem(&mLimbs[i].segments, sizeof(uint16_t), 16);
			mFile->readElem(&mLimbs[i].segment_diff_coord, sizeof(float), 3 * 16); // 3 <-> x,y,z
			mFile->readElem(&mLimbs[i].lengths, sizeof(float), 16);
		}
	}


    /** Helper SubMesh filler class. Receives a pointer to the UV and Vertex arrays, and is filled with triangles */
    class SubMeshFiller {
        public:
            SubMeshFiller(SubMesh* sm, size_t nvert, Vertex* vertices, size_t nnorms, Vertex* normals, size_t nlights, ObjLight* lights, size_t nuvs, UVMap* uvs, bool useuvmap) :
                    mVertices(vertices),
                    mNormals(normals),
                    mUVs(uvs),
                    mUseUV(useuvmap),
                    mBuilt(false),
                    mSubMesh(sm),
                    mSkeleton(NULL),
                    mLights(lights),
                    mNumVerts(nvert),
                    mNumNorms(nnorms),
                    mNumLights(nlights),
                    mNumUVs(nuvs) {
            };

            ~SubMeshFiller() {};

            void setMaterialName(const String& matname) { mMaterialName = matname; mSubMesh->setMaterialName(mMaterialName); };
            bool needsUV() {return mUseUV; };

            void addPolygon(int bone, size_t numverts, uint16_t normal, uint16_t* vidx, uint16_t* lightidx, uint16_t* uvidx);

            /// For AI meshes. All 3 coords index vert, norm and uv at once
            void addTriangle(uint16_t a, uint16_t bone_a, uint16_t b, uint16_t bone_b, uint16_t c, uint16_t bone_c);

            void setSkeleton(const SkeletonPtr& skel) { mSkeleton = skel; };

            void build();

            struct VertexDefinition {
                uint16_t vertex;
                uint16_t normal;
                uint16_t light;
                uint16_t uvidx; // Left zero if the UV's are not used
                int bone;
            };

        protected:
            uint16_t getIndex(int bone, uint16_t vert, uint16_t norm, uint16_t light, uint16_t uv);

			/// Unpacks the 30 bit normal from light struct (at index idx)
			Vector3 getUnpackedNormal(uint16_t idx);

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

			// Lights
			ObjLight* mLights;

            size_t mNumVerts, mNumNorms, mNumLights, mNumUVs;

            typedef std::vector< uint16_t > IndexList;
            IndexList mIndexList;

            typedef std::vector< VertexDefinition > VertexList;

            VertexList mVertexList;
    };

    bool operator==(const SubMeshFiller::VertexDefinition &a, const SubMeshFiller::VertexDefinition& b) {
        return ((a.vertex == b.vertex) && (a.normal == b.normal) && (a.uvidx == b.uvidx) && (a.bone == b.bone) && (a.light == b.light));
    }

    void SubMeshFiller::addPolygon(int bone, size_t numverts, uint16_t normal, uint16_t* vidx, uint16_t* lightidx, uint16_t* uvidx) {
        // For each of the vertices, search for the vertex/normal combination
        // If not found, insert one
        // As we triangulate the polygon, the order is slightly different from simple 0-(n-1)
        uint16_t last_index;
        uint16_t max_index;

        if (mUseUV) {
            last_index = getIndex(bone, vidx[0], normal, lightidx[0], uvidx[0]);
            max_index = getIndex(bone, vidx[numverts-1], normal, lightidx[numverts-1], uvidx[numverts-1]);
        } else {
            last_index = getIndex(bone, vidx[0], normal, lightidx[0], 0);
            max_index = getIndex(bone, vidx[numverts-1], normal, lightidx[numverts-1], 0);
        }

        for (size_t i = 1; i < (numverts-1); i++) {
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

    void SubMeshFiller::addTriangle(uint16_t a, uint16_t bone_a, uint16_t b, uint16_t bone_b, uint16_t c, uint16_t bone_c) {

        uint16_t idxa = getIndex(bone_a, a, a, 0, a);
        uint16_t idxb = getIndex(bone_b, b, b, 0, b);
        uint16_t idxc = getIndex(bone_c, c, c, 0, c);

        mIndexList.push_back(idxa);
        mIndexList.push_back(idxb);
        mIndexList.push_back(idxc);
    }

    uint16_t SubMeshFiller::getIndex(int bone, uint16_t vert, uint16_t norm, uint16_t light, uint16_t uv) {
        // Find the record with the same parameters
        // As I'm a bit lazy, I do this by iterating the whole vector
        // Check the limits!
        if (mNumVerts <= vert)
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Vertex Index out of range!", "SubMeshFiller::getIndex");
		if (!mLights) {
			if (mNumNorms <= norm)
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Normal Index out of range!", "SubMeshFiller::getIndex");
		} else {
			if (mNumLights <= light)
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Light Index out of range!", "SubMeshFiller::getIndex");
		}

        if (mNumUVs <= uv && mUseUV)
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "UV Index out of range!", "SubMeshFiller::getIndex");

        VertexList::const_iterator it = mVertexList.begin();
        uint16_t index = 0;

        VertexDefinition vdef;
        vdef.vertex = vert;

        if (!mLights) {
			vdef.normal = norm;
			vdef.light = 0;
        } else {
        	vdef.normal = 0;
			vdef.light = light;
        }

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

	Vector3 SubMeshFiller::getUnpackedNormal(uint16_t idx) {
		// get the ref to light struct at idx
		if (mNumLights <= idx)
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Light Index out of range!", "SubMeshFiller::getUnpackedNormal");

		Vector3 res;

		uint32_t src = mLights[idx].packed_normal;

		/* The Vector is organized as follows

		bit 32 -> bit 0

		XXXX XXXX | XXYY YYYY || YYYY ZZZZ | ZZZZ ZZ00

		each of those are fixed point signed numbers (10 bit)
		*/

		res.z = (int16_t)((int16_t)(src & 0x0FFFC) << 4) / 16384.0f;
		res.y = (int16_t)((src >> 6) & 0x0FFC0) / 16384.0f;
		res.x = (int16_t)((src >> 16) & 0x0FFC0) / 16384.0f;

		return res;
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
            Vector3 norm;

            if (mLights != NULL) {
            	norm = getUnpackedNormal(it->light);
            } else {
				norm = Vector3(mNormals[it->normal].x, mNormals[it->normal].y, mNormals[it->normal].z);
            }

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

        mSubMesh->vertexData->reorganiseBuffers(decl->getAutoOrganisedDeclaration(true, true));

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
            vba.weight = 1.0f; // TODO: Stretchy vertices will have this split to two parts

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

	/// Abstract parent for bin file loaders. Handles the common parts of model loading
	class DarkBINFileLoader {
		public:
			DarkBINFileLoader(Mesh* mesh, const Opde::FilePtr& file, unsigned int version) :
						mVersion(version),
						mMesh(mesh),
                        mFile(file) {
            };

		protected:
            unsigned int mVersion;
            Mesh* mMesh;
            Opde::FilePtr mFile;

			typedef std::map<int, MaterialPtr> OgreMaterials;
			typedef std::map< int, SubMeshFiller* > FillerMap;
	};

    /** Object Mesh loader class. identified by LGMD in the file offset 0. Accepts revision 3, 4 models.
    * This class fills the supplied mesh with a Ogre's version of the Mesh. */
    class ObjectMeshLoader : public DarkBINFileLoader {
        public:
            ObjectMeshLoader(Mesh* mesh, const Opde::FilePtr& file, unsigned int version) : DarkBINFileLoader(mesh, file, version),
                        mMaterials(NULL),
                        mMaterialsExtra(NULL),
                        mVHots(NULL),
                        mVertices(NULL),
                        mLights(NULL),
                        mUVs(NULL),
                        mSubObjects(NULL),
                        mNumUVs(0),
                        mNumNorms(0),
                        mNumLights(0) {

                if ((mVersion != 3) && (mVersion != 4))
                    OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Unsupported object mesh version : " + StringConverter::toString(mVersion),"ObjectMeshLoader::ObjectMeshLoader");

                mMaterialService = GET_SERVICE(MaterialService);
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
                delete[] mLights;

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
            void readLights();

            Matrix3 convertRotation(const float m[9]);

            SubMeshFiller* getFillerForPolygon(ObjPolygon& ply);

            int getMaterialIndex(int slotidx);
            /// Creates a material from the pallete index (solid color material)
            MaterialPtr createPalMaterial(String& matname, int palindex);

            void readVertex(Vertex& vtx);
            MaterialPtr prepareMaterial(String matname, MeshMaterial& mat, MeshMaterialExtra& matext);


            BinHeader mHdr;
            MeshMaterial* mMaterials;
            MeshMaterialExtra* mMaterialsExtra;
            VHotObj* mVHots;
            Vertex* mVertices;
            Vertex* mNormals;
			ObjLight* mLights;

            UVMap* mUVs;
            SubObjectHeader* mSubObjects;

			int mNumUVs;
            int mNumNorms;
            int mNumLights;


            std::map<int, int> mSlotToMatNum; // only v3 uses this. v4 is filled 1:1

            FillerMap mFillers;

            OgreMaterials mOgreMaterials;

            SkeletonPtr mSkeleton;

            MaterialServicePtr mMaterialService;
    };

	class AIMeshLoader: public DarkBINFileLoader {
		public:
			AIMeshLoader(Mesh* mesh, const Opde::FilePtr& file, unsigned int version);
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

			MaterialPtr prepareMaterial(String matname, AIMaterial& mat);

			void readVectors(Vertex* target, size_t count);

			SubMeshFiller* getFillerForSlot(int slot);

			/// LGMM header (offset 0x08 in file)
			AIMeshHeader mHeader;

			/// joint remappings (?)
			uint8_t *mJointsIn, *mJointsOut;

			/// source of material definitions
			AIMaterial *mMaterials;

			/// remaps .BIN joints to .CAL joints (in bin, i'd call those chunks in fact, not joints)
			AIMapper *mMappers;

			/// geometry chunks per mapper
			AIJointInfo *mJoints;

			/// vertices
			Vertex* mVertices;

			/// normals
            Vertex* mNormals;

            /// uv's (stored as Vertex in file, but we use the same submesh filler class)
            UVMap* mUVs;

            /// AI triangles
            AITriangle* mTriangles;

            /// materials per slot ID
            OgreMaterials mOgreMaterials;

            /// Fillers for polys
            FillerMap mFillers;

            /// Loader for the skeleton
            CalSkeletonLoader* mCalLoader;

            /// simple mapper for vertex->joint
            uint8_t *mVertexJointMap;
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
         mMesh->_setBounds(AxisAlignedBox(mHdr.bbox_min[0], mHdr.bbox_min[1], mHdr.bbox_min[2],
                mHdr.bbox_max[0],mHdr.bbox_max[1],mHdr.bbox_max[2]));*/

        // Spherical radius
        mMesh->_setBoundingSphereRadius(mHdr.sphere_rad);


        mMesh->prepareForShadowVolume();
        mMesh->buildEdgeList();

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
                mFile->readElem(&mMaterialsExtra[n].trans, 4);
                mFile->readElem(&mMaterialsExtra[n].illum, 4);
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
    void ObjectMeshLoader::readLights() {
        if (mNumLights > 0) {
            mLights = new ObjLight[mNumLights];

            mFile->seek(mHdr.offset_light);

			for (int n = 0; n < mNumLights; n++) {
				mFile->readElem(&mLights[n].material, 2);
				mFile->readElem(&mLights[n].point, 2);
				mFile->readElem(&mLights[n].packed_normal, 4);
			}

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

            SubMeshFiller* f = new SubMeshFiller(sm, mHdr.num_verts, mVertices, mNumNorms, mNormals, mNumLights, mLights, mNumUVs, mUVs, use_uvs);

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

        omat->setLightingEnabled(true);
		omat->setReceiveShadows(false);

        // fill it with the values given
        Pass *pass = omat->getTechnique(0)->getPass(0);

        // Defaults:
        // Ambient is one. It is controlled by mission ambient setting...
        pass->setAmbient(1,1,1);
        pass->setDiffuse(1,1,1,1);
        pass->setSpecular(0,0,0,0);
        pass->setCullingMode(CULL_CLOCKWISE);

        if (mat.type == MD_MAT_TMAP) {

            // Texture unit state for the main texture...
            TextureUnitState* tus;

			// TODO: This ugly code should be in some special service. Name it ToolsService (getObjectTextureFileName, etc) and should handle language setting!
			String txtname = String("txt16/") + String(mat.name);

			if (!ResourceGroupManager::getSingleton().resourceExists(ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, String("txt16/") + String(mat.name))) {
               // Not in txt16, will be in txt then...
               txtname = String("txt/") + String(mat.name);
            }

			pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
			pass->setAlphaRejectSettings(CMPF_GREATER, 128); // Alpha rejection.
			// default depth bias
			pass->setDepthBias(0.01, 0.01);

			// Some basic lightning settings
            tus = pass->createTextureUnitState(txtname);
            // The model textures are not animated like this. They probably use a texture swapping tweq or something
			// tus = mMaterialService->createAnimatedTextureState(pass, txtname, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 5);

            tus->setTextureAddressingMode(TextureUnitState::TAM_WRAP);
            tus->setTextureCoordSet(0);
            tus->setTextureFiltering(TFO_BILINEAR);

            // If the transparency is used
            if (( mHdr.mat_flags & MD_MAT_TRANS) && (matext.trans > 0)) {
            	// set at least the transparency value for the material
                pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
                pass->setDepthWriteEnabled(false);
                tus->setColourOperation(LBO_ALPHA_BLEND);
                pass->setAlphaRejectFunction(CMPF_ALWAYS_PASS); // Alpha rejection reset. Does not live good with the following:
                tus->setAlphaOperation(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, 1 - matext.trans);
            }

            // Illumination of the material. Converted to ambient lightning here
            if (( mHdr.mat_flags & MD_MAT_ILLUM) && (matext.illum > 0)) {
                // set the illumination
                pass->setSelfIllumination(matext.illum, matext.illum, matext.illum);
            }

        } else if (mat.type == MD_MAT_COLOR) {
            // Fill in a color-only material
            TextureUnitState* tus = pass->createTextureUnitState();

			ColourValue matcolor(mat.colour[2] / 255.0, mat.colour[1] / 255.0, mat.colour[0] / 255.0);

            // set the material color
            // tus->setColourOperationEx(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, matcolor);
            pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
            pass->setAmbient(matcolor);
			pass->setDiffuse(matcolor);

            if (( mHdr.mat_flags & MD_MAT_TRANS ) && (matext.trans > 0)) {
                tus->setColourOperation(LBO_ALPHA_BLEND);
                tus->setAlphaOperation(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, 1 - matext.trans);
            }

            // Illumination of a color based material. Hmm. Dunno if this is right, but i simply multiply the color with the illumination
            if (( mHdr.mat_flags & MD_MAT_ILLUM) && (matext.illum > 0)) {
                // set the illumination
                pass->setSelfIllumination(matcolor.r * matext.illum, matcolor.g * matext.illum, matcolor.b * matext.illum);
            }

            // omat->setCullingMode(CULL_ANTICLOCKWISE);

        } else
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("Invalid material type : ") + StringConverter::toString(mat.type), "ObjectMeshLoader::prepareMaterial");

        omat->setShadingMode(SO_GOURAUD);
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
	/*--------------------- AIMeshLoader        -----------------------*/
	/*-----------------------------------------------------------------*/
	AIMeshLoader::AIMeshLoader(Mesh* mesh, const Opde::FilePtr& file, unsigned int version) :
		DarkBINFileLoader(mesh, file, version),
		mJointsIn(NULL),
		mJointsOut(NULL),
		mMaterials(NULL),
		mMappers(NULL),
		mJoints(NULL),
		mVertices(NULL),
		mNormals(NULL),
		mUVs(NULL),
		mTriangles(NULL),
        mCalLoader(NULL),
		mVertexJointMap(NULL) {

		if (mVersion > 2 || mVersion < 1)
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("File has version outside (0,1) range, and thus cannot be handled ") + mMesh->getName(), "AIMeshLoader::AIMeshLoader");
	};

	AIMeshLoader::~AIMeshLoader() {
		delete[] mJointsIn;
		delete[] mJointsOut;
		delete[] mMaterials;
		delete[] mMappers;
		delete[] mJoints;
		delete[] mVertices;
		delete[] mNormals;
		delete[] mUVs;
		delete[] mTriangles;
		delete[] mVertexJointMap;
		delete mCalLoader;

        FillerMap::iterator it = mFillers.begin();

        for ( ; it != mFillers.end(); ++it) {
			delete it->second;
        }

        mFillers.clear();
	}

    void AIMeshLoader::load() {
    	// For the AI mesh to be usable, we need the cal file.
    	// The cal file has the same name as the BIN file, but .CAL extension
        String name = mMesh->getName();
        String group = mMesh->getGroup();

        // Get the real filename from the nameValuePairList
        // That means: truncate to the last dot, append .bin to the filename
        size_t dot_pos = name.find_last_of(".");

        String basename = name;
        if (dot_pos != String::npos) {
            basename = name.substr(0, dot_pos);
        }

        basename += ".cal";

        // load the skeleton
        mCalLoader = new CalSkeletonLoader(basename, group, mMesh);

		// if this fails, at least we'll be clean on the mesh side
		// because the exception won't have to be handled for cleaning
        mCalLoader->load();

        // load
        // 1. the header
        readHeader();

        // 2. the .BIN joint ID remapping struct (I suppose this swaps .BIN joint id's somehow)
        mFile->seek(mHeader.offset_joint_remap, File::FSEEK_BEG);

        mJointsIn = new uint8_t[mHeader.num_joints];
        mJointsOut = new uint8_t[mHeader.num_joints];

        mFile->read(mJointsIn, mHeader.num_joints); // no need for byteswaps here
        mFile->read(mJointsOut, mHeader.num_joints); // no need for byteswaps here

        // 3. the Joint remap info. BIN joint to .cal joint mapping, probably other data as well
        readMappers();

        // 4. the materials
        readMaterials();

        // 5. Joints - triangles per .BIN joint, vertices per .BIN joint (I suppose there can be some weigting done)
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
        mVertexJointMap = new uint8_t[mHeader.num_vertices];

        for (int16_t j = 0; j < mHeader.num_vertices; ++j)
			mVertexJointMap[j] = 0;


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
				AITriangle& tri = mTriangles[mJoints[j].start_poly + v];

				SubMeshFiller* f = getFillerForSlot(tri.mat); // maybe slots are not used after all

				if (!f)
					OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("Filler not found for slot!"), "AIMeshLoader::readMaterials");

				f->addTriangle(tri.a, mVertexJointMap[tri.a], tri.b, mVertexJointMap[tri.b], tri.c, mVertexJointMap[tri.c]);
			}
        }

        FillerMap::iterator it = mFillers.begin();

        for (; it != mFillers.end(); ++it) {
            it->second->build();
        }

        // DONE!
		mMesh->load();

        // DONE!
        mMesh->_updateCompiledBoneAssignments();
        // cleanup?
    }

    void AIMeshLoader::readHeader() {
    	mFile->readElem(mHeader.zeroes, sizeof(uint32), 3);

    	mFile->read(&mHeader.num_what1, 1);
    	mFile->read(&mHeader.num_mappers, 1);
    	mFile->read(&mHeader.num_mats, 1);
    	mFile->read(&mHeader.num_joints, 1);

    	mFile->readElem(&mHeader.num_polys, 2);
    	mFile->readElem(&mHeader.num_vertices, 2);

    	mFile->readElem(&mHeader.num_stretchy, 4);

    	// offsets...
    	mFile->readElem(&mHeader.offset_joint_remap, 4);
    	mFile->readElem(&mHeader.offset_mappers, 4);
    	mFile->readElem(&mHeader.offset_mats, 4);
    	mFile->readElem(&mHeader.offset_joints, 4);
    	mFile->readElem(&mHeader.offset_poly, 4);
    	mFile->readElem(&mHeader.offset_norm, 4);
    	mFile->readElem(&mHeader.offset_vert, 4);
    	mFile->readElem(&mHeader.offset_uvmap, 4);
    	mFile->readElem(&mHeader.offset_blends, 4);
    	mFile->readElem(&mHeader.offset_U9, 4);
    }

	void AIMeshLoader::readMaterials() {
		// repeat for all materials
		uint8_t i;

		if (mHeader.num_mats < 1) // TODO: This could be fatal
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("File contains no materials ")  + mMesh->getName(), "AIMeshLoader::readMaterials");

		mMaterials = new AIMaterial[mHeader.num_mats];

		mFile->seek(mHeader.offset_mats, File::FSEEK_BEG);

		for (i = 0 ; i < mHeader.num_mats; ++i) {
			mFile->read(mMaterials[i].name, 16);

			// version dep.
			if (mVersion > 1) {
				mFile->readElem(&mMaterials[i].ext_flags, 4);
				mFile->readElem(&mMaterials[i].trans, 4);
				mFile->readElem(&mMaterials[i].illum, 4);
				mFile->readElem(&mMaterials[i].unknown, 4);
			} else {
				mMaterials[i].ext_flags = 0;
				mMaterials[i].trans = 0.0f;
				mMaterials[i].illum = 0.0f;
				mMaterials[i].unknown = 0.0f;
			}

			// back to version independent loading
			mFile->readElem(&mMaterials[i].unk1, 4);
			mFile->readElem(&mMaterials[i].unk2, 4);

			mFile->read(&mMaterials[i].type, 1);
			mFile->read(&mMaterials[i].slot_num, 1);

			mFile->readElem(&mMaterials[i].s_unk1, 2);
			mFile->readElem(&mMaterials[i].s_unk2, 2);
			mFile->readElem(&mMaterials[i].s_unk3, 2);
			mFile->readElem(&mMaterials[i].s_unk4, 2);
			mFile->readElem(&mMaterials[i].s_unk5, 2);

			mFile->readElem(&mMaterials[i].l_unk3, 4);

			// prepare this as Ogre's material
			MaterialPtr mat = prepareMaterial(mMesh->getName() + String("/") + String(mMaterials[i].name), mMaterials[i]);

			// mOgreMaterials.insert(make_pair(mMaterials[i].slot_num, mat));
			mOgreMaterials.insert(make_pair(i, mat)); // mMaterials[i].slot_num
		}
	}

	void AIMeshLoader::readMappers() {
		mFile->seek(mHeader.offset_mappers, File::FSEEK_BEG);

		if (mHeader.num_mappers < 1)
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("File contains no mappers ") + mMesh->getName(), "AIMeshLoader::readMappers");

		mMappers = new AIMapper[mHeader.num_mappers];

		for (uint8_t i = 0 ; i < mHeader.num_mappers; ++i) {
			mFile->readElem(&mMappers[i].unk1, 4);

			mFile->read(&mMappers[i].joint, 1);
			mFile->read(&mMappers[i].en1, 1);
			mFile->read(&mMappers[i].jother, 1);
			mFile->read(&mMappers[i].en2, 1);

			mFile->readElem(&mMappers[i].rotation, 4, 3);
		}
	}

	void AIMeshLoader::readJoints() {
		mFile->seek(mHeader.offset_joints, File::FSEEK_BEG);

		if (mHeader.num_joints < 1)
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("File contains no joints ") + mMesh->getName(), "AIMeshLoader::readJoints");

		mJoints = new AIJointInfo[mHeader.num_joints];

		for (uint8_t i = 0 ; i < mHeader.num_joints; ++i) {
			mFile->readElem(&mJoints[i].num_polys, 2);
			mFile->readElem(&mJoints[i].start_poly, 2);
			mFile->readElem(&mJoints[i].num_vertices, 2);
			mFile->readElem(&mJoints[i].start_vertex, 2);

			mFile->readElem(&mJoints[i].jflt, 4);

			mFile->readElem(&mJoints[i].sh6, 2);
			mFile->readElem(&mJoints[i].mapper_id, 2);
		}
	}

	void AIMeshLoader::readTriangles() {
		mFile->seek(mHeader.offset_poly, File::FSEEK_BEG);

		if (mHeader.num_polys < 1)
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("File contains no polygons ") + mMesh->getName(), "AIMeshLoader::readVertices");

		mTriangles = new AITriangle[mHeader.num_polys];

		for (int16_t i = 0; i < mHeader.num_polys; ++i) {
			mFile->readElem(&mTriangles[i].a, 2);
			mFile->readElem(&mTriangles[i].b, 2);
			mFile->readElem(&mTriangles[i].c, 2);
			mFile->readElem(&mTriangles[i].mat, 2);

			mFile->readElem(&mTriangles[i].f_unk, 4);

			mFile->readElem(&mTriangles[i].index, 2);

			mFile->readElem(&mTriangles[i].flag, 2);
		}
	}

	void AIMeshLoader::readVertices() {
		mFile->seek(mHeader.offset_vert, File::FSEEK_BEG);

		if (mHeader.num_vertices < 1)
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("File contains no vertices ") + mMesh->getName(), "AIMeshLoader::readVertices");

		mVertices = new Vertex[mHeader.num_vertices];

		readVectors(mVertices, mHeader.num_vertices);
	}

	void AIMeshLoader::readNormals() {
		mFile->seek(mHeader.offset_norm, File::FSEEK_BEG);

		if (mHeader.num_vertices < 1) // TODO: This could be fatal
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("File contains no normals ") + mMesh->getName(), "AIMeshLoader::readNormals");

		mNormals = new Vertex[mHeader.num_vertices];

		readVectors(mNormals, mHeader.num_vertices);
	}

	void AIMeshLoader::readUVs() {
		mFile->seek(mHeader.offset_uvmap, File::FSEEK_BEG);

		if (mHeader.num_vertices < 1) // TODO: This could be fatal
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("File contains no uv's ") + mMesh->getName(), "AIMeshLoader::readNormals");

		mUVs = new UVMap[mHeader.num_vertices];

		float bogus_z;

		for (int16_t i = 0; i < mHeader.num_vertices; ++i) {
			mFile->readElem(&mUVs[i].u, 4);
			mFile->readElem(&mUVs[i].v, 4);
			mFile->readElem(&bogus_z, 4); // we simply ignore Z... till we find we can't ;)
		}
	}

	void AIMeshLoader::readVectors(Vertex* target, size_t count) {
		for (size_t i = 0; i < count; ++i) {
			mFile->readElem(&target[i].x, 4);
			mFile->readElem(&target[i].y, 4);
			mFile->readElem(&target[i].z, 4);
		}
	}

    MaterialPtr AIMeshLoader::prepareMaterial(String matname, AIMaterial& mat) {
        // Look if the material is already defined or not (This enables anyone to replace the material without modifying anything)
        if (MaterialManager::getSingleton().resourceExists(matname)) {
            MaterialPtr fmat = MaterialManager::getSingleton().getByName(matname);
            return fmat;
        }

        // We'll create a material given the mat and matext structures
        MaterialPtr omat = MaterialManager::getSingleton().create(matname, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

        omat->setLightingEnabled(true);
		omat->setReceiveShadows(false);

        // fill it with the values given
        Pass *pass = omat->getTechnique(0)->getPass(0);

        // Defaults:
        // Ambient is one. It is controlled by mission ambient setting...
        pass->setAmbient(1,1,1);
        pass->setDiffuse(1,1,1,1);
        pass->setSpecular(0,0,0,0);

        if (mat.type == MD_MAT_TMAP) {
            // Texture unit state for the main texture...
            TextureUnitState* tus;

			// TODO: This ugly code should be in some special service. Name it ToolsService (getObjectTextureFileName, etc) and should handle language setting!
            String txtname = String("txt16/") + String(mat.name);
            // First, let's look into txt16 dir, then into the txt dir. I try to
             try {

                    TextureManager::getSingleton().load(txtname,
                                                ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D);
             } catch (Exception) {
                    // Error loading from txt16...
                    txtname = String("txt/") + String(mat.name);
                }

			pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
			pass->setAlphaRejectSettings(CMPF_GREATER, 128); // Alpha rejection.

			// Some basic lightning settings
            tus = pass->createTextureUnitState(txtname);

            tus->setTextureAddressingMode(TextureUnitState::TAM_WRAP);
            tus->setTextureCoordSet(0);
            tus->setTextureFiltering(TFO_BILINEAR);

            // If the transparency is used
            if (( mat.ext_flags & MD_MAT_TRANS) && (mat.trans > 0)) {
                // set at least the transparency value for the material
                pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
                pass->setDepthWriteEnabled(false);
                tus->setColourOperation(LBO_ALPHA_BLEND);
                pass->setAlphaRejectFunction(CMPF_ALWAYS_PASS); // Alpha rejection reset. Does not live good with the following:
                tus->setAlphaOperation(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, 1 - mat.trans);
            }

            // Illumination of the material. Converted to ambient lightning here
            if (( mat.ext_flags & MD_MAT_ILLUM) && (mat.illum > 0)) {
                // set the illumination
                pass->setSelfIllumination(mat.illum, mat.illum, mat.illum);
            }

            // omat->setCullingMode(CULL_ANTICLOCKWISE);

        } else if (mat.type == MD_MAT_COLOR) {
            // Fill in a color-only material
            TextureUnitState* tus = pass->createTextureUnitState();

			// TODO: Code. We don't know where the color is, yet!
			ColourValue matcolor(1, 1, 1);

            // set the material color
            // tus->setColourOperationEx(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, matcolor);
			pass->setDiffuse(matcolor);

            if (( mat.ext_flags & MD_MAT_TRANS ) && (mat.trans > 0)) {
                // tus->setColourOperation(LBO_ALPHA_BLEND);
                tus->setAlphaOperation(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, 1 - mat.trans);
            }

            // Illumination of a color based material. Hmm. Dunno if this is right, but i simply multiply the color with the illumination
            if (( mat.ext_flags & MD_MAT_ILLUM) && (mat.illum > 0)) {
                // set the illumination
                pass->setSelfIllumination(matcolor.r * mat.illum, matcolor.g * mat.illum, matcolor.b * mat.illum);
            }

        } else
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("Invalid material type : ") + StringConverter::toString(mat.type), "AIMeshLoader::prepareMaterial");

        omat->setShadingMode(SO_GOURAUD);
		omat->load();

		return omat;
    }

    SubMeshFiller* AIMeshLoader::getFillerForSlot(int slot) {
    	FillerMap::iterator it = mFillers.find(slot);

    	if (it != mFillers.end()) {
			return it->second;
    	} else {
    		// find the material name
			OgreMaterials::iterator mit = mOgreMaterials.find(slot);

			if (mit != mOgreMaterials.end()) {
				SubMesh* sm = mMesh->createSubMesh("SubMesh" + StringConverter::toString(slot));

				SubMeshFiller* f = new SubMeshFiller(sm, mHeader.num_vertices, mVertices, mHeader.num_vertices, mNormals, 0, NULL, mHeader.num_vertices, mUVs, true);

				// Set the material for the submesh
				f->setMaterialName(mit->second->getName());

				f->setSkeleton(mCalLoader->getSkeleton());

				mFillers.insert(make_pair(slot, f));

				return f;
			} else {
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("Slot is not occupied by material ") + StringConverter::toString(slot) +  " file " + mMesh->getName(),
						"AIMeshLoader::prepareMaterial");
			}
    	}
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
            // AI mesh
            AIMeshLoader ldr(m, f, version);

            try {
            	ldr.load();
            } catch (Opde::FileException &e) {
                LogManager::getSingleton().logMessage("An exception happened while loading the mesh " + basename + " : " + e.getDetails());
            }
        } else if (header == "LGMD") {
            // Object model mesh
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


