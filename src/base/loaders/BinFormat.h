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
 *	  $Id$
 *
 *****************************************************************************/

/*
thanks go to shadowspawn for his great contribution (and Ryan Nunn for his work
on this thing too, as well as maybe others, whom I do not know about) Parts of
this file are exact copies or free-style rewrites of the contributed code
*/

#ifndef __BINFORMAT_H
#define __BINFORMAT_H

#include "config.h"
#include "integers.h"
#include "File.h"

namespace Opde {


// Material flags (known)
enum MaterialFlags {
    MD_MAT_TRANS = 1,
    MD_MAT_ILLUM = 2
};

// Material Types (known)
enum MaterialType {
    MD_MAT_TMAP  = 0,
    MD_MAT_COLOR = 1
};

// Polygon types ( & 0x07h )
enum PolygonType {
    MD_PGON_NONE  = 0,
    MD_PGON_SOLID = 1,
    MD_PGON_WIRE  = 2,
    MD_PGON_TMAP  = 3
};

// Polygon Flags ( & 0x60h )
enum PolygonFlags {
    MD_PGON_SOLID_COLOR_PAL    = 0x20,
    MD_PGON_SOLID_COLOR_VCOLOR = 0x40
};

/*
#define MD_PGON_TMAP                0x1B    // 0001 1011
#define MD_PGON_SOLID_COLOR_PAL     0x39    // 0011 1001
#define MD_PGON_SOLID_COLOR_VCOLOR  0x59    // 0101 1001
*/

// Sub object types (Guess)
enum SubObjType {
    MD_SUB_NONE  = 0,
    MD_SUB_ROT   = 1,
    MD_SUB_SLIDE = 2
};

// BSP Node types (Guess)
enum BSPNodeType {
    MD_NODE_RAW   = 0,
    MD_NODE_SPLIT = 1,
    MD_NODE_CALL  = 2,
    MD_NODE_HDR   = 4
};

/// A single vertex (3D vector).
struct Vertex {
    float x;
    float y;
    float z;

    Vertex() { x = y = z = 0; };
    Vertex(float _x, float _y, float _z) {
        x = _x;
        y = _y;
        z = _z;
    }
};

File &operator >>(File &st, Vertex &v) {
    st >> v.x >> v.y >> v.z;
    return st;
}

File &operator <<(File &st, const Vertex &v) {
    st << v.x << v.y << v.z;
    return st;
}

struct PolyParts {
    int32_t a;
    int32_t b;
    int32_t c;
};

File &operator >>(File &st, PolyParts &p) {
    st >> p.a >> p.b >> p.c;
    return st;
}

File &operator <<(File &st, const PolyParts &p) {
    st << p.a << p.b << p.c;
    return st;
}

struct PolyPartsShorts {
    int16_t a;
    int16_t b;
    int16_t c;
};

File &operator >>(File &st, PolyPartsShorts &p) {
    st >> p.a >> p.b >> p.c;
    return st;
}

File &operator <<(File &st, const PolyPartsShorts &p) {
    st << p.a << p.b << p.c;
    return st;
}

struct UVMap {
    float u;
    float v;
};

File &operator >>(File &st, UVMap &m) {
    st >> m.u >> m.v;
    return st;
}

File &operator <<(File &st, const UVMap &m) {
    st << m.u << m.v;
    return st;
}

/// The main header of all .BIN files. Describes the contents of the file and
/// it's version.
struct BinHeadType {
    char ID[4];       /// 'LGMD', 'LGMM' - either model or AI mesh
    uint32_t version; // Versions - 3, 4 for LGMD, 1 for LGMM are supported
};

/// the main header of LGMD .BIN file:
struct BinHeader {
    char ObjName[8];
    float sphere_rad;
    float max_poly_rad;

    // TODO: These could be Vertex
    float bbox_max[3];
    float bbox_min[3];
    float parent_cen[3];

    uint16_t num_pgons;
    uint16_t num_verts;
    uint16_t num_parms;
    uint8_t num_mats;
    uint8_t num_vcalls;
    uint8_t num_vhots;
    uint8_t num_objs;

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
    /// Size of one record. We only know how to handle 0x08 (transp+illum)
    int32_t size_mat_extra;

    File &read(File &st, unsigned version) {
        st.read(ObjName, 8);
        st.readElem(&sphere_rad, 4);
        st.readElem(&max_poly_rad, 4);

        st.readElem(bbox_max, 4, 3);   // vector
        st.readElem(bbox_min, 4, 3);   // vector
        st.readElem(parent_cen, 4, 3); // vector

        st.readElem(&num_pgons, 2);
        st.readElem(&num_verts, 2);
        st.readElem(&num_parms, 2);

        st.read(&num_mats, 1);
        st.read(&num_vcalls, 1);
        st.read(&num_vhots, 1);
        st.read(&num_objs, 1);

        st.readElem(&offset_objs, 4);
        st.readElem(&offset_mats, 4);
        st.readElem(&offset_uv, 4);
        st.readElem(&offset_vhots, 4);
        st.readElem(&offset_verts, 4);
        st.readElem(&offset_light, 4);
        st.readElem(&offset_norms, 4);
        st.readElem(&offset_pgons, 4);
        st.readElem(&offset_nodes, 4);
        st.readElem(&model_size, 4);

        // version 4 addons
        if (version == 4) {
            st.readElem(&mat_flags, 4);
            st.readElem(&offset_mat_extra, 4);
            st.readElem(&size_mat_extra, 4);
        } else { // no mat flags, init to some non-colliding values
            mat_flags = 0;
            offset_mat_extra = 0;
            size_mat_extra = 0;
        }

        //
        return st;
    }
};

// the sizes of the header versions
// const size_t SIZE_BIN_HDR_V4 = (sizeof(BinHeader));
// const size_t SIZE_BIN_HDR_V3 = (SIZE_BIN_HDR_V4 - 12);

/// the attachment joint definition (arbitary attachment "slot")
struct VHotObj {
    uint32_t index;
    Vertex point;
};

File &operator >>(File &f, VHotObj &o) {
    f >> o.index >> o.point;
    return f;
}

File &operator <<(File &f, const VHotObj &o) {
    f << o.index << o.point;
    return f;
}

/// Material definition struct for LGMD type .BIN file
struct MeshMaterial {
    char name[16];
    uint8_t type; // MD_MAT_COLOR or MD_MAT_TMAP
    uint8_t slot_num;

    union {
        // MD_MAT_TMAP
        struct {
            uint32_t handle; // Couldn't care less
            float uvscale;   // Couldn't care less
        };

        // MD_MAT_COLOR
        struct {
            uint8_t colour[4];
            uint32_t ipal_index; // Couldn't care less
        };
    };
};

/// Optional additional .BIN file material parameters (stored separately)
struct MeshMaterialExtra {
    float trans;
    float illum;
};

File &operator >>(File &f, MeshMaterialExtra &mext) {
    f >> mext.trans >> mext.illum;
    return f;
}

File &operator <<(File &f, const MeshMaterialExtra &mext) {
    f << mext.trans << mext.illum;
    return f;
}

/// Transformation structure that describes how the model part is attached to
/// it's parent (.BIN LGMD uses a transform tree, similar to skeleton for AI
/// meshes)
struct SubObjTransform {
    int32_t parent; /// A numbered parent identification (Parent sub-object
                    /// index) or -1 if no parent exists.
    // Comment: I expect this to rather be an SubObject index.
    float min_range; /// minimal angle/translation ?
    float max_range; /// maximal angle/translation ?
    float rot[9]; /// Transformation matrix. Rotation and translation comparing
                  /// the parent object (not used for parent imho)
    Vertex axle_point; /// Position of this sub-object
};

File &operator >>(File &f, SubObjTransform &t) {
    // the transform stuff
    f >> t.parent
      >> t.min_range
      >> t.max_range;

    f.readElem(t.rot, 4, 9);

    f >> t.axle_point;

    return f;
}

File &operator <<(File &f, const SubObjTransform &t) {
    // the transform stuff
    f << t.parent
      << t.min_range
      << t.max_range;

    f.writeElem(t.rot, 4, 9);

    f << t.axle_point;

    return f;
}

/// Header of the subobject. Describes the stored geometry for the .BIN LGMD
/// subobject.
struct SubObjectHeader {
    char name[8];

    // the movement of the object 0 - none, 1 - rotate, 2 - slide
    uint8_t movement;

    SubObjTransform trans;

    int16_t child_sub_obj;
    int16_t next_sub_obj;
    int16_t vhot_start;
    int16_t sub_num_vhots;
    int16_t point_start;
    int16_t sub_num_points;
    int16_t light_start;
    int16_t sub_num_lights;
    int16_t norm_start;
    int16_t sub_num_norms;
    int16_t node_start;
    int16_t sub_num_nodes;
};

File &operator >>(File &f, SubObjectHeader &h) {
    f.read(h.name, 8);

    f >> h.movement
      >> h.trans
      >> h.child_sub_obj
      >> h.next_sub_obj
      >> h.vhot_start
      >> h.sub_num_vhots
      >> h.point_start
      >> h.sub_num_points
      >> h.light_start
      >> h.sub_num_lights
      >> h.norm_start
      >> h.sub_num_norms
      >> h.node_start
      >> h.sub_num_nodes;

    return f;
}

File &operator <<(File &f, const SubObjectHeader &h) {
    f.write(h.name, 8);

    f << h.movement
      << h.trans
      << h.child_sub_obj
      << h.next_sub_obj
      << h.vhot_start
      << h.sub_num_vhots
      << h.point_start
      << h.sub_num_points
      << h.light_start
      << h.sub_num_lights
      << h.norm_start
      << h.sub_num_norms
      << h.node_start
      << h.sub_num_nodes;

    return f;
}

/// BSP node header - header for .BIN LGMD geometry nodes definitions
struct NodeHeader {
    static const uint32_t SIZE = 3;

    uint8_t subObjectID; // So I can skip those sub-objs that don't match
    // This is probably used if MD_NODE_CALL skips from one object to another.
    // I would reckon that the transform of object indicated here is used rather
    // than the one given by the object in progress
    uint8_t object_number;
    uint8_t c_unk1;

};

File &operator >>(File &f, NodeHeader &h) {
    f >> h.subObjectID >> h.object_number >> h.c_unk1;
    return f;
}

File &operator <<(File &f, const NodeHeader &h) {
    f << h.subObjectID << h.object_number << h.c_unk1;
    return f;
}

// const size_t NODE_HEADER_SIZE = 3;

/// BSP split node header - secondary header for .BIN LGMD BSP node split plane
/// definition
struct NodeSplit {
    Vertex sphere_center;
    float sphere_radius;
    int16_t pgon_before_count;
    uint16_t normal;   // Split plane normal
    float d;           // Split plane d
    short behind_node; // offset to the node on the behind (from offset_nodes)
    short front_node;  // offset to the node on the front (from offset_nodes)
    short pgon_after_count;
};

File &operator >>(File &f, NodeSplit &s) {
    f >> s.sphere_center >> s.sphere_radius >> s.pgon_before_count >>
        s.normal >> s.d >> s.behind_node >> s.front_node >> s.pgon_after_count;
    return f;
}

File &operator <<(File &f, const NodeSplit &s) {
    f << s.sphere_center << s.sphere_radius << s.pgon_before_count << s.normal
      << s.d << s.behind_node << s.front_node << s.pgon_after_count;
    return f;
}

// const size_t NODE_SPLIT_SIZE = 26;

/// BSP call node header - secondary header for .BIN LGMD BSP node indirection
/// definition
struct NodeCall {
    Vertex sphere_center;
    float sphere_radius;
    short pgon_before_count;
    short call_node; // Inserted node?
    short pgon_after_count;
};

File &operator >>(File &f, NodeCall &c) {
    f >> c.sphere_center >> c.sphere_radius >> c.pgon_before_count >>
        c.call_node >> c.pgon_after_count;
    return f;
}

File &operator <<(File &f, const NodeCall &c) {
    f << c.sphere_center << c.sphere_radius << c.pgon_before_count <<
        c.call_node << c.pgon_after_count;
    return f;
}

// const size_t NODE_CALL_SIZE = 22;

/// BSP RAW node header - secondary header for .BIN LGMD BSP node raw data
/// definition
struct NodeRaw // Simple Node. No splitting
{
    Vertex sphere_center;
    float sphere_radius;
    short pgon_count;
};

File &operator >>(File &f, NodeRaw &n) {
    f >> n.sphere_center >> n.sphere_radius >> n.pgon_count;
    return f;
}

File &operator <<(File &f, const NodeRaw &n) {
    f << n.sphere_center << n.sphere_radius << n.pgon_count;
    return f;
}


// const size_t NODE_RAW_SIZE = 18;

// If version 3 and type is MD_PGON_TMAP or MD_PGON_SOLID_COLOR_VCOLOR
// data is the material index. Range: 1 - num_materials
//
// In any version, if type is MD_PGON_SOLID_COLOR_PAL data is the palette index

// Polygon definition for .BIN LGMD. Defines one polygon of the model.
struct ObjPolygon {
    uint16_t index;    /// Index of the Polygon
    int16_t data;       // ?
    uint8_t type;      /// MD_PGON Type
    uint8_t num_verts; /// Number of verts in polygon
    uint16_t norm;     /// Polygon normal number
    float d;           /// d - makes up the plane definition with norm
};

File &operator>>(File &f, ObjPolygon &p) {
    f >> p.index >> p.data >> p.type >> p.num_verts >> p.norm >> p.d;
    return f;
}

File &operator>>(File &f, const ObjPolygon &p) {
    f << p.index << p.data << p.type << p.num_verts << p.norm << p.d;
    return f;
}

const int ObjLight_Size = 8;

/// Normal specifier (per vertex) for .BIN LGMD
struct ObjLight {
    /// Material reference
    uint16_t material;

    /// Point on object reference
    uint16_t point;

    /// Packed normal vector (10 bits per axis, signed)
    uint32_t packed_normal;
};

File &operator>>(File &f, ObjLight &l) {
    f >> l.material >> l.point >> l.packed_normal;
    return f;
}

File &operator>>(File &f, const ObjLight &l) {
    f << l.material << l.point << l.packed_normal;
    return f;
}

//----- These are related to the CAL files: -----

/// The header struct of the .CAL file
struct CalHdr {
    int32_t version; // We only know version 1
    int32_t num_torsos;
    int32_t num_limbs;
};

File &operator>>(File &f, CalHdr &ch) {
    f >> ch.version >> ch.num_torsos >> ch.num_limbs;
    return f;
}

File &operator>>(File &f, const CalHdr &ch) {
    f << ch.version << ch.num_torsos << ch.num_limbs;
    return f;
}

//  Torso array (array of TorsoV1) follows header
/// .CAL file torso definition (next to header, int the num_torsos count)
struct CalTorso {
    uint32_t root;  // Root - the root joint index of this torso. Init to 0,0,0
                    // for parent == -1 to get zero - positioned skeleton
    int32_t parent; // -1 - the torso's parent (-1 for root torso)
    int32_t fixed_count;       // count of joints of this torso (maxed to 16)
    uint32_t fixed_joints[16]; // index remap of the joints (could be checked
                               // for uniqueness for sanity checks)
    Vertex fixed_joint_diff_coord[16]; // the relative position of the torso's
                                       // joint to the root joint
};

File &operator>>(File &f, CalTorso &t) {
    f >> t.root >> t.parent >> t.fixed_count;

    for (int32_t c = 0; c < 16; ++c) {
        f >> t.fixed_joints[c];
    }

    for (int32_t c = 0; c < 16; ++c) {
        f >> t.fixed_joint_diff_coord[c];
    }

    return f;
}

File &operator>>(File &f, const CalTorso &t) {
    f << t.root << t.parent << t.fixed_count;

    for (int32_t c = 0; c < 16; ++c) {
        f << t.fixed_joints[c];
    }

    for (int32_t c = 0; c < 16; ++c) {
        f << t.fixed_joint_diff_coord[c];
    }

    return f;
}

/// .CAL file limb definition - Limbs follow the Torsos in the .CAL file
struct CalLimb {
    int32_t torso_index;           /// index of the torso we attach to
    int32_t junk1;                 /// What's this?
    int32_t num_segments;          /// count of joints in this limb
    uint16_t attachment_joint;     /// joint to which the limb attaches
    uint16_t segments[16];         /// indices of the joints of this limb
    Vertex segment_diff_coord[16]; /// relative to the previous limb's joint!
    float lengths[16];             /// Lengths of the segment
};

File &operator>>(File &f, CalLimb &l) {
    f >> l.torso_index >> l.junk1 >>
        l.num_segments >> l.attachment_joint;

    for (int32_t c = 0; c < 16; ++c) {
        f >> l.segments[c];
    }

    for (int32_t c = 0; c < 16; ++c) {
        f >> l.segment_diff_coord[c];
    }

    for (int32_t c = 0; c < 16; ++c) {
        f >> l.lengths[c];
    }

    return f;
}

File &operator>>(File &f, const CalLimb &l) {
    f << l.torso_index << l.junk1 <<
        l.num_segments << l.attachment_joint;

    for (int32_t c = 0; c < 16; ++c) {
        f << l.segments[c];
    }

    for (int32_t c = 0; c < 16; ++c) {
        f << l.segment_diff_coord[c];
    }

    for (int32_t c = 0; c < 16; ++c) {
        f << l.lengths[c];
    }

    return f;
}

//----- The Structures used in the LGMM AI .BIN mesh file -----
/// the main header of the .BIN LGMM model. This is the primary header of AI
/// meshes (.BIN files starting LGMM)
struct AIMeshHeader {
    uint32_t zeroes[3];    /// Always seems to be 0
    uint8_t num_what1;     /// '0'
    uint8_t num_mappers;   /// Count for U2 (*20)
    uint8_t num_mats;      /// Number of materials
    uint8_t num_joints;    /// Number of joints?
    int16_t num_polys;     /// Polygon count (Count for U4 * 16)
    int16_t num_vertices;  /// Total Vertex count
    uint32_t num_stretchy; /// Stretchy Vertexes - blended between two joints
    uint32_t offset_joint_remap; /// Joint map?, num_joints elements
    uint32_t offset_mappers;     /// joint mapping definitions
    uint32_t offset_mats;   /// looks likes material offset, see object header
    uint32_t offset_joints; /// Per-Joint Polygon info. The joints mentioned
                            /// here are not the same joints as in .CAL
    uint32_t offset_poly;   /// polygons (num_polys)
    uint32_t offset_norm;   /// Normals (no counter, (offset_vert-offset_norm)/12)
    uint32_t offset_vert;   /// Vertex data (munged) - num_vertices
    uint32_t offset_uvmap; /// (U8-U7) = UV maps + 32 bit packed normals
    uint32_t offset_blends; /// Floats (num_stretchy). Blending factors. All in
                            /// the range 0-1. Probably blend factors between
                            /// two joints. Count - the same as num_stretchy
    uint32_t offset_U9;     /// Zero. All the time it seems

    File &read(File &st) {
        st.readElem(zeroes, sizeof(uint32_t), 3);

        st.read(&num_what1, 1);
        st.read(&num_mappers, 1);
        st.read(&num_mats, 1);
        st.read(&num_joints, 1);

        st.readElem(&num_polys, 2);
        st.readElem(&num_vertices, 2);

        st.readElem(&num_stretchy, 4);

        // offsets...
        st.readElem(&offset_joint_remap, 4);
        st.readElem(&offset_mappers, 4);
        st.readElem(&offset_mats, 4);
        st.readElem(&offset_joints, 4);
        st.readElem(&offset_poly, 4);
        st.readElem(&offset_norm, 4);
        st.readElem(&offset_vert, 4);
        st.readElem(&offset_uvmap, 4);
        st.readElem(&offset_blends, 4);
        st.readElem(&offset_U9, 4);

        return st;
    }

};

// then, there are the joint remapping structs (2x num_joints of bytes)

/// This structure seems to map the AI mesh joints to the Cal joints.
struct AIMapper {
    int32_t unk1;
    /// in the joint info (joint->poly lists) this stuct is referenced, and this
    /// attr is seeked
    int8_t joint;
    /// 0/1 I guess these enable the usage of the blending for the particular
    /// joint
    int8_t en1;
    /// maybe stretchy vertex reference, or whatever. Need more info here
    int8_t jother;
    /// 0/1 Maybe this is enabling the referencing of stretchy vertices
    int8_t en2;
    float rotation[3]; /// I just guess this can be rotation for the bone
};

File &operator >>(File &f, AIMapper &m) {
    f >> m.unk1 >> m.joint >> m.en1 >> m.jother >> m.en2 >> m.rotation[0] >>
        m.rotation[1] >> m.rotation[2];
    return f;
}

File &operator <<(File &f, const AIMapper &m) {
    f << m.unk1 << m.joint << m.en1 << m.jother << m.en2 << m.rotation[0] <<
        m.rotation[1] << m.rotation[2];
    return f;
}

// We handle revision 1 and 2 AI meshes. These have different material
// structure. commented are the versions for the ver-dependent fields
/// Material definition structure for AI type meshes (LGMM). Describes the
/// material used on AI meshes.
struct AIMaterial {
    char name[16];

    // Only in rev. 2 mesh: (This part is skipped for rev. 1 meshes)
    uint32_t ext_flags; // 2 only - indicate which of the params is used (trans,
                        // illum, etc). Bitmask
    float trans;        // 2 only
    float illum;        // 2 only
    float unknown;      // 2 only

    // back to both rev. fields:
    uint32_t unk1;
    uint32_t unk2;
    unsigned char type;     // Textured, color-only, etc....
    unsigned char slot_num; // material's slot. We know this one from the Object
                            // meshes, don't we?

    // some other data, not identified yet
    // I'd bet we'll see 8 bytes of info here, the same as in MeshMaterial -
    // color and ipal_index, OR, handle and uv-scale
    uint16_t s_unk1; // 2
    uint16_t s_unk2; // 4
    uint16_t s_unk3; // 6
    uint16_t s_unk4; // 8
    uint16_t s_unk5; // What would this be?
    uint32_t l_unk3; // and this?

    File &read(File &st, unsigned version) {
        st.read(name, 16);

        // version dep.
        if (version > 1) {
            st >> ext_flags
               >> trans
               >> illum
               >> unknown;
        } else {
            ext_flags = 0;
            trans = 0.0f;
            illum = 0.0f;
            unknown = 0.0f;
        }

        // back to version independent loading
        st >> unk1 >> unk2

            >> type >> slot_num

            >> s_unk1 >> s_unk2 >> s_unk3 >> s_unk4 >> s_unk5

            >> l_unk3;
        return st;
    }
};

/// Joint -> polygons mapping struct for AI meshes.
struct AIJointInfo {
    int16_t num_polys;    /// Number of polygons
    int16_t start_poly;   /// Start poly
    int16_t num_vertices; /// Number of vertices
    int16_t start_vertex; /// Start vertex
    float jflt;         /// I suppose this is a blending factor for the bone
    int16_t sh6; /// Flag (?) - there are few places for TG this is not zero, but
               /// either 1,2 or 3
    int16_t mapper_id; /// ID of the mapper struct
};

File &operator >>(File &f, AIJointInfo &j) {
    f >> j.num_polys >> j.start_poly >> j.num_vertices >> j.start_vertex >>
        j.jflt >> j.sh6 >> j.mapper_id;
    return f;
}

File &operator <<(File &f, const AIJointInfo &j) {
    f << j.num_polys << j.start_poly << j.num_vertices << j.start_vertex <<
        j.jflt << j.sh6 << j.mapper_id;
    return f;
}

/// Triangle in AI mesh definition. Defines one triangle using indices to the
/// vertex table, references material and exposes various flags.
struct AITriangle {
    int16_t vert[3]; /// vertex index, 3x
    int16_t mat;     /// material ID
    float   d;       /// plane D coeff
    int16_t norm;    /// normal index
    int16_t flags;   /// stretch or not? This would seem to be a good place
                     /// to inform about it
};

File &operator >>(File &f, AITriangle &t) {
    f >> t.vert[0] >> t.vert[1] >> t.vert[2] >> t.mat >> t.d >> t.norm >> t.flags;
    return f;
}

File &operator <<(File &f, AITriangle &t) {
    f << t.vert[0] << t.vert[1] << t.vert[2] << t.mat << t.d << t.norm << t.flags;
    return f;
}

} // namespace Opde

#endif
