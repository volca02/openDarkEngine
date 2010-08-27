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
#include "DarkCommon.h"
#include "File.h"
#include "FileCompat.h"

#include "Vector3.h"

// the only one which collided is wr_cell_hdr, but to be sure...
#pragma pack(push, 1)

namespace Opde {

	struct WRHeader { // SIZE: 8
		uint32_t  	unk;
		uint32_t	numCells;
		
		friend File& operator<<(File& st, const WRHeader& ch) {
			st << ch.unk << ch.numCells;
			return st;
		}
		
		friend File& operator>>(File& st, WRHeader& ch) {
			st >> ch.unk >> ch.numCells;
			return st;
		}
	};

	struct WRCellHeader { // SIZE: 31
		uint8_t	numVertices; // vertex count

		uint8_t	numPolygons;  // total number of polygons
		uint8_t	numTextured; // textured polys count
		uint8_t	numPortals; // faces that define a portal... Count this number from face_maps to get the first index of portal

		uint8_t	numPlanes; // plane count
		uint8_t	mediaType; // air == 1, water == 2 [TNH]
		uint8_t	cellFlags; // bit 6 is set in fog cells. bits 3+4 are set in doorways, probably for vision blocking  [TNH]. bit 1 - wireframe
		uint32_t	nxn; // size of the weird struct in bytes?

		uint16_t polymapSize; // this is repeated at the start of the polygon index list. you could do a sanity-check against it [TNH]
		uint8_t	numAnimLights; // number of animated lights - animlm indexes length (the array before lightmap descriptors, indexing bits of animflags to light numbers)
		uint8_t	flowGroup; // 0-no flow group, otherwise the flow group no.

		// cell's bounding sphere
		Vector3	center;
		float	radius; // Only an approximation, but enough to guarantee that every point in the cell is enclosed by this sphere.
		
		friend File& operator<<(File& st, const WRCellHeader& ch) {
			st << ch.numVertices << ch.numPolygons << ch.numTextured << ch.numPortals << ch.numPlanes
			   << ch.mediaType << ch.cellFlags << ch.nxn << ch.polymapSize << ch.numAnimLights << ch.flowGroup
			   << ch.center << ch.radius;
			return st;
		}
		
		friend File& operator>>(File& st, WRCellHeader& ch) {
			st >> ch.numVertices >> ch.numPolygons >> ch.numTextured >> ch.numPortals >> ch.numPlanes
			   >> ch.mediaType >> ch.cellFlags >> ch.nxn >> ch.polymapSize >> ch.numAnimLights >> ch.flowGroup
			   >> ch.center >> ch.radius;
			return st;
		}
	};

	struct WRPolygon { // SIZE: 8
		uint8_t	flags; // Nonzero for watered polygons
		uint8_t	count; // Polygon vertices count
		uint8_t	plane; //  plane number
		uint8_t	unk;   // seems zero
		uint16_t	tgtCell; // target leaf for this portal...
		uint8_t	unk1; // Cell motion related
		uint8_t	unk2; // Some flags... 41, FF... etc... (0x010 might be light map only render)
		
		friend File& operator<<(File& st, const WRPolygon& ch) {
			st << ch.flags << ch.count << ch.plane << ch.unk << ch.tgtCell << ch.unk1 << ch.unk2;
			return st;
		}
		
		friend File& operator>>(File& st, WRPolygon& ch) {
			st >> ch.flags >> ch.count >> ch.plane >> ch.unk >> ch.tgtCell >> ch.unk1 >> ch.unk2;
			return st;
		}
	};

	struct WRPolygonTexturing { // SIZE: 12+12+12+12 = 48
		Vector3	axisU; // U axis
		Vector3	axisV; // V axis - both directions of texture growth (e.g. U axis and V axis) - and they are not normalised! (in some way related to scale)

		int16_t		u; // txt shift u (must divide by 1024 to get float number (and I dunno why, I had to invert it too))
		int16_t		v; // txt shift v

		uint8_t		txt; // texture number (index to the texture list)
		uint8_t		originVertex; // the vertex index of the origin vertex - the vertex used as a reference for texturing
		uint16_t		unk; // something related to texture cache

		float		scale; // scale of the texture
		Vector3		center;
		
		friend File& operator<<(File& st, const WRPolygonTexturing& ch) {
			st << ch.axisU << ch.axisV << ch.u << ch.v << ch.txt << ch.originVertex << ch.unk << ch.scale << ch.center;
			return st;
		}
		
		friend File& operator>>(File& st, WRPolygonTexturing& ch) {
			st >> ch.axisU >> ch.axisV >> ch.u >> ch.v >> ch.txt >> ch.originVertex >> ch.unk >> ch.scale >> ch.center;
			return st;
		}
	};

	/** Lightmap Information struct. */
	struct WRLightInfo { // SIZE: 4+4+12 = 20
		int16_t u; // LMAP U shift probably (if, then the same approach as in the wr_face_info_t)
		int16_t v; // LMAP V shift probably

		uint16_t  lx; // this is the dimension X stored as a 16 bit value
		uint8_t  ly; // this is the dimension Y
		uint8_t  lx8; // 8bit version of lx

		uint32_t staticLmapPtr;   // Static lightmap pointer in memory - ignored by us
		uint32_t dynamicLmapPtr;    // Dynamic lightmap pointer in memory - ignored by us

		uint32_t animflags; // map of animlight lightmaps present - bit 1 means yes for that light - count ones, add 1 and you get the total num of lmaps for this lmap info
		
		friend File& operator<<(File& st, const WRLightInfo& ch) {
			st << ch.u << ch.v << ch.lx << ch.ly << ch.lx8 << ch.staticLmapPtr << ch.dynamicLmapPtr << ch.animflags;
			
			return st;
		}
		
		friend File& operator>>(File& st, WRLightInfo& ch) {
			st >> ch.u >> ch.v >> ch.lx >> ch.ly >> ch.lx8 >> ch.staticLmapPtr >> ch.dynamicLmapPtr >> ch.animflags;
			
			return st;
		}
	};


	/**
	* A bsp Tree node - e.g. a leaf or a split plane
	*/
	struct WRBSPNode { // 20b - BSP node
		// As TNH says: 24 bits node number, 8 bits flags.
		// I put those into one field, as Msvc and GCC differ in bitfield packing
		uint32_t ndn_fl;
		// uint8_t flags; // 0x100 == 1 -> leaf node
		int32_t cell; // if non-leaf, the splitting cells number
		int32_t plane; // if non-leaf, the splitting cells plane number - e.g. read nodes[cell].plane[plane] to get the plane you want
		uint32_t front; // Or Cell index if leaf node (or END if 0xFFFFFF)
		uint32_t back;  // Not used in leaf node
		
		friend File& operator<<(File& st, const WRBSPNode& ch) {
			st << ch.ndn_fl << ch.cell << ch.plane << ch.front << ch.back;
			
			return st;
		}
		
		friend File& operator>>(File& st, WRBSPNode& ch) {
			st >> ch.ndn_fl >> ch.cell >> ch.plane >> ch.front >> ch.back;
			
			return st;
		}
	};

	#define _BSP_NODENUM(a) (a.ndn_fl & 0x00FFFFFF)
	#define _BSP_FLAGS(a) (a.ndn_fl  >> 24)
#pragma pack(pop)

} // end of Opde namespace

#endif
