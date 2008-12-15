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

#include "WRCell.h"
#include "OgreException.h"
#include "OgreSceneManager.h"
#include "OgreManualObject.h"
#include "OgreMaterial.h"
#include "OgreMaterialManager.h"
#include "OgreTextureManager.h"
#include "OgrePrerequisites.h"
#include "OpdeException.h"
#include "logger.h"
#include "WRCommon.h"
#include "WorldRepService.h"
#include "LightService.h"

using namespace Ogre;

namespace Opde {


	//------------------------------------------------------------------------------------
	WRCell::WRCell(WorldRepService* owner, Ogre::DarkGeometry* targetGeom) :
			cellNum(-1), 
		        vertices(NULL),
		        face_maps(NULL),
		        face_infos(NULL),
		        poly_indices(NULL),
		        planes(NULL),
			mLoaded(false),
			mPortalsDone(false), 
			mOwner(owner), 
			mLevelGeometry(targetGeom),
		        mLights(NULL) {
		bspNode = NULL;

		mMaterialService = GET_SERVICE(MaterialService);
		mLightService = GET_SERVICE(LightService);
	}


	//------------------------------------------------------------------------------------
	WRCell::~WRCell() {
		if (mLoaded) {

			delete[] vertices;
			delete[] face_maps;
			delete[] face_infos;

			for (int i = 0; i < header.num_polygons; i++)
				delete[] poly_indices[i];

			delete[] poly_indices;

			delete[] planes;
		}
	}


	//------------------------------------------------------------------------------------
	void WRCell::loadFromChunk(unsigned int _cell_num, FilePtr& chunk, int lightSize) {
		assert(!mLoaded);

		// Copy the Cell id
		cellNum = _cell_num;

		// check the lightmap pixel size
		assert((lightSize >= 1) && (lightSize <= 2));

		// The number in the comment is used by me to check if I deallocate what I allocated...
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
		wr_plane_t* wr_planes = new wr_plane_t[header.num_planes];
		chunk->read(wr_planes, sizeof(wr_plane_t) * header.num_planes);

		planes = new Ogre::Plane[header.num_planes];
		// convert the planes to the ogre format
		for (int x = 0; x < header.num_planes; x++) {
			wr_plane_t origpl = wr_planes[x];
			Ogre::Plane tplane;

			tplane.normal = Vector3(origpl.normal.x, origpl.normal.y, origpl.normal.z);
			tplane.d = origpl.d;

			planes[x] = tplane;
		}
		delete[] wr_planes;

		// and load it's light info
		mLights = mLightService->_loadLightDefinitionsForCell(cellNum, chunk, header.num_anim_lights, header.num_textured, face_infos);

		mLoaded = true;
	}


	//------------------------------------------------------------------------------------
	const Ogre::Plane& WRCell::getPlane(int index) {

		assert(mLoaded);

		if (index > header.num_planes)
			OPDE_EXCEPT("Plane index is out of bounds", "WRCell::getPlane");

		return planes[index];
	}


	//------------------------------------------------------------------------------------
	void WRCell::insertTexturedVertex(Ogre::ManualObject *manual, int faceNum, wr_coord_t coord,
	        const Ogre::Vector2& displacement, const std::pair<Ogre::uint, Ogre::uint>& dimensions, Vector3 origin) {

		BspVertex vert;

		constructBspVertex(faceNum, coord, displacement, dimensions, &vert);

		manual->position(vert.position[0] - origin.x, vert.position[1] - origin.y, vert.position[2] - origin.z);

		manual->textureCoord(vert.texcoords[0], vert.texcoords[1]);

		// !NLM!
		manual->textureCoord(vert.lightmap[0], vert.lightmap[1]);

		Vector3 normal(vert.normal[0], vert.normal[1], vert.normal[2]);

		manual->normal(normal);
	}


	//------------------------------------------------------------------------------------
	void WRCell::constructBspVertex(int faceNum, wr_coord_t pos, const Ogre::Vector2& displacement, const std::pair<
	        Ogre::uint, Ogre::uint>& dimensions, BspVertex *vtx) {
		// Position copy
		vtx->position[0] = pos.x;
		vtx->position[1] = pos.y;
		vtx->position[2] = pos.z;

		uint pln = face_maps[faceNum].plane;


		// Normal copy
		Vector3 normal(planes[pln].normal.x, planes[pln].normal.y, planes[pln].normal.z);
		normal.normalise();

		vtx->normal[0] = normal.x;
		vtx->normal[1] = normal.y;
		vtx->normal[2] = normal.z;

		// texturing coordinates
		float tx, ty;
		// Lightmapping coordinates
		float lx, ly;


		// the texturing axises
		Vector3 ax_u, ax_v;
		// normalised versions of the above
		Vector3 nax_u, nax_v;


		// Texturing axises
		wr_coord_t _axu = face_infos[faceNum].ax_u;
		wr_coord_t _axv = face_infos[faceNum].ax_v;


		// convert the vectors to the Ogre types
		ax_u = Vector3(_axu.x, _axu.y, _axu.z);
		ax_v = Vector3(_axv.x, _axv.y, _axv.z);

		// Normalise the texturing axises
		nax_u = ax_u.normalisedCopy(); // Normalized version of the axis U
		nax_v = ax_v.normalisedCopy(); // -"- V

		// Lengths of the axises - contains scale projected to the axis somewhat. somehow.
		float l_ax_u = ax_u.length(); // the length of the axis U
		float l_ax_v = ax_v.length(); // -"- V

		// UV shifts
		float sh_u, sh_v;


		// The UV shifts can't simply be center vertex based! It all seems to be 0 vertex based.

		sh_u = face_infos[faceNum].u / 4096.0;
		sh_v = face_infos[faceNum].v / 4096.0;

		// the original vertex:
		Vector3 tmp = Vector3(pos.x, pos.y, pos.z);


		// The first vertex in the poly.
		wr_coord_t& first = vertices[poly_indices[faceNum][0]]; //cell->face_maps[faceNum].flags

		// converted to Ogre Vector3
		Vector3 o_first = Vector3(first.x, first.y, first.z);


		// The texturing origin is substracted
		tmp -= o_first;

		// UV shifts apply
		tmp = tmp + ((ax_u * sh_u) + (ax_v * sh_v));

		// relative pixel sizes (float) - seems that the texture mapper scales the whole thing with this
		float rs_x = dimensions.first / 64.0;
		float rs_y = dimensions.second / 64.0;


		// Projected to the UV space.
		tx = (nax_u.dotProduct(tmp)) / (l_ax_u * rs_x); // we divide by scale to normalize
		ty = (nax_v.dotProduct(tmp)) / (l_ax_v * rs_y);

		// - write the result into the BspVertex
		vtx->texcoords[0] = tx;
		vtx->texcoords[1] = ty;

		// ----------------- LIGHTMAP COORDS --------------------
		wr_coord_t& o_center = face_infos[faceNum].center;
		Vector3 center = Vector3(o_center.x, o_center.y, o_center.z);

		tmp = Vector3(pos.x, pos.y, pos.z);

		Vector3 orig = tmp; // for debugging, remove...

		// The scale defines the pixel size. Nothing else does
		float scale = face_infos[faceNum].scale;


		// lightmaps x,y sizes
		const wr_light_info_t& li = mLights->getLightInfo(faceNum);

		unsigned int sz_x = li.lx;
		unsigned int sz_y = li.ly;

		// Shifts. These are *128 (e.g. fixed point, 6 bits represent 0-1 range) to get 0-1 shift (in pixels)
		// Weird thing about these is that they are nonsymetrical, and signed (max value 63, min about -15 or such).
		sh_u = li.u / 128;
		sh_v = li.v / 128;

		// rel. pos
		// tmp -= poly_center;
		// tmp -= center;
		// tmp -= o_first;

		// shifting by UV shifts...
		tmp -= ((nax_u * sh_u) + (nax_v * sh_v));

		// To the UV space
		lx = ((nax_u.dotProduct(tmp)) - displacement.x) / (sz_x * scale / 4.0) + 0.5;
		ly = ((nax_v.dotProduct(tmp)) - displacement.y) / (sz_y * scale / 4.0) + 0.5;

		/*
		 lx = ((nax_u.dotProduct(tmp))) / ( sz_x * scale / 4.0);
		 ly = ((nax_v.dotProduct(tmp))) / ( sz_y * scale / 4.0);
		 */

		// remap to the atlas coords and insert it to the vertex as a second texture coord...
		Vector2 lmUV = mLights->mapUV(faceNum, Vector2(lx,ly));

		vtx->lightmap[0] = lmUV.x; // lmUV.x
		vtx->lightmap[1] = lmUV.y;

	}


	//------------------------------------------------------------------------------------
	Vector2 WRCell::calcLightmapDisplacement(int polyNum) {
		// ------------- Calculate the UV center displacement (as the poly center is off)
		// readout from the cell info
		wr_coord_t _axu = face_infos[polyNum].ax_u;
		wr_coord_t _axv = face_infos[polyNum].ax_v;


		// convert the vectors to the Ogre types
		Vector3 ax_u = Vector3(_axu.x, _axu.y, _axu.z);
		Vector3 ax_v = Vector3(_axv.x, _axv.y, _axv.z);


		// Normalise those texturing axises
		Vector3 nax_u = ax_u.normalisedCopy(); // Normalized version of the axis U
		Vector3 nax_v = ax_v.normalisedCopy();

		Vector2 min_uv(1E16, 1E16);
		Vector2 max_uv(-1E16, -1E16);


		// Precalculate the lightmap displacement. (To get the resulting lmap uv to 0-1 range)
		for (int vert = 0; vert < face_maps[polyNum].count; vert++) {
			// find the min and max coords in texture space
			wr_coord_t coord = vertices[poly_indices[polyNum][vert]];

			Vector3 vcoord(coord.x, coord.y, coord.z);


			// To uv space
			Vector2 uvs(nax_u.dotProduct(vcoord), nax_v.dotProduct(vcoord));

			min_uv.x = std::min(min_uv.x, uvs.x);
			min_uv.y = std::min(min_uv.y, uvs.y);
			max_uv.x = std::max(max_uv.x, uvs.x);
			max_uv.y = std::max(max_uv.y, uvs.y);
		}

		// Now, compute displacement.
		max_uv -= min_uv;

		return min_uv + max_uv / 2;
		// return min_uv;
		/*

		 // Calculate the centroid of the polygon
		 // The zero index vertex UV space coord:
		 wr_coord_t coord = vertices[ poly_indices[polyNum][0] ];

		 Vector3 vcoord(coord.x, coord.y, coord.z);

		 // To uv space
		 Vector2 zuv(nax_u.dotProduct(vcoord), nax_v.dotProduct(vcoord));

		 Vector2 center(0,0);
		 Real parea = 0;

		 // Precalculate the lightmap displacement. (To get the resulting lmap uv to 0-1 range)
		 for (int vert = 1; vert < face_maps[polyNum].count - 1; vert++) {
		 // find the min and max coords in texture space
		 wr_coord_t coord = vertices[ poly_indices[polyNum][vert] ];

		 Vector3 vcoord(coord.x, coord.y, coord.z);

		 // To uv space
		 Vector2 uvs(nax_u.dotProduct(vcoord), nax_v.dotProduct(vcoord));

		 // The next vertex
		 coord = vertices[ poly_indices[polyNum][vert+1] ];


		 vcoord = Vector3(coord.x, coord.y, coord.z);

		 // To uv space
		 Vector2 uvsn(nax_u.dotProduct(vcoord), nax_v.dotProduct(vcoord));

		 // Calculate the centroid of the triangle
		 Vector2 centroid = (uvs + uvsn + zuv) / 3;

		 Vector2 disp1 = uvs-zuv;
		 Vector2 disp2 = uvsn-zuv;
		 Real area = disp1.x * disp2.y - disp1.y * disp2.x;

		 center += centroid * area;
		 parea += area;
		 }

		 center /= parea;

		 return center;
		 */
	}


	//------------------------------------------------------------------------------------
	void WRCell::constructPortalMeshes(Ogre::SceneManager *sceneMgr) {
		// some checks on the status. These are hard mistakes
		assert(mLoaded);

		int portalStart = header.num_polygons - header.num_portals;

		for (int polyNum = portalStart; polyNum < header.num_textured; polyNum++) {
			// Prepare the object's name
			StringUtil::StrStreamType modelName;
			modelName << "cell_" << cellNum << "_portal_" << polyNum;

			// Each portal's mesh gets it's own manual object. This way we minimize the mesh attachments to hopefully minimal set
			ManualObject* manual = sceneMgr->createManualObject(modelName.str());


			// Iterate through the faces, and add all the triangles we can construct using the polygons defined
			std::pair<Ogre::uint, Ogre::uint> dimensions;


			// getMaterialName(face_infos[polyNum].txt, lightMaps[polyNum]->getAtlasIndex(), dimensions, face_maps[polyNum].flags)
			MaterialPtr mat = mMaterialService->getWRMaterialInstance(face_infos[polyNum].txt,
			        -1, face_maps[polyNum].flags); // mLightService->getAtlasForCellPolygon(cellNum, polyNum)

			dimensions = mMaterialService->getTextureDimensions(face_infos[polyNum].txt);

			// begin inserting one polygon
			manual->begin(mat->getName());

			// temporary array of indices (for polygon triangulation)
			Vector2 displacement = calcLightmapDisplacement(polyNum);


			// Hmm. I use a SceneNode centered at the polygon's center. Otherwise I get huge radius (from 0,0,0 of the sceneNode).
			wr_coord_t polyCenter = face_infos[polyNum].center;
			Vector3 nodeCenter = Vector3(polyCenter.x, polyCenter.y, polyCenter.z);

			wr_coord_t zero;
			zero.x = 0;
			zero.y = 0;
			zero.z = 0;
			// insertTexturedVertex(manual, polyNum, zero, displacement, dimensions, nodeCenter);

			// for each vertex, insert into the model
			for (int vert = 0; vert < face_maps[polyNum].count; vert++) {
				wr_coord_t vrelative = vertices[poly_indices[polyNum][vert]];

				insertTexturedVertex(manual, polyNum, vrelative, displacement, dimensions, nodeCenter);
			}

			// now feed the indexes
			for (int t = 1; t < face_maps[polyNum].count - 1; t++) {
				// push back the indexes
				manual->index(0);
				manual->index(t);

				manual->index(t + 1);
			}

			manual->end();

			LOG_DEBUG("Attaching cell water portal %d geometry...", cellNum);
			// Attach the resulting object to the node with the center in the center vertex of the mesh...
			SceneNode* meshNode = sceneMgr->createSceneNode(modelName.str());
			meshNode->setPosition(nodeCenter);

			meshNode->attachObject(manual);

			// Attach the new scene node to the root node of the scene (this ensures no further transforms, other than we want,
			// take place, and that the geom will be visible only when needed)
			sceneMgr->getRootSceneNode()->addChild(meshNode);

			if (meshNode) {
				meshNode->needUpdate(true);
			}

			LOG_DEBUG("   - Attaching cell water portal %d geometry : done", cellNum);
		}
	}


	//------------------------------------------------------------------------------------
	int WRCell::attachPortals(DarkSceneManager* smgr) {
		assert(bspNode);
		assert(!mPortalsDone);

		// The textured portals are both texturing source and portals. This solves the problems of that approach.
		int PortalOffset = header.num_polygons - header.num_portals;


		// Number of removed vertices (This number indicates the count of vertices removed due to the
		// fact that the vector to the next vertex is the same (to some degree) as from the last one to here)
		int optimized = 0;

		for (int portalNum = PortalOffset; portalNum < header.num_polygons; portalNum++) {
			Ogre::Plane portalPlane;

			portalPlane = getPlane(face_maps[portalNum].plane);

			Portal *portal = smgr->createPortal(smgr->getBspLeaf(cellNum), smgr->getBspLeaf(
			        face_maps[portalNum].tgt_cell), portalPlane);

			for (int vert = 0; vert < face_maps[portalNum].count; vert++) {
				// for each vertex of that poly
				wr_coord_t coord = vertices[poly_indices[portalNum][vert]];

				portal->addPoint(coord.x, coord.y, coord.z);
			} // for each vertex

			// Debbuging portal order ID
			portal->setPortalID(portalNum - PortalOffset);

			// refresh the polygon bounding sphere... for clipping optimisation
			portal->refreshBoundingVolume();

			// Optimize the portal
			optimized += portal->optimize();

			// insert to the plane->(portal set) map
			std::pair<Ogre::BspNode::PlanePortalMap::iterator, bool> ptset = mPortalMap.insert(
			        Ogre::BspNode::PlanePortalMap::value_type(face_maps[portalNum].plane, Ogre::PortalList()));
			ptset.first->second.insert(portal);
		}

		mPortalsDone = true;
		return optimized;
	}


	//------------------------------------------------------------------------------------
	int WRCell::getVertexCount() {
		// The total vertex count is gonna be the count of polygon vertices (It would not give a big optimization to do otherwise - material-normal-vertex combinations)
		int total = 0;


		// Only the non-transparent geometry, please - no textured portals
		int faceCount = header.num_polygons - header.num_portals;

		for (int polyNum = 0; polyNum < faceCount; polyNum++) {
			total += face_maps[polyNum].count;
		}

		return total;
	}


	//------------------------------------------------------------------------------------
	int WRCell::getIndexCount() {
		// The total count of indices, following the same scenario as for vertices, will be counted as 3*tri_count
		// For I do triangle fans using first vertex as a base vertex, the tri_count = vertex_count - 2 , index count is triple
		// Note that this method of triangulation is only possible for convex polygons, which we do always have
		int total = 0;


		// Only the non-transparent geometry, please - no textured portals
		int faceCount = header.num_polygons - header.num_portals;

		for (int polyNum = 0; polyNum < faceCount; polyNum++) {
			int pvc = face_maps[polyNum].count;
			assert(pvc > 2);
			total += 3 * (pvc - 2);
		}

		return total;
	}


	//------------------------------------------------------------------------------------
	int WRCell::getFaceCount() {
		// this one is simple in the above mentioned scenario

		return header.num_polygons - header.num_portals;
	}


	//------------------------------------------------------------------------------------
	void WRCell::createCellGeometry() {
		// some checks on the status. These are hard mistakes
		assert(mLoaded);

		// Contains material name -> polygon list
		std::map<std::string, std::vector<int> > matToPolys;
		// polygon index to txt Dimensions
		std::map<int, std::pair<uint, uint> > polyToDim;


		// int faceCount = header.num_polygons - header.num_portals;
		int faceCount = header.num_textured;


		// Cell is recentered with this
		wr_coord_t cellCenter = header.center;
		Vector3 nodeCenter = Vector3(cellCenter.x, cellCenter.y, cellCenter.z);


		// Now let's iterate over the materials
		// Prepare the object's name
		StringUtil::StrStreamType modelName;
		modelName << "cell_" << cellNum;

		// Attach the resulting object to the node with the center in the center vertex of the mesh...
		if (faceCount <= 0) {
			LOG_INFO("A geometry - less cell encountered, skipping the mesh generation");
			return;
		}

		// Step one. Map materials and polygons to iterate over
		for (int polyNum = 0; polyNum < faceCount; polyNum++) {
			std::pair<Ogre::uint, Ogre::uint> dimensions;

			MaterialPtr mat = mMaterialService->getWRMaterialInstance(face_infos[polyNum].txt,
			        mLightService->getAtlasForCellPolygon(cellNum, polyNum), face_maps[polyNum].flags);


			// insert the poly index into the list of that material
			std::pair<std::map<std::string, std::vector<int> >::iterator, bool> res = matToPolys.insert(make_pair(
			        mat->getName(), std::vector<int>()));

			res.first->second.push_back(polyNum);

			dimensions = mMaterialService->getTextureDimensions(face_infos[polyNum].txt);


			// Could be rather per material. But well. just for test anyway
			polyToDim.insert(make_pair(polyNum, dimensions));
		}

		std::map<std::string, std::vector<int> >::iterator it = matToPolys.begin();

		for (; it != matToPolys.end(); it++) {
			DarkFragment* frag = mLevelGeometry->createFragment(cellNum, MaterialManager::getSingleton().getByName(
			        it->first));

			std::vector<int>::iterator pi = it->second.begin();


			// each of those polygons
			for (; pi != it->second.end(); pi++) {
				// Iterate through the faces, and add all the triangles we can construct using the polygons defined
				std::pair<Ogre::uint, Ogre::uint> dimensions;

				int polyNum = *pi;

				std::map<int, std::pair<uint, uint> >::iterator dimi = polyToDim.find(polyNum);

				if (dimi == polyToDim.end())
					OPDE_EXCEPT("Missing polygon texture dimensions!", "WrCell::constructCellMesh");

				dimensions = dimi->second;

				Vector2 displacement = calcLightmapDisplacement(polyNum);


				// for each vertex, insert into the model
				uint32_t *idxmap = new uint32_t[face_maps[polyNum].count];

				for (int vert = 0; vert < face_maps[polyNum].count; vert++) {
					wr_coord_t vrelative = vertices[poly_indices[polyNum][vert]];


					// insertTexturedVertex(manual, polyNum, vrelative, displacement, dimensions, nodeCenter);
					BspVertex vtx;
					constructBspVertex(polyNum, vrelative, displacement, dimensions, &vtx);

					idxmap[vert] = frag->vertex(Vector3(vtx.position[0], vtx.position[1], vtx.position[2]), Vector3(
					        vtx.normal[0], vtx.normal[0], vtx.normal[0]), Vector2(vtx.texcoords[0], vtx.texcoords[1]),
					        Vector2(vtx.lightmap[0], vtx.lightmap[1]));
				}

				// now feed the indexes
				for (int t = 1; t < face_maps[polyNum].count - 1; t++) {
					// push back the indexes
					frag->index(idxmap[0]);
					frag->index(idxmap[t + 1]);
					frag->index(idxmap[t]);
				}

				delete[] idxmap;
			}
		}
	}


	//------------------------------------------------------------------------------------
	void WRCell::setBspNode(Ogre::BspNode* tgtNode) {
		bspNode = tgtNode;
		bspNode->setIsLeaf(true);
		
		bspNode->_setCellFlags(header.cell_flags);
	}


	//------------------------------------------------------------------------------------
	Ogre::BspNode* WRCell::getBspNode() {
		return bspNode;
	}


	//------------------------------------------------------------------------------------
	Ogre::Vector3 WRCell::getCenter() {
		Vector3 center(header.center.x, header.center.y, header.center.z);

		return center;
	}

}
