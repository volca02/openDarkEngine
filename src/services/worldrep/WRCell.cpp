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


#include "WRCell.h"
#include "OgreException.h"
#include "OgreSceneManager.h"
#include "OgreManualObject.h"
#include "OgreMaterial.h"
#include "OgreMaterialManager.h"
#include "OgreTextureManager.h"
#include "OgrePrerequisites.h"

using namespace Ogre;

namespace Opde {

	//------------------------------------------------------------------------------------
	WRCell::WRCell() {
	}
	
	//------------------------------------------------------------------------------------
	WRCell::~WRCell() {
		delete[] vertices;
		delete[] face_maps;
		delete[] face_infos;
		
		for (int i = 0; i < header.num_polygons; i++)
			delete[] poly_indices[i];
		
		delete[] poly_indices;
		
		delete[] planes;
		
		delete[] anim_map;
		
		delete[] lm_infos;
		
		for (int i = 0; i < header.num_textured; i++) {
			
			for (int l = 0; l < lmcounts[i]; l++)
				delete[] lmaps[i][l];
			
			delete[] lmaps[i];
		}
		
		delete[] lmaps;
		delete[] lmcounts;
		
		delete[] obj_indices;
		
		delete[] lightMaps;
	}
	
	//------------------------------------------------------------------------------------
	void WRCell::loadFromChunk(DarkDatabaseChunk *chunk, int lightSize) {
		// The number in the comment is used by me to check if I deallocate what I allocated...
		mLightSize = lightSize;
		
		// load the header
		chunk->read(&header, sizeof(wr_cell_hdr_t));
		
		//1. load the vertices
		vertices = new wr_coord_t[header.num_vertices];
		chunk->read(vertices, sizeof(wr_coord_t) * header.num_vertices);
		
		//2. load the cell's polygon mapping
		face_maps = new wr_polygon_t[header.num_polygons];
		chunk->read(face_maps, sizeof(wr_polygon_t) * header.num_polygons);
		
		//3. load the cell's texturing infos
		face_infos = new wr_polygon_texturing_t[header.num_textured];
		chunk->read(face_infos, sizeof(wr_polygon_texturing_t) * header.num_textured);
		
		// polygon mapping struct.. len and data
	
		// load the polygon indices map (indices to the vertex data for this cell)
		// skip the total count of the indices
		uint32_t num_indices;
		chunk->read(&num_indices, sizeof(uint32_t));
		
		// 4.
		poly_indices = new uint8_t*[header.num_polygons];
		
		//5. for each polygon there is
		for (int x = 0; x < header.num_polygons; x++) {
			poly_indices[x] = new uint8_t[face_maps[x].count];
			chunk->read(&(poly_indices[x][0]), face_maps[x].count);
		}
		
		//6. load the planes
		planes = new wr_plane_t[header.num_planes];
		chunk->read(planes, sizeof(wr_plane_t) * header.num_planes);
		
		//7. anim lights map
		// load the light id's array
		anim_map = new int16_t[header.num_anim_lights];
		chunk->read(anim_map, sizeof(int16_t) * header.num_anim_lights);
		
		//8. load the lightmap descriptors
		lm_infos = new wr_light_info_t[header.num_textured];
		chunk->read(lm_infos, sizeof(wr_light_info_t) * header.num_textured);
				
		//9. load the lightmaps
		// alloc the space for all the pointers
		lmaps = new uint8_t**[header.num_textured];
		lmcounts = new uint8_t[header.num_textured];
		
		int i;
		for (i = 0; i < header.num_textured; i++) {
			// Calculate the number of lmaps
			int lmcount = countBits(lm_infos[i].animflags) + 1;
			
			lmcounts[i] = lmcount;
			//10. allocate space for all the lmap pointers (1+Anim) for actual face
			lmaps[i] = new uint8_t*[lmcount];
			
			int lmsize = lm_infos[i].lx * lm_infos[i].ly * mLightSize;
			
			// for each lmap, put the ptr to data
			int lmap;
			
			// this stores the pointer to the anim lmaps too (all of them)
			for (lmap = 0; lmap < lmcount; lmap++) {
				// 11. Read one lightmap
				lmaps[i][lmap] =  new uint8_t[lmsize];
				chunk->read(&(lmaps[i][lmap][0]), lmsize);
			}
		}
		
		// list of object lights (anim+static) affecting this cell
		// _uint32 unk;
		// chunk->read(&unk, sizeof(_uint32));
		
		chunk->read(&obj_count, sizeof(uint32_t));
		
		// 12. Light object indexes
		obj_indices = new uint16_t[obj_count];
		chunk->read(&(obj_indices[0]), sizeof(uint16_t) * obj_count);
	}
	
	//------------------------------------------------------------------------------------
	int WRCell::countBits(uint32_t src) {
		// Found this trick in some code by Sean Barrett [TNH]
		int count = src;
		count = (count & 0x55555555) + ((count >>  1) & 0x55555555); // max 2
		count = (count & 0x33333333) + ((count >>  2) & 0x33333333); // max 4
		count = (count + (count >> 4)) & 0x0f0f0f0f; // max 8 per 4, now 8 bits
		count = (count + (count >> 8)); // max 16 per 8 bits
		count = (count + (count >> 16)); // max 32 per 8 bits
		return count & 0xff;
	}
	
	//------------------------------------------------------------------------------------
	const wr_plane_t WRCell::getPlane(int index) {
		if (index > header.num_planes)
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Plane index is out of bounds", "WorldRepService::getPlane");
		
		return planes[index];
	}
	
	//------------------------------------------------------------------------------------
	String WRCell::getMaterialName(unsigned int texture, unsigned int atlasnum, std::pair< Ogre::uint, Ogre::uint > &dimensions) {
		StringUtil::StrStreamType tmp;
		
		bool isSky = (texture == SKY_TEXTURE);
		
		if  (isSky) { 
			tmp << "SkyShader";
		} else {
			tmp << "Shader" << texture << "#" << atlasnum;
		}
		std::string shaderName = tmp.str();
		
		MaterialPtr shadMat = MaterialManager::getSingleton().getByName(shaderName);
		
		// If the material was not yet constructed, we'll do so now...
		if (shadMat.isNull()) {
			if (isSky)  // Should not be here if the polygon is sky textured
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Sky material 'SkyShader' was not found (should be already created)", "WRCell::getMaterialHandle");
			
			// Texture and prototype name
			StringUtil::StrStreamType txtName;
			txtName << "@template" << texture;
			
			
			MaterialPtr origMat = MaterialManager::getSingleton().getByName(txtName.str());
			
			if (!origMat.isNull()) {
				shadMat = origMat->clone(shaderName); // Clone material - we'll be adding lightmaps
			} else {
				// OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Material "+txtName.str()+" was not found (should be already created)", "BspLevel::loadDarkLevel");
				shadMat = MaterialManager::getSingleton().create(shaderName, ResourceGroupManager::getSingleton().getWorldResourceGroupName());
				std::cerr << " * Src. material empty, not cloning " << txtName.str() << std::endl;
			}
			
			// Volca: I disable the lightmaps for now, as they cause two problems.
			// 1. They are badly shifted/scaled 
			// 2. They are causing the water texture to disappear
			// To enable those, just uncomment the following line
			/*
			StringUtil::StrStreamType lightmapName;
			lightmapName << "@lightmap" << atlasnum;
			
			Pass *shadPass = shadMat->getTechnique(0)->getPass(0);
			
			if (shadPass->getNumTextureUnitStates() <= 1) {
				// Lightmap texture is added here
				TextureUnitState* tex = shadPass->createTextureUnitState(lightmapName.str());
				
				// Blend
				tex->setColourOperation(LBO_MODULATE);
				// Use 2nd texture co-ordinate set
				tex->setTextureCoordSet(1);
				
				// Clamp
				tex->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
				
				// Switch filtering off to see lmap pixels: TFO_NONE
				tex->setTextureFiltering(TFO_ANISOTROPIC);
			} else { // There is a definition of the lightmapping pass already, we only update that definition
				TextureUnitState* tex = shadPass->getTextureUnitState(1);
				tex->setTextureName(lightmapName.str());
				tex->setTextureCoordSet(1);
			} 
			//*/
		}
		
		// Get the texture dimensions
		
		dimensions = std::make_pair((Ogre::uint)64, (Ogre::uint)64); // failback to 64x64
		
		if (shadMat->getNumTechniques() > 0) 
				if (shadMat->getTechnique(0)->getNumPasses() > 0)
					if (shadMat->getTechnique(0)->getPass(0)->getNumTextureUnitStates() > 0)
						dimensions = shadMat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureDimensions();
		
		return tmp.str();
	}
	
	//------------------------------------------------------------------------------------
	void WRCell::insertTexturedVertex(ManualObject *manual, int faceNum, wr_coord_t coord, std::pair< Ogre::uint, Ogre::uint > dimensions) {
		// New vertex...
		manual->position(coord.x, coord.y, coord.z);
		
		// texturing coordinates
		float tx, ty;
		// Lightmapping coordinates
		float lx, ly;
		
		// the texturing axises
		Vector3 ax_u, ax_v;
		// normalised versions of the above
		Vector3 nax_u, nax_v;
		
		// readout from the cell info
		wr_coord_t _axu = face_infos[faceNum].ax_u;
		wr_coord_t _axv = face_infos[faceNum].ax_v;
		
		// convert the vectors to the Ogre types
		ax_u = Vector3(_axu.x, _axu.y, _axu.z);
		ax_v = Vector3(_axv.x, _axv.y, _axv.z);
		
		// Normalise those texturing axises
		nax_u = ax_u.normalisedCopy(); // Normalized version of the axis U
		nax_v = ax_v.normalisedCopy(); // -"- V
		
		// Lengths of the axises - contains scale projected to the axis somewhat. somehow.
		float l_ax_u = ax_u.length(); // the length of the axis U
		float l_ax_v = ax_v.length(); // -"- V
		
		// UV shifts
		float sh_u, sh_v;

		// HA: The UV shifts can't simply be center vertex based! It all seems to be 0 vertex based. 
		// Not always right it is, I'm afraid...
		
		sh_u = face_infos[faceNum].u / 4096.0;  
		sh_v = face_infos[faceNum].v / 4096.0; 
		
		// the original vertex:
		Vector3 tmp = Vector3(coord.x, coord.y, coord.z);
		
		// The first vertex in the poly.
		wr_coord_t& first = vertices[ poly_indices[faceNum][0] ]; //cell->face_maps[faceNum].flags
				
		// converted to Ogre Vector3
		Vector3 o_first = Vector3(first.x, first.y, first.z);
		
		// The texturing origin is substracted		
		tmp -= o_first;
		
		// UV shifts apply
		tmp = tmp + ((ax_u * sh_u) + (ax_v * sh_v));
		
		// relative pixel sizes (float) - seems that the texture mapper scales the whole thing with this
		float rs_x = dimensions.first  / 64.0;
		float rs_y = dimensions.second / 64.0;
		
		// Projected to the UV space.
		tx = (nax_u.dotProduct(tmp)) / (l_ax_u * rs_x); // we divide by scale to normalize 
		ty = (nax_v.dotProduct(tmp)) / (l_ax_v * rs_y); 
		
		manual->textureCoord(tx, ty);
		
		// ----------------- LIGHTMAP COORDS --------------------
		// Get the facts here:
		// The lightmap is scaled: scale/4.0 is the stretch for the pixel size. 
		// This means that scale 4 will result in 1x1 (world coords) pixel size, scale 8.00 will result in 2x2 pixel size (world coords).
		// Lightmaps are (+1,+1) sized than expected to be by the polygon bounding rectangle dimensions (3->4,13->14). 
		// This is because of the rounding to the pixel sizes of the resulting buffers (IMHO).
		
		// lmaps ARE center vertex based
		wr_coord_t& o_center = face_infos[faceNum].center;
		Vector3 center = Vector3(o_center.x, o_center.y, o_center.z);
		
		tmp = Vector3(coord.x, coord.y, coord.z);
		
		Vector3 orig = tmp; // for debugging, remove...
		
		// The scale defines the pixel size. Nothing else does
		float scale = face_infos[faceNum].scale;
		
		// lightmaps x,y sizes
		unsigned int sz_x = lm_infos[faceNum].lx;
		unsigned int sz_y = lm_infos[faceNum].ly;
		
		// Shifts. These are *64 (e.g. fixed point, 5 bits represent 0-1 range) to get 0-1 shift (in pixels)
		// Weird thing about these is that they are nonsymetrical, and signed. The Center vertex
		sh_u = lm_infos[faceNum].u; 
		sh_v = lm_infos[faceNum].v;
		
		// rel. pos (to the center). The supplied center is not always The center you would expect (average X/Y/Z value of all polygon vertices)
		// but it is the base for lightmap texturing is seems.
		tmp -= center;
		
		// tmp -= o_first;
		
		sh_u /= 64;
		sh_v /= 64;
		
		
		// shifting by UV shifts...
		// tmp += ( (nax_u * sh_u) + (nax_v * sh_v) );
		
		// To the UV space
		// The ratio should be the somewhat same for both X and Y (squared pixels). Thus the repetition of the Y twice?
		// We do 2 things here - convert to UV space and normalize. Let's separate these into two steps
		
		lx = (nax_u.dotProduct(tmp)) / ( sz_x * scale / 4.0) + 0.5;  // step one. - world to UV space conversion, and scaling.
		ly = (nax_v.dotProduct(tmp)) / ( sz_y * scale / 4.0) + 0.5;
		
		// remap to the atlas coords and insert it to the vertex as a second texture coord...
		manual->textureCoord(lightMaps[faceNum]->toAtlasCoords(Vector2(lx,ly)));
		
		//memcpy(dest->normal, &cell->planes[ cell->face_maps[faceNum].plane ].normal,  sizeof(float) * 3);
		manual->normal(planes[faceNum].normal.x, planes[faceNum].normal.y, planes[faceNum].normal.z);
	}
	
	//------------------------------------------------------------------------------------
	void WRCell::constructStaticGeometry(int cellNum, SceneNode *node, SceneManager *sceneMgr, LightAtlasList* atlasList) {
		// Prepare the materials first.
		
		
		StringUtil::StrStreamType modelName;
		modelName << "cell_" << cellNum << "_geometry";	
					
		
		ManualObject* manual = sceneMgr->createManualObject(modelName.str());
		
		for (int polyNum = 0; polyNum < header.num_textured; polyNum++) {
			// Iterate through the faces, and add all the triangles we can construct using the polygons defined
			std::pair< Ogre::uint, Ogre::uint > dimensions;
			
			// begin inserting one polygon
			manual->begin(getMaterialName(face_infos[polyNum].txt, lightMaps[polyNum]->getAtlasIndex(), dimensions));
			
			// temporary array of indices (for polygon triangulation)
			int *V = new int[face_maps[polyNum].count];
			
			for (int vert = 0; vert < face_maps[polyNum].count; vert++) {
				// for each vertex of the poly
				wr_coord_t coord = vertices[ poly_indices[polyNum][vert] ];
				
				insertTexturedVertex(manual, polyNum, coord, dimensions);
				
				V[vert] = vert;
			} // for each vertex
			
			// now feed the indices array
			for (int t = 0; t < face_maps[polyNum].count - 2; t++) {
				// we take 3 consecutive points
				int u,v,w;
				u = V[t];
				v = V[t+1];
				w = V[t+2];

				// chop out the vertex index, it is allready mapped.
				V[t+1] = u;

				// push back the indexes
				manual->index(u);
				manual->index(v);
				manual->index(w);
			}
			
			delete[] V;
					
			manual->end();
		}
		
		node->attachObject(manual);
	}
	
	//------------------------------------------------------------------------------------
	void WRCell::attachPortals(std::vector< DarkSceneNode *> nodeList, DarkSceneNode *thisNode) {
		// The textured portals are both texturing source and portals. This solves the problems of that approach.
		int PortalOffset = header.num_polygons - header.num_portals;
		
		for (int portalNum = PortalOffset; portalNum < header.num_polygons; portalNum++) {
			// There is probably a complication in the normal of the plane. I'll let the polygon class handle that in some method
			Ogre::Plane portalPlane;
			wr_plane_t wrPlane = planes[face_maps[portalNum].plane];
			
			portalPlane.normal = Vector3(wrPlane.normal.x, wrPlane.normal.y, wrPlane.normal.z);
			portalPlane.d = wrPlane.d;
			
			Portal *portal = new Portal(thisNode, nodeList[face_maps[portalNum].tgt_cell], portalPlane);
			
			for (int vert = 0; vert < face_maps[portalNum].count; vert++) {
				// for each vertex of that poly
				wr_coord_t coord = vertices[ poly_indices[portalNum][vert] ];
				
				portal->addVertex(coord.x, coord.y, coord.z);
			} // for each vertex
			
			// Debbuging portal order ID
			portal->setPortalID(portalNum - PortalOffset);
			
			// refresh the polygon bounding sphere... for clipping optimisation
			portal->refreshBoundingVolume();
			
			// Attach the portal to the scene Nodes
			portal->attach();
		}
	}
	
	//------------------------------------------------------------------------------------
	void WRCell::atlasLightMaps(LightAtlasList* atlasList) {
		int ver = mLightSize - 1;
		// Array of lmap references
		lightMaps = new LightMap*[header.num_textured];
		
		for (int face = 0; face < header.num_textured; face++) {
			AtlasInfo info;
			LightMap* lmap = atlasList->addLightmap(ver, face_infos[face].txt, (char *)lmaps[face][0], 
					lm_infos[face].lx,
					lm_infos[face].ly, info);
			
			lightMaps[face] = lmap;
			
			// Let's iterate through the animated lmaps
			// we have anim_map (array of light id's), cell->header->anim_lights contains the count of them.
			int bit_idx = 1, lmap_order = 1;
			
			for (int anim_l = 0; anim_l < header.num_anim_lights; anim_l++) {
				if ((lm_infos[face].animflags & bit_idx) > 0) {
					// There is a anim lmap for this light and face
					lmpixel *converted = LightMap::convert((char *)lmaps[face][lmap_order], 
								lm_infos[face].lx,
								lm_infos[face].ly, 
								ver);
					
					
					lmap->AddSwitchableLightmap(anim_map[anim_l], converted);
					
					lmap_order++;
				}
				
				bit_idx <<= 1;
			}
			
		} // for each face
	}

}
