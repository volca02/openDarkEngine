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
thanks go to shadowspawn for his great contribution (and Ryan Nunn for his work on this thing too, as well as maybe others, whom I do not know about)
Parts of this file are exact copies or free-style rewrites of the contributed code
*/

#ifndef __BINFORMAT_H
#define __BINFORMAT_H

#include "config.h"

// Material flags (known)
#define MD_MAT_TRANS		1
#define MD_MAT_ILLUM		2

// Material Types (known)
#define MD_MAT_TMAP		0
#define MD_MAT_COLOR		1

// Polygon types ( & 0x07h )
#define MD_PGON_NONE		0
#define MD_PGON_SOLID		1
#define MD_PGON_WIRE		2
#define MD_PGON_TMAP		3

// Polygon Flags ( & 0x60h )
#define MD_PGON_SOLID_COLOR_PAL     0x20
#define MD_PGON_SOLID_COLOR_VCOLOR  0x40

/*
#define MD_PGON_TMAP                0x1B    // 0001 1011
#define MD_PGON_SOLID_COLOR_PAL     0x39    // 0011 1001
#define MD_PGON_SOLID_COLOR_VCOLOR  0x59    // 0101 1001
*/

// Sub object types (Guess)
#define MD_SUB_NONE		0
#define MD_SUB_ROT		1
#define MD_SUB_SLIDE		2

// BSP Node types (Guess)
#define MD_NODE_RAW         0
#define MD_NODE_SPLIT       1
#define MD_NODE_CALL        2
#define MD_NODE_HDR         4

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

struct PolyParts {
    int32_t             a;
    int32_t             b;
    int32_t             c;
};

struct PolyPartsShorts {
    int16_t	      a;
    int16_t           b;
    int16_t           c;
};

struct UVMap {
    float           u;
    float           v;
};

/// The main header of all .BIN files. Describes the contents of the file and it's version.
struct BinHeadType {
    char            ID[4];          /// 'LGMD', 'LGMM' - either model or AI mesh
    uint32_t        version;        // Versions - 3, 4 for LGMD, 1 for LGMM are supported
};

/// the main header of LGMD .BIN file:
typedef struct BinHeader {
	char            ObjName[8];
	float	        sphere_rad;
	float	        max_poly_rad;

	float	        bbox_max[3];
	float	        bbox_min[3];
	float	        parent_cen[3];

	uint16_t	num_pgons;
	uint16_t	num_verts;
	uint16_t	num_parms;
	uint8_t	num_mats;
	uint8_t	num_vcalls;
	uint8_t	num_vhots;
	uint8_t	num_objs;

	uint32_t            offset_objs;
	uint32_t            offset_mats;
	uint32_t            offset_uv;
	uint32_t            offset_vhots;
	uint32_t            offset_verts;
	uint32_t            offset_light;
	uint32_t            offset_norms;
	uint32_t            offset_pgons;
	uint32_t            offset_nodes;
	uint32_t            model_size;

	// version 4 addons
	int32_t	            mat_flags;
	int32_t	            offset_new1;
	int32_t	            offset_new2;
} BinHeader;

// the sizes of the header versions
#define SIZE_BIN_HDR_V4 (sizeof(BinHeader))
#define SIZE_BIN_HDR_V3 (SIZE_BIN_HDR_V4 - 12)

/// the attachment joint definition (arbitary attachment "slot")
typedef struct      VHotObj {
    uint32_t        index;
    Vertex          point;
} VHotObj;

/// Material definition struct for LGMD type .BIN file
typedef struct {
	char		name[16];
	uint8_t	type;		// MD_MAT_COLOR or MD_MAT_TMAP
	uint8_t	slot_num;

	union {
		// MD_MAT_TMAP
		struct {
			uint32_t	handle;		// Couldn't care less
			float		uvscale;	// Couldn't care less
		};

		// MD_MAT_COLOR
		struct {
			uint8_t	colour[4];
			uint32_t	ipal_index;	// Couldn't care less
		};
	};
} MeshMaterial;

/// Optional additional .BIN file material parameters (stored separately)
typedef struct {
	float		trans;
	float		illum;
} MeshMaterialExtra;

/// Transformation structure that describes how the model part is attached to it's parent (.BIN LGMD uses a transform tree, similar to skeleton for AI meshes)
typedef struct {
    int32_t	    parent; /// A numbered parent identification (Parent sub-object index) or -1 if no parent exists.
    // Comment: I expect this to rather be an SubObject index.
    float           min_range;   /// minimal angle/translation ?
    float           max_range;   /// maximal angle/translation ?
    float           rot[9]; /// Transformation matrix. Rotation and translation comparing the parent object (not used for parent imho)
    Vertex           AxlePoint;   /// Position of this sub-object
} SubObjTransform;

/// Header of the subobject. Describes the stored geometry for the .BIN LGMD subobject.
typedef struct {
	char	name[8];

	uint8_t movement; // the movement of the object 0 - none, 1 - rotate, 2 - slide

	SubObjTransform trans;

	short           child_sub_obj;
	short           next_sub_obj;
	short           vhot_start;
	short           sub_num_vhots;
	short           point_start;
	short           sub_num_points;
	short           light_start;
	short           sub_num_lights;
	short           norm_start;
	short           sub_num_norms;
	short           node_start;
	short           sub_num_nodes;
} SubObjectHeader;

/// BSP node header - header for .BIN LGMD geometry nodes definitions
typedef struct      NodeHeader
{
    uint8_t            subObjectID; // So I can skip those sub-objs that don't match
    // This is probably used if MD_NODE_CALL skips from one object to another.
    // I would reckon that the transform of object indicated here is used rather than the one given by the object in progress
    uint8_t            object_number;
    uint8_t            c_unk1;

} NodeHeader;

#define NODE_HEADER_SIZE 3
/// BSP split node header - secondary header for .BIN LGMD BSP node split plane definition
typedef struct      NodeSplit
{
    Vertex           sphere_center;
    float           sphere_radius;
    int16_t         pgon_before_count;
    uint16_t        normal;             // Split plane normal
    float           d;                  // Split plane d
    short           behind_node;        // offset to the node on the behind (from offset_nodes)
    short           front_node;         // offset to the node on the front (from offset_nodes)
    short           pgon_after_count;
} NodeSplit;

#define NODE_SPLIT_SIZE 26
/// BSP call node header - secondary header for .BIN LGMD BSP node indirection definition
typedef struct      NodeCall
{
    Vertex          sphere_center;
    float           sphere_radius;
    short           pgon_before_count;
    short           call_node; // Inserted node?
    short           pgon_after_count;
} NodeCall;

#define NODE_CALL_SIZE 22

/// BSP RAW node header - secondary header for .BIN LGMD BSP node raw data definition
typedef struct      NodeRaw // Simple Node. No splitting
{
    Vertex          sphere_center;
    float           sphere_radius;
    short           pgon_count;
} NodeRaw;

#define NODE_RAW_SIZE 18

// If version 3 and type is MD_PGON_TMAP or MD_PGON_SOLID_COLOR_VCOLOR
// data is the material index. Range: 1 - num_materials
//
// In any version, if type is MD_PGON_SOLID_COLOR_PAL data is the palette index

// Polygon definition for .BIN LGMD. Defines one polygon of the model.
typedef struct      ObjPolygon {
    uint16_t  index;              /// Index of the Polygon
    int8_t    data;               // ?
    uint8_t   type;               /// MD_PGON Type
    uint8_t   num_verts;          /// Number of verts in polygon
    uint16_t  norm;               /// Polygon normal number
    float     d;                  // ?
} ObjPolygon;


const int ObjLight_Size = 8;

/// Normal specifier (per vertex) for .BIN LGMD
typedef struct ObjLight {
	/// Material reference
	uint16_t material;

	/// Point on object reference
	uint16_t point;

	/// Packed normal vector (10 bits per axis, signed)
	uint32_t packed_normal;
} ObjLight;


//----- These are related to the CAL files: -----

/// The header struct of the .CAL file
typedef struct {
    int32_t     Version; // We only know version 1
    int32_t     num_torsos;
    int32_t     num_limbs;
} CalHdr;

//  Torso array (array of TorsoV1) follows header
/// .CAL file torso definition (next to header, int the num_torsos count)
typedef struct {
    uint32_t            root; // Root - the root joint index of this torso. Init to 0,0,0 for parent == -1 to get zero - positioned skeleton
    int32_t            parent; // -1 - the torso's parent (-1 for root torso)
    int32_t            fixed_count; // count of joints of this torso (maxed to 16)
    uint32_t            fixed_joints[16]; // index remap of the joints (could be checked for uniqueness for sanity checks)
    Vertex          fixed_joint_diff_coord[16]; // the relative position of the torso's joint to the root joint
} CalTorso;

/// .CAL file limb definition - Limbs follow the Torsos in the .CAL file
typedef struct {
    int32_t		torso_index; /// index of the torso we attach to
    int32_t		junk1; /// What's this?
    int32_t		num_segments;  /// count of joints in this limb
    uint16_t		attachment_joint; /// joint to which the limb attaches
    uint16_t		segments[16]; /// indices of the joints of this limb
    Vertex			segment_diff_coord[16]; /// relative to the previous limb's joint!
    float			lengths[16];             /// Lengths of the segment
} CalLimb;

//----- The Structures used in the LGMM AI .BIN mesh file -----
/// the main header of the .BIN LGMM model. This is the primary header of AI meshes (.BIN files starting LGMM)
typedef struct {
	uint32_t            zeroes[3];      /// Always seems to be 0
	uint8_t             num_what1;         /// '0'
    uint8_t             num_mappers;         /// Count for U2 (*20)
    uint8_t             num_mats;       /// Number of materials
    uint8_t             num_joints;     /// Number of joints?
    int16_t             num_polys;        /// Polygon count (Count for U4 * 16)
    int16_t             num_vertices;       /// Total Vertex count
    uint32_t            num_stretchy;       /// Stretchy Vertexes - blended between two joints
    uint32_t            offset_joint_remap;      /// Joint map?, num_joints elements
    uint32_t            offset_mappers;      /// joint mapping definitions
    uint32_t            offset_mats;    /// looks likes material offset, see object header
    uint32_t            offset_joints;  /// Per-Joint Polygon info. The joints mentioned here are not the same joints as in .CAL
    uint32_t            offset_poly;    /// (U5-U4) = polygons
    uint32_t            offset_norm;    /// (U6-U5) = Normals
    uint32_t            offset_vert;    /// (U7-U6) = Vertexes (munged) - num_vertices
    uint32_t            offset_uvmap;   /// (U8-U7) = UV maps. Z is junk (count - the same as vertex count)
    uint32_t            offset_blends;  /// Floats (num_stretchy). Blending factors. All in the range 0-1. Probably blend factors between two joints. Count - the same as num_stretchy
    uint32_t            offset_U9;      /// Zero. All the time it seems
} AIMeshHeader;

// then, there are the joint remapping structs (2x num_joints of bytes)

/// This structure seems to map the AI mesh joints to the Cal joints.
typedef struct {
    long            unk1;
    char            joint; /// in the joint info (joint->poly lists) this stuct is referenced, and this attr is seeked
    char            en1; /// 0/1 I guess these enable the usage of the blending for the particular joint
    char            jother; /// maybe stretchy vertex reference, or whatever. Need more info here
    char            en2; /// 0/1 Maybe this is enabling the referencing of stretchy vertices
    float           rotation[3]; /// I just guess this can be rotation for the bone
} AIMapper;

// We handle revision 1 and 2 AI meshes. These have different material structure. commented are the versions for the ver-dependent fields
/// Material definition structure for AI type meshes (LGMM). Describes the material used on AI meshes.
typedef struct {
	char name[16];

	// Only in rev. 2 mesh: (This part is skipped for rev. 1 meshes)
	uint32_t	ext_flags; // 2 only - indicate which of the params is used (trans, illum, etc). Bitmask
	float trans; // 2 only
	float illum; // 2 only
	float unknown; // 2 only

	// back to both rev. fields:
	uint32_t unk1;
	uint32_t unk2;
	unsigned char type; // Textured, color-only, etc....
	unsigned char slot_num; // material's slot. We know this one from the Object meshes, don't we?

	// some other data, not identified yet
	// I'd bet we'll see 8 bytes of info here, the same as in MeshMaterial - color and ipal_index, OR, handle and uv-scale
	uint16_t   s_unk1; // 2
    uint16_t   s_unk2; // 4
    uint16_t   s_unk3; // 6
    uint16_t   s_unk4; // 8
    uint16_t   s_unk5; // What would this be?
    uint32_t   l_unk3; // and this?

} AIMaterial;


/// Joint -> polygons mapping struct for AI meshes.
typedef struct JointInfo {
    short           num_polys;            /// Number of polygons
    short           start_poly;            /// Start poly
    short           num_vertices;            /// Number of vertices
    short           start_vertex;            /// Start vertex
    float           jflt;		/// I suppose this is a blending factor for the bone
    short           sh6;            /// Flag (?) - there are few places for TG this is not zero, but either 1,2 or 3
    short           mapper_id;            /// ID of the mapper struct
} AIJointInfo;

/// Triangle in AI mesh definition. Defines one triangle using indices to the vertex table, references material and exposes various flags.
typedef struct {
    short           a; /// vertex indicex
    short           b; /// vertex indicex
    short           c; /// vertex indicex
    short           mat; /// material ID
    float           f_unk; /// some float? Hmm what could've this be?
    short           index; /// index of this
    unsigned short  flag; /// stretch or not? This would seem to be a good place to inform about it
} AITriangle;


#endif
