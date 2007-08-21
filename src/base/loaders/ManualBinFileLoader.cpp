/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2007 openDarkEngine team
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

#include "ManualBinFileLoader.h"
#include "File.h"
#include "BinFormat.h"
#include "lgcolors.h"
#include <OgreStringConverter.h>
#include <OgreMaterial.h>
#include <OgreMaterialManager.h>
#include <OgreTechnique.h>

using namespace std;
using namespace Opde; // For the Opde::File

namespace Ogre {

    /** Helper SubMesh filler class. Receives a pointer to the UV and Vertex arrays, and is filled with triangles */
    class SubMeshFiller {
        public:
            SubMeshFiller(Vertex* vertices, UVMap* uvs, bool useuvmap) : mVertices(vertices), mUVs(uvs), mUseUV(useuvmap), mBuilt(false) {};
            ~SubMeshFiller() {};

            void setMaterialName(String& matname) { mMaterialName = matname; };
            bool needsUV() {return mUseUV; };

            void addPolygon(size_t numverts, uint16_t* vidx, uint16_t* normidx, uint16_t* uvidx = NULL);

            void build();

        protected:
            /// Global vertex data pointer
            Vertex* mVertices;
            /// Global UV table pointer
            UVMap* mUVs;
            /// used Material name
            String mMaterialName;
            /// Uses UVs
            bool mUseUV;
            /// Already built
            bool mBuilt;
    };


    void SubMeshFiller::addPolygon(size_t numverts, uint16_t* vidx, uint16_t* normidx, uint16_t* uvidx) {

    }

    void SubMeshFiller::build() {


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

                FillerMap::iterator it = mFillers.begin();

                for (; it != mFillers.end(); ++it) {
                    delete it->second;
                    mFillers.erase(it);
                }
            };

            void load();

        protected:
            void readObjects();
            void loadSubObject(int obj);
            void loadSubNode(int obj, size_t offset);
            void loadPolygons(int obj, size_t count);
            void readBinHeader();
            void readMaterials();
            void readVHot();
            void readUVs();
            void readVertices();

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
            UVMap* mUVs;
            SubObjectHeader* mSubObjects;

            int mNumUVs;


            std::map<int, int> mSlotToMatNum; // only v3 uses this. v4 is filled 1:1
            typedef std::map< int, SubMeshFiller* > FillerMap;

            FillerMap mFillers;

            typedef std::map<int, MaterialPtr> OgreMaterials;

            OgreMaterials mOgreMaterials;
    };

    //---------------------------------------------------------------
    void ObjectMeshLoader::load() {
        // read all the fields of the BIN header...
        readBinHeader();

        // progress with loading. Calculate the count of the UV records
        mNumUVs = (mHdr.offset_vhots - mHdr.offset_uv) / sizeof (UVMap);

        // some material should be present
        assert(mHdr.num_mats != 0);

        // Read the materials
        readMaterials();

        // read UV
        readUVs();

        // Read the VHots if present
        readVHot();

        // Read the vertices
        readVertices();

        // Everything is ready. Can proceed with the filling
        readObjects();

        // Final step. Build the submeshes
        FillerMap::iterator it = mFillers.begin();

        for (; it != mFillers.end(); ++it) {
            it->second->build();
        }

        // DONE!
    }


    //---------------------------------------------------------------
    void ObjectMeshLoader::readObjects() {
        // Seek at the beginning of the object
        mSubObjects = new SubObjectHeader[mHdr.num_objs];

        // Load all the subobj headers
        for (int n = 0; n < mHdr.num_objs; ++n ) {
            mFile->read(mSubObjects[n].name, 8);
            mFile->read(&mSubObjects[n].movement, 1);

            // the transform stuff
            mFile->readElem(&mSubObjects[n].trans.JointNumber, 4);
            mFile->readElem(&mSubObjects[n].trans.min_range, 4);
            mFile->readElem(&mSubObjects[n].trans.max_range, 4);
            mFile->readElem(&mSubObjects[n].trans.f, 4, 9);

            // the joint position
            readVertex(mSubObjects[n].trans.AxlePoint);

            // the rest of the struct
            mFile->readElem(&mSubObjects[n].child_sub_obj, 2, 12); // Todo: short int == 2 bytes?
        };

        // The subobject table is loaded. Now load all the subobj's
        for (int n = 0; n < mHdr.num_objs; ++n ) {
            loadSubObject(n);
        };
    }


    //-------------------------------------------------------------------
    void ObjectMeshLoader::loadSubObject(int obj) {
        // We expect a reference to a root Node of the subobj
        loadSubNode(obj, mSubObjects[obj].node_start);
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
                mFile->read(&ndhdr.flag, 1);
                mFile->read(&ndhdr.object_number, 1);
                mFile->read(&ndhdr.c_unk1, 1);

                // TODO: Door handles. One instance for two handles. Same node tree
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

                // TODO:
                loadSubNode (obj, nc.call_node);

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

            uint16_t* norms = new uint16_t[op.num_verts];
            mFile->readElem(verts, 2, op.num_verts);

            uint16_t* uvs = NULL;

            if (f->needsUV()) {
                uvs = new uint16_t[op.num_verts];
            }

            f->addPolygon(op.num_verts, verts, norms, uvs);

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
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,"Unknown Material type : " + mMaterials[n].type, "ManualBinFileLoader::readBinHeader");
        }

        // construct anyway
        mMaterialsExtra = new MeshMaterialExtra[mHdr.num_mats];

        // if we need extended material attributes
        if ( mHdr.mat_flags & MD_MAT_TRANS || mHdr.mat_flags & MD_MAT_ILLUM ) {

            for (int n = 0; n < mHdr.num_mats; n++) {
                mFile->readElem(&mMaterialsExtra[n].illum, 4);
                mFile->readElem(&mMaterialsExtra[n].trans, 4);
            }

        }

        // Prepare the material slot mapping, if used
        // This means, slot index will point to material index
        for (int n = 0; n < mHdr.num_mats; n++) {
            if (mVersion == 3) {
                mSlotToMatNum.insert(make_pair(mMaterials[n].slot_num,n));
            } else
                mSlotToMatNum.insert(make_pair(n,n)); // plain mapping used
        }

        // Now prepare the raw material index to Ogre Material reference mapping to be used through the conversion
        for (int n = 0; n < mHdr.num_mats; n++) {
            MaterialPtr mat = prepareMaterial(mMesh->getName() + "/" + mMaterials[n].name, mMaterials[n], mMaterialsExtra[n]);

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
        if (mNumUVs > 0) { // can be zero if all the model is only color (No TXT)
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
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,"Number of vertices is zero!", "ObjectMeshLoader::readVertices");
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

        FillerMap::iterator it;
        String matName = "";
        int fillerIdx;

        // TODO: MD_PGON_WIRE

        // Depending on the type, find the right filler, or construct one if not yet constructed
        if (type == MD_PGON_TMAP) {
            // sanity check the material index
            if (ply.index >= mHdr.num_mats)
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,"Material index out of range", "ObjectMeshLoader::getFillerForPolygon");

            // Get the material index from the index (can be slot idx...)
            int matidx = getMaterialIndex(ply.index);

            // Color mode is ignored. We search the filler table simply by the material index
            it = mFillers.find(matidx);
            fillerIdx = matidx;
            matName = mMesh->getName() + "/" + mMaterials[matidx].name;

        } else if (type == MD_PGON_SOLID) {
            // Solid color polygon. This means we need to see if we use Material or Color table index
            if (color_mode == MD_PGON_SOLID_COLOR_PAL) {
                // Dynamically created material. We allocate negative numbers for these fillers
                // Color mode is ignored. We search the filler table simply by the material index

                matName = mMesh->getName() + "/Color" + StringConverter::toString(ply.index);

                // Generate the solid material...
                MaterialPtr material = createPalMaterial(matName, ply.index);

                FillerMap::iterator it = mFillers.find(-ply.index);
                fillerIdx = -ply.index;

                mOgreMaterials.insert(make_pair(-ply.index,material));

            } else if (color_mode == MD_PGON_SOLID_COLOR_VCOLOR) {

                int matidx = getMaterialIndex(ply.index);

                FillerMap::iterator it = mFillers.find(matidx);
                fillerIdx = matidx;
                matName = mMesh->getName() + "/" + mMaterials[matidx].name;
            } else
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,"Unrecognized color_mode for polygon", "ObjectMeshLoader::getFillerForPolygon");
        } else
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,"Unknown or invalid polygon type: " + ply.type, "ObjectMeshLoader::getFillerForPolygon");


        if (it == mFillers.end()) {
            SubMeshFiller* f = new SubMeshFiller(mVertices, mUVs, true);
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
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,"Unknown material slot index : " + slotidx, "ObjectMeshLoader::getMaterialIndex");
    }

    //-------------------------------------------------------------------
    MaterialPtr ObjectMeshLoader::prepareMaterial(String matname, MeshMaterial& mat, MeshMaterialExtra& matext) {
        // Look if the material is already defined or not (This enables anyone to replace the material without modifying anything)
        if (MaterialManager::getSingleton().resourceExists(matname)) {
            MaterialPtr fmat = MaterialManager::getSingleton().getByName(matname);
            return fmat;
        }

        // We'll create a material given the mat and matext structures
        MaterialPtr omat = MaterialManager::getSingleton().create(matname, mMesh->getGroup());

        // fill it with the values given
        Pass *pass = omat->getTechnique(0)->getPass(0);

        if (mat.type == MD_MAT_TMAP) {
            // Texture unit state for the main texture...
            TextureUnitState* tus = pass->createTextureUnitState(mat.name);

            tus->setTextureAddressingMode(TextureUnitState::TAM_WRAP);
            tus->setTextureCoordSet(0);
            tus->setTextureFiltering(TFO_BILINEAR);

            // If the transparency or illumination is used
            if (( mHdr.mat_flags & MD_MAT_TRANS || mHdr.mat_flags & MD_MAT_ILLUM ) && (matext.trans < 1)) {
                // set at least the transparency value for the material
                tus->setColourOperation(LBO_ALPHA_BLEND);
                tus->setAlphaOperation(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, matext.trans);
                pass->setDepthWriteEnabled(false);
            } else {
                // Set replace on all first layer textures for now
                tus->setColourOperation(LBO_REPLACE);
            }

            omat->setCullingMode(CULL_ANTICLOCKWISE);
            omat->setLightingEnabled(true);
        } else {
            // Fill in a color-only material
            TextureUnitState* tus = pass->createTextureUnitState(mat.name);

            // set the material color. Now I don't know what illumination is, so I just ignore
            tus->setColourOperationEx(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, ColourValue(mat.colour[2], mat.colour[1], mat.colour[0])); // , mat.colour[3]

            if (( mHdr.mat_flags & MD_MAT_TRANS || mHdr.mat_flags & MD_MAT_ILLUM ) && (matext.trans < 1)) {
                tus->setColourOperation(LBO_ALPHA_BLEND);
                tus->setAlphaOperation(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, matext.trans);
                tus->setAlphaOperation(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, matext.trans);
            } else {
                tus->setColourOperation(LBO_REPLACE);
            }

        }

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
        MaterialPtr omat = MaterialManager::getSingleton().create(matname, mMesh->getGroup());

        // fill it with the values given
        Pass *pass = omat->getTechnique(0)->getPass(0);

        // Fill in a color-only material
        TextureUnitState* tus = pass->createTextureUnitState(0);

        // set the material color from the lg system color table
        tus->setColourOperationEx(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT,
                ColourValue(
                    lg_system_colors[palindex][0],
                    lg_system_colors[palindex][1],
                    lg_system_colors[palindex][2]
                    )
            );

        tus->setColourOperation(LBO_REPLACE);

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

        //Open the file, and detect the mesh type (Model/AI)
        Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource(m->getName(), m->getGroup(), true, resource);

        FilePtr f = new OgreFile(stream);

        char _hdr[5];
        _hdr[4] = 0;

        uint32_t version;

        // Look at the 4 byte header
        f->read(_hdr, 4); // read the LGMM/LGMD header
        f->read(&version, 4); // read the version int

        String header(_hdr);

        if (_hdr == "LGMM") {
            // AI mesh. Not yet supported
        } else if (_hdr == "LGMD") {
            // model. Supported for good. Load!
            ObjectMeshLoader ldr(m, f, version);
            ldr.load(); // that's all. Will do what's needed
        } else
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("Unknown BIN model format : ") + _hdr, "ManualBinFileLoader::loadResource");
    }
}


