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

/*
thanks go to shadowspawn for his great contribution (and Ryan Nunn for his work on this thing too, as well as maybe other, which I do not know about)
*/

#ifndef __BINFORMAT_H
#define __BINFORMAT_H

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
};

typedef struct PolyParts {
    int32_t             a;
    int32_t             b;
    int32_t             c;
};

typedef struct PolyPartsShorts {
    int16_t	      a;
    int16_t           b;
    int16_t           c;
};

typedef struct UVMap {
    float           u;
    float           v;
};

// Main header
typedef struct BinHeadType {
    char            ID[4];          // 'LGMD', 'LGMM'
    uint32_t        version;        // 3, 4 - 1
};

// the object file type headers:
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

// the ?skeleton? joint definitions(?)
typedef struct      VHotObj
{
    uint32_t        index;
    Vertex          point;
} VHotObj;

// Material definitions
typedef struct MeshMaterial {
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
};

// Optional
struct MeshMaterialExtra {
	float		trans;
	float		illum;
};

typedef struct {
    int32_t	    parent; // A numbered parent identification (Parent sub-object index) or -1 if no parent exists.
    // Comment: I expect this to rather be an SubObject index.
    float           min_range;   // minimal angle/translation ?
    float           max_range;   // maximal angle/translation ?
    float           rot[9]; // Transformation matrix. Rotation and translation comparing the parent object (not used for parent imho)
    Vertex           AxlePoint;   // Position of this sub-object
} SubObjTransform;

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

// geometry nodes definitions
typedef struct      NodeHeader
{
    uint8_t            subObjectID; // So I can skip those sub-objs that don't match
    // This is probably used if MD_NODE_CALL skips from one object to another.
    // I would reckon that the transform of object indicated here is used rather than the one given by the object in progress
    uint8_t            object_number;
    uint8_t            c_unk1;

} NodeHeader;

#define NODE_HEADER_SIZE 3

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

typedef struct      NodeCall
{
    Vertex          sphere_center;
    float           sphere_radius;
    short           pgon_before_count;
    short           call_node; // Inserted node?
    short           pgon_after_count;
} NodeCall;

#define NODE_CALL_SIZE 22

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

typedef struct      ObjPolygon {
    uint16_t  index;              // Index of the Polygon
    int8_t    data;               // ?
    uint8_t   type;               // MD_PGON Type
    uint8_t   num_verts;          // Number of verts in polygon
    uint16_t  norm;               // Polygon normal number
    float     d;                  // ?
} ObjPolygon;


#endif
