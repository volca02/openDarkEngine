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


#ifndef __OGREOPDETYPES_H
#define __OGREOPDETYPES_H

#include "LightmapAtlas.h"
#include "integers.h"

// the only one which collided is wr_cell_hdr, but to be sure...
#pragma pack(push, 1)

namespace Opde {

	typedef struct { // SIZE: 8
		uint32_t  	unk;
		uint32_t	num_cells;
	} wr_hdr_t;

	typedef struct { // SIZE: 8
		float	 x;
		float    y;
		float    z;
	} wr_coord_t;


	typedef struct { // SIZE: 31
		uint8_t	num_vertices; // vertex count

		uint8_t	num_polygons;  // total number of polygons
		uint8_t	num_textured; // textured polys count
		uint8_t	num_portals; // faces that define a portal... Count this number from face_maps to get the first index of portal

		uint8_t	num_planes; // plane count
		uint8_t	media_type; // air == 1, water == 2 [TNH]
		uint8_t	cell_flags; // bit 6 is set in fog cells. bits 3+4 are set in doorways, probably for vision blocking  [TNH]. bit 1 - wireframe
		uint32_t	nxn; // size of the weird struct in bytes?

		uint16_t polymap_size; // this is repeated at the start of the polygon index list. you could do a sanity-check against it [TNH]
		uint8_t	num_anim_lights; // number of animated lights - animlm indexes length (the array before lightmap descriptors, indexing bits of animflags to light numbers)
		uint8_t	flow_group; // 0-no flow group, otherwise the flow group no.

		// cell's bounding sphere
		wr_coord_t	center;
		float	radius; // Only an approximation, but enough to guarantee that every point in the cell is enclosed by this sphere.
	} wr_cell_hdr_t;

	typedef struct { // SIZE: 8
		float	x;
		float   y;
	} wr_coord_2d_t;


	typedef struct { // SIZE: 8
		uint8_t	flags; // Nonzero for watered polygons
		uint8_t	count; // Polygon vertices count
		uint8_t	plane; //  plane number
		uint8_t	unk;   // seems zero
		uint16_t	tgt_cell; // target leaf for this portal...
		uint8_t	unk1[2]; // the second seems to contain some flags... 41, FF... etc...
	}  wr_polygon_t;

	typedef struct { // SIZE: 12+12+12+12 = 48
		wr_coord_t	ax_u; // U axis
		wr_coord_t	ax_v; // V axis - both directions of texture growth (e.g. U axis and V axis) - and they are not normalised! (in some way related to scale)

		int16_t		u; // txt shift u (must divide by 1024 to get float number (and I dunno why, I had to invert it too))
		int16_t		v; // txt shift v

		uint8_t		txt; // texture number (index to the texture list)
		uint8_t		origin_vtx; // the vertex index of the origin vertex - the vertex used as a reference for texturing
		uint8_t		unk[2];

		float		scale; // scale of the texture
		wr_coord_t	center;
	} wr_polygon_texturing_t;

	// a plane
	typedef struct { // SIZE: 16
		wr_coord_t	normal;
		float		d;
	} wr_plane_t;

	/** Lightmap Information struct. */
	typedef struct { // SIZE: 4+4+12 = 20
		int16_t u; // LMAP U shift probably (if, then the same approach as in the wr_face_info_t)
		int16_t v; // LMAP V shift probably

		uint16_t  lx; // this is the dimension X
		uint8_t  ly; // this is the dimension Y
		uint8_t  unk;

		uint32_t static_lmap_ptr;   // Static lightmap pointer in memory - ignored by us
		uint32_t dynamic_lmap_ptr;    // Dynamic lightmap pointer in memory - ignored by us

		uint32_t animflags; // map of animlight lightmaps present - bit 1 means yes for that light - count ones, add 1 and you get the total num of lmaps for this lmap info
	} wr_light_info_t;


	/**
	* A bsp Tree node - e.g. a leaf or a split plane
	*/
	typedef struct { // 20b - BSP node
		// As TNH says: 24 bits node number, 8 bits flags.
		// I put those into one field, as Msvc and GCC differ in bitfield packing
		uint32_t ndn_fl;
		// uint8_t flags; // 0x100 == 1 -> leaf node
		int32_t cell; // if Nonleaf, the splitting cells number
		int32_t plane; // if Nonleaf, the splitting cells plane number - e.g. read nodes[cell].plane[plane] to get the plane you want
		uint32_t front; // Or Cell index if leaf node (or END if 0xFFFFFF)
		uint32_t back;  // Not used in leaf node
	} wr_BSP_node_t;

	#define _BSP_NODENUM(a) (a.ndn_fl & 0x00FFFFFF)
	#define _BSP_FLAGS(a) (a.ndn_fl  >> 24)
#pragma pack(pop)

} // end of Opde namespace

#endif
