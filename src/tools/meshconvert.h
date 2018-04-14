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
 *****************************************************************************/

/*
thanks go to shadowspawn for his great contribution (and Ryan Nunn for his work
on this thing too, as well as maybe other, which I do not know about)
*/

#ifndef bin2mesh_h
#define bin2mesh_h

#include <iostream>
#include <map>
#include <stdio.h>
#include <vector>

#include "integers.h"
#include "logging.h"

// Material flags (known)
#define MD_MAT_TRANS 1
#define MD_MAT_ILLUM 2

// Material Types (known)
#define MD_MAT_TMAP 0
#define MD_MAT_COLOR 1

// Polygon types (Guess)
#define MD_PGON_SOLID 0
#define MD_PGON_WIRE 1
#define MD_PGON_COLOR_PAL 2
#define MD_PGON_COLOR_VCOLOR 4

// Polygon types (Known)
#define MD_PGON_TMAP 0x1B               // 0001 1011
#define MD_PGON_SOLID_COLOR_PAL 0x39    // 0011 1001
#define MD_PGON_SOLID_COLOR_VCOLOR 0x59 // 0101 1001

// Sub object types (Guess)
#define MD_SUB_NONE 0
#define MD_SUB_ROT 1
#define MD_SUB_SLIDE 2

// BSP Node types (Guess)
#define MD_NODE_RAW 0
#define MD_NODE_SPLIT 1
#define MD_NODE_CALL 2
#define MD_NODE_HDR 4

#pragma pack(push, 1)

typedef struct Vertex {
    float x;
    float y;
    float z;

    Vertex() { x = y = z = 0; };
    Vertex(float _x, float _y, float _z) {
        x = _x;
        y = _y;
        z = _z;
    }
} Vertex;

typedef struct {
    int32_t a;
    int32_t b;
    int32_t c;
} PolyParts;

typedef struct {
    int16_t a;
    int16_t b;
    int16_t c;
} PolyPartsShorts;

typedef struct {
    float u;
    float v;
} UVMap;

// Main header
typedef struct {
    char ID[4];       // 'LGMD', 'LGMM'
    uint32_t version; // 3, 4 - 1
} BinHeadType;

// the object file type headers:
typedef struct BinHeader {
    char ObjName[8];
    float sphere_rad;
    float max_poly_rad;

    float bbox_max[3];
    float bbox_min[3];
    float parent_cen[3];

    uint16_t num_pgons;
    uint16_t num_verts;
    uint16_t num_parms;
    unsigned char num_mats;
    unsigned char num_vcalls;
    unsigned char num_vhots;
    unsigned char num_objs;

    uint32_t offset_objs;
    uint32_t offset_mats;
    uint32_t offset_uv;
    uint32_t offset_vhots;
    uint32_t offset_verts;
    uint32_t offset_light;
    uint32_t offset_norms;
    uint32_t offset_pgons;
    uint32_t offset_nodes;
    uint32_t model_size;

    // version 4 addons
    int32_t mat_flags;
    int32_t offset_mat_extra;
    int32_t size_mat_extra;

    // version 6 addons
    uint32_t offset_hdr2;
    uint32_t offset_end_parts;
} BinHeader;

// the sizes of the header versions
#define SIZE_BIN_HDR_V6 (sizeof(BinHeader))
#define SIZE_BIN_HDR_V4 (SIZE_BIN_HDR_V6 - 8)
#define SIZE_BIN_HDR_V3 (SIZE_BIN_HDR_V4 - 12)

typedef struct BinHeader2 {
    short num_uvs;
    short num_verts;
    char unk1;
    char unk2;
    uint32_t offset_verts;
    uint32_t offset_uv;
    uint32_t offset_unk1;
    uint32_t offset_norms;
    uint32_t offset_parts;
    uint32_t offset_unk2;
} BinHeader2;

// the ?skeleton? joint definitions(?)
typedef struct VHotObj {
    uint32_t index;
    Vertex point;
} VHotObj;

// Material definitions
struct MeshMaterial {
    char name[16];
    unsigned char type; // MD_MAT_COLOR or MD_MAT_TMAP
    unsigned char slot_num;

    union {
        // MD_MAT_TMAP
        struct {
            uint32_t handle; // Couldn't care less
            float uvscale;   // Couldn't care less
        };

        // MD_MAT_COLOR
        struct {
            unsigned char colour[4];
            uint32_t ipal_index; // Couldn't care less
        };
    };
};

// Optional
struct MeshMaterialExtra {
    float trans;
    float illum;
};

typedef struct {
    uint32_t JointNumber; // A numbered joint identification
    float min_range;      // minimal angle/translation ?
    float max_range;      // maximal angle/translation ?
    float f[9];           // Transformation matrix
    Vertex AxlePoint;     // seems to be a Joint position
} SubObjTransform;

typedef struct {
    char name[8];

    unsigned char
        movement; // the movement of the object 0 - none, 1 - rotate, 2 - slide

    SubObjTransform trans;

    short child_sub_obj;
    short next_sub_obj;
    short vhot_start;
    short sub_num_vhots;
    short point_start;
    short sub_num_points;
    short light_start;
    short sub_num_lights;
    short norm_start;
    short sub_num_norms;
    short node_start;
    short sub_num_nodes;
} SubObjectHeader;

// geometry nodes definitions
typedef struct NodeHeader {
    char flag;
    char object_number;
    char c_unk1;

} NodeHeader;

typedef struct NodeSplit {
    Vertex sphere_center;
    float sphere_radius;
    short pgon_before_count;
    short normal;      // Split plane normal
    float d;           // Split plane d
    short behind_node; // offset to the node on the behind (from offset_nodes)
    short front_node;  // offset to the node on the front (from offset_nodes)
    short pgon_after_count;
} NodeSplit;

typedef struct NodeCall {
    Vertex sphere_center;
    float sphere_radius;
    short pgon_before_count;
    short call_node; // Inserted node?
    short pgon_after_count;
} NodeCall;

typedef struct NodeRaw // Simple Node. No splitting
{
    Vertex sphere_center;
    float sphere_radius;
    short pgon_count;
} NodeRaw;

// If version 3 and type is MD_PGON_TMAP or MD_PGON_SOLID_COLOR_VCOLOR
// data is the material index. Range: 1 - num_materials
//
// In any version, if type is MD_PGON_SOLID_COLOR_PAL data is the palette index

typedef struct ObjPolygon {
    unsigned short index;    // Index of the Polygon
    short data;              // ?
    unsigned char type;      // MD_PGON Type
    unsigned char num_verts; // Number of verts in polygon
    unsigned short norm;     // Polygon normal number
    float d;                 // ?
} ObjPolygon;

#pragma pack(pop)

// what would definetaly be nice would be some << operators for these used
// structs, this way we could cout << material[1]; One can dream!
std::ostream &operator<<(std::ostream &os, const Vertex &v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

std::ostream &operator<<(std::ostream &os, const SubObjTransform &t) {
    os << "Joint:" << t.JointNumber << std::endl;
    os << "Min range:" << t.min_range << std::endl;
    os << "Max range:" << t.max_range << std::endl;
    os << "Matrix: ";

    for (int x = 0; x < 9; x++) {

        if (x % 3 == 0) {
            os.width(10);
            os << std::right;
            os << std::endl;
        }

        os.precision(3);
        os.width(5);
        os << std::right;
        os << " " << t.f[x];
    }

    os << "\n";

    os << "Joint (axle) point : " << t.AxlePoint;

    return os;
}

// helper class, we insert triangles of a single material into here.
// It should do this:
// smm.setVertexListPointer(vertices, count);
// smm.setUVListPointer(vertices, count);
// smm.setMaterial(materialstruct, index_of_this_material);
// smm.setTransform(NULL / SubObjHdr);
// smm.addTriangle(ida,idb,idc); // error if textured
// smm.addTriangle(ida,idb,idc, uva, uvb, uvc); // error if non-textured
// smm.output... ? ->
//	take all the vertices needed.
//	Make a conversion map - global -> local indexes
//	convert vertices using transformation, if needed
//	convert all indices (vertex / uv) to the local number.
//	put some vectors including our prepared structs out. (uv vectors,
//vertices, triangle-list) 		and optional vertex -> joint mapping (if specified)

/*
The thing is that each vertex in the output list has one Uv mapping entry. So if
we encounter a vertex index that allready exists in the list, we have to look if
it has the same uv. (so we have to look for combinations vertex-uv)
*/
using namespace std;

class SingleMaterialMesh {
private:
    vector<int> vertices;  // vertex indices (for triangles)
    vector<int> uvmapping; // uv map indices (for vertex UV mapping)
    vector<int> objidxs;   // map of the sub-object indices per vertex

    vector<PolyParts> triangles; // indexed triangle list
    bool hasuv;

    int material;
    char *baseName;

    Vertex *srcvertices;
    int num_vertices;

    UVMap *uvmaps;
    int num_uvmaps;

    SubObjectHeader *objects;
    int num_objects;

    int addVertex(int objidx, int vidx) {
        if (hasuv) {
            log_error("singleMaterialMesh.addVertex() : The material %d is "
                      "textured. I need UV!",
                      material);
            return -1;
        }

        // first we go through the allready inserted vertices, and look for the
        // same combination. if found, we have the resulting index of vertex to
        // be added to triangle definitions, otherwise, we have to insert a new
        // one
        for (unsigned int idx = 0; idx < vertices.size(); idx++) {
            if ((vertices[idx] == vidx) && (objidxs[idx] == objidx))
                return idx;
        }

        // we're here, so none found
        vertices.push_back(vidx);
        objidxs.push_back(objidx);

        return vertices.size() - 1; // the last index is the one
    }

    int addVertexUV(int objidx, int vidx, int uvidx) {
        if (!hasuv) {
            log_error("singleMaterialMesh.addVertexUV() : The material %d is "
                      "non-textured. I don't need UV!",
                      material);
            return -1;
        }

        // first we go through the allready inserted vertices, and look for the
        // same combination. if found, we have the resulting index of vertex to
        // be added to triangle definitions, otherwise, we have to insert a new
        // one
        for (unsigned int idx = 0; idx < vertices.size(); idx++) {
            if ((vertices[idx] == vidx) && (uvmapping[idx] == uvidx) &&
                (objidxs[idx] == objidx))
                return idx;
        }

        // we're here, so none found
        vertices.push_back(vidx);
        uvmapping.push_back(uvidx);
        objidxs.push_back(objidx);

        return vertices.size() - 1; // the last index is the one
    }

    /**
            Recalc the vertex coordinates using the given transformation;
    */
    Vertex transformVertex(int index) {
        Vertex in = srcvertices[vertices[index]];

        int objidx = objidxs[index];
        if (objidx == 0) // zero object index -> no transform
            return in;

        Vertex out;
        const SubObjTransform &trans = objects[objidx].trans;

        out.x = in.x * trans.f[0] + in.y * trans.f[3] + in.z * trans.f[6] +
                trans.AxlePoint.x;
        out.y = in.x * trans.f[1] + in.y * trans.f[4] + in.z * trans.f[7] +
                trans.AxlePoint.y;
        out.z = in.x * trans.f[2] + in.y * trans.f[5] + in.z * trans.f[8] +
                trans.AxlePoint.z;

        return out;
    }

public: //---------------------------------------------------
    SingleMaterialMesh(char *baseName, int materialnum, bool textured) {
        this->baseName = baseName;
        this->material = materialnum;
        hasuv = textured;
        num_vertices = num_objects = num_uvmaps = 0;
        objects = NULL;
        srcvertices = NULL;
        uvmaps = NULL;
    }

    void setObjects(SubObjectHeader *objects, int count) {
        this->objects = objects;
        num_objects = count;
    }

    void setVertices(Vertex *srcvtxs, int count) {
        this->srcvertices = srcvtxs;
        num_vertices = count;
    }

    void setUVMaps(UVMap *uvmaps, int count) {
        this->uvmaps = uvmaps;
        num_vertices = count;
    }

    void addTriangle(int objidx, short a, short b, short c) {
        if (hasuv) {
            log_error("singleMaterialMesh.addTriangle(a,b,c) : The material %d "
                      "is textured. I need UV!",
                      material);
            return;
        }

        // because we are nontextured, we only use vertices vector
        short ra = addVertex(objidx, a);
        short rb = addVertex(objidx, b);
        short rc = addVertex(objidx, c);

        PolyParts tri;
        tri.a = ra;
        tri.b = rb;
        tri.c = rc;

        triangles.push_back(tri);
    }

    void addTriangle(int objidx, short a, short b, short c, short uva,
                     short uvb, short uvc) {
        if (!hasuv) {
            log_error("singleMaterialMesh.addTriangle(a,b,c,-uv-) : The "
                      "material %d is non-textured. I need no UV!",
                      material);
            return;
        }

        // because we are nontextured, we only use vertices vector
        short ra = addVertexUV(objidx, a, uva);
        short rb = addVertexUV(objidx, b, uvb);
        short rc = addVertexUV(objidx, c, uvc);

        PolyParts tri;
        tri.a = ra;
        tri.b = rb;
        tri.c = rc;

        triangles.push_back(tri);
    }

    // preline should contain the right ammount of \t characters to indent the
    // xml correctly
    void output(ofstream &outter, const char *preline) {
        // output function
        //  we transform each of the entries according to the objidxs index
        //  (non-zero), zero index means no transform
        // so the vertices are transformed,

        if (objects == NULL) {
            log_error("singleMaterialMesh : Object list not set!");
            return;
        }

        if (srcvertices == NULL) {
            log_error("singleMaterialMesh : Vertex list not set!");
            return;
        }

        // the submesh header (an awfully long line)
        outter << preline << "<submesh material=\"" << baseName << "/"
               << "Mat" << material
               << "\" usesharedvertices=\"false\" use32bitindexes=\"false\" "
                  "operationtype=\"triangle_list\">"
               << endl;

        // output the triangles
        outter << preline << "\t<faces count=\"" << triangles.size() << "\">"
               << endl;

        for (unsigned int n = 0; n < triangles.size(); n++) {
            PolyParts &p = triangles[n];

            outter << preline << "\t\t<face ";
            outter << "v1=\"" << p.a << "\" ";
            outter << "v2=\"" << p.b << "\" ";
            outter << "v3=\"" << p.c << "\"/>" << endl;
        }

        outter << preline << "\t</faces>" << endl;

        // geometry (vertices and uvs)
        outter << preline << "\t<geometry vertexcount=\"" << vertices.size()
               << "\">" << endl;

        outter << preline
               << "\t\t<vertexbuffer positions=\"true\" normals=\"false\">"
               << endl;

        // output vertices
        for (unsigned int n = 0; n < vertices.size(); n++) {
            outter << preline << "\t\t\t<vertex>" << endl;
            outter << preline << "\t\t\t\t<position ";

            Vertex v = transformVertex(n);

            outter << "x=\"" << v.x << "\" ";
            outter << "y=\"" << v.y << "\" ";
            outter << "z=\"" << v.z << "\"/>" << endl;

            outter << preline << "\t\t\t</vertex>" << endl;
        }

        outter << preline << "\t\t</vertexbuffer>" << endl;

        // if we texture
        if (hasuv) {
            outter << preline
                   << "\t\t<vertexbuffer texture_coord_dimensions_0=\"2\" "
                      "texture_coords=\"1\">"
                   << endl;

            for (unsigned int n = 0; n < vertices.size(); n++) {
                outter << preline << "\t\t\t<vertex>" << endl;
                outter << preline << "\t\t\t\t<texcoord ";

                const UVMap &u = uvmaps[uvmapping[n]];

                outter << "u=\"" << u.u << "\" ";
                outter << "v=\"" << u.v << "\"/>" << endl;

                outter << preline << "\t\t\t</vertex>" << endl;
            }

            outter << preline << "\t\t</vertexbuffer>" << endl;
        }

        outter << preline << "\t</geometry>" << endl;

        outter << preline << "</submesh>" << endl;
    }
};

#endif
