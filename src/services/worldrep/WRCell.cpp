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
			mCellNum(-1), 
		        mVertices(NULL),
		        mFaceMaps(NULL),
		        mFaceInfos(NULL),
		        mPolyIndices(NULL),
		        mPlanes(NULL),
			mLoaded(false),
			mPortalsDone(false), 
			mOwner(owner), 
			mLevelGeometry(targetGeom),
		        mLights(NULL) {
		mBSPNode = NULL;

		mMaterialService = GET_SERVICE(MaterialService);
		mLightService = GET_SERVICE(LightService);
	}


	//------------------------------------------------------------------------------------
	WRCell::~WRCell() {
		if (mLoaded) {

			delete[] mVertices;
			delete[] mFaceMaps;
			delete[] mFaceInfos;

			for (int i = 0; i < mHeader.numPolygons; i++)
				delete[] mPolyIndices[i];

			delete[] mPolyIndices;

			delete[] mPlanes;
		}
	}


	//------------------------------------------------------------------------------------
	void WRCell::loadFromChunk(unsigned int _cell_num, FilePtr& chunk, int lightSize) {
		assert(!mLoaded);

		// Copy the Cell id
		mCellNum = _cell_num;

		// check the lightmap pixel size
		assert((lightSize >= 1) && (lightSize <= 2));

		// The number in the comment is used by me to check if I deallocate what I allocated...
		// load the header
		*chunk >> mHeader;

		//1. load the vertices
		mVertices = new Vector3[mHeader.numVertices];
		
		for (size_t i = 0; i < mHeader.numVertices; ++i)
			*chunk >> mVertices[i]; 
				
		//2. load the cell's polygon mapping
		mFaceMaps = new WRPolygon[mHeader.numPolygons];
		for (size_t i = 0; i < mHeader.numPolygons; ++i)
			*chunk >> mFaceMaps[i];

		//3. load the cell's texturing infos
		mFaceInfos = new WRPolygonTexturing[mHeader.numTextured];
		for (size_t i = 0; i < mHeader.numTextured; ++i)
			*chunk >> mFaceInfos[i];

		// polygon mapping struct.. len and data

		// load the polygon indices map (indices to the vertex data for this cell)
		// skip the total count of the indices
		uint32_t num_indices;
		*chunk >> num_indices;

		// 4.
		mPolyIndices = new uint8_t*[mHeader.numPolygons];

		//5. for each polygon there is
		for (int x = 0; x < mHeader.numPolygons; x++) {
			mPolyIndices[x] = new uint8_t[mFaceMaps[x].count];
			chunk->read(&(mPolyIndices[x][0]), mFaceMaps[x].count);
		}

		//6. load the planes
		mPlanes = new Ogre::Plane[mHeader.numPlanes];
		for (size_t i = 0; i < mHeader.numPlanes; ++i)
			*chunk >> mPlanes[i];
		
		// and load it's light info
		mLights = mLightService->_loadLightDefinitionsForCell(mCellNum, chunk, mHeader.numAnimLights, mHeader.numTextured, mFaceInfos);

		mLoaded = true;
	}


	//------------------------------------------------------------------------------------
	const Ogre::Plane& WRCell::getPlane(int index) {

		assert(mLoaded);

		if (index > mHeader.numPlanes)
			OPDE_EXCEPT("Plane index is out of bounds", "WRCell::getPlane");

		return mPlanes[index];
	}


	//------------------------------------------------------------------------------------
	void WRCell::insertTexturedVertex(Ogre::ManualObject *manual, int faceNum, const Vector3& coord,
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
	void WRCell::constructBspVertex(int faceNum, const Vector3& pos, const Ogre::Vector2& displacement, const std::pair<
	        Ogre::uint, Ogre::uint>& dimensions, BspVertex *vtx) {
		// Position copy
		vtx->position[0] = pos.x;
		vtx->position[1] = pos.y;
		vtx->position[2] = pos.z;

		uint pln = mFaceMaps[faceNum].plane;


		// Normal copy
		Vector3 normal(mPlanes[pln].normal.x, mPlanes[pln].normal.y, mPlanes[pln].normal.z);
		normal.normalise();

		vtx->normal[0] = normal.x;
		vtx->normal[1] = normal.y;
		vtx->normal[2] = normal.z;

		// texturing coordinates
		float tx, ty;
		// Lightmapping coordinates
		float lx, ly;


		// normalised versions of the above
		Vector3 nax_u, nax_v;

		// Texturing axes
		const Vector3& ax_u = mFaceInfos[faceNum].axisU;
		const Vector3& ax_v = mFaceInfos[faceNum].axisV;

		// Normalise the texturing axises
		nax_u = ax_u.normalisedCopy(); // Normalized version of the axis U
		nax_v = ax_v.normalisedCopy(); // -"- V

		// Lengths of the axises - contains scale projected to the axis somewhat. somehow.
		float l_ax_u = ax_u.length(); // the length of the axis U
		float l_ax_v = ax_v.length(); // -"- V

		// UV shifts
		float sh_u, sh_v;


		// The UV shifts can't simply be center vertex based! It all seems to be 0 vertex based.

		sh_u = mFaceInfos[faceNum].u / 4096.0;
		sh_v = mFaceInfos[faceNum].v / 4096.0;

		// the original vertex:
		Vector3 tmp = pos;


		// The first vertex in the poly.
		const Vector3& o_first = mVertices[mPolyIndices[faceNum][0]]; //cell->face_maps[faceNum].flags

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
		// const Vector3& center = mFaceInfos[faceNum].center;

		tmp = pos;

		Vector3 orig = tmp; // for debugging, remove...

		// The scale defines the pixel size. Nothing else does
		float scale = mFaceInfos[faceNum].scale;


		// lightmaps x,y sizes
		const WRLightInfo& li = mLights->getLightInfo(faceNum);

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
		// This method is just a hack. We need to find a proper way to detect light map on polygon alignment
		// readout from the cell info
		const Vector3& ax_u = mFaceInfos[polyNum].axisU;
		const Vector3& ax_v = mFaceInfos[polyNum].axisV;

		// Normalise those texturing axises
		Vector3 nax_u = ax_u.normalisedCopy(); // Normalized version of the axis U
		Vector3 nax_v = ax_v.normalisedCopy();

		Vector2 min_uv(1E16, 1E16);
		Vector2 max_uv(-1E16, -1E16);


		// Precalculate the lightmap displacement. (To get the resulting lmap uv to 0-1 range)
		for (int vert = 0; vert < mFaceMaps[polyNum].count; vert++) {
			// find the min and max coords in texture space
			const Vector3& vcoord = mVertices[mPolyIndices[polyNum][vert]];

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
	}


	//------------------------------------------------------------------------------------
	void WRCell::constructPortalMeshes(Ogre::SceneManager *sceneMgr) {
		// some checks on the status. These are hard mistakes
		assert(mLoaded);

		int portalStart = getFaceCount();

		for (int polyNum = portalStart; polyNum < mHeader.numTextured; polyNum++) {
			// Prepare the object's name
			StringUtil::StrStreamType modelName;
			modelName << "cell_" << mCellNum << "_portal_" << polyNum;

			// Each portal's mesh gets it's own manual object. This way we minimize the mesh attachments to hopefully minimal set
			ManualObject* manual = sceneMgr->createManualObject(modelName.str());


			// Iterate through the faces, and add all the triangles we can construct using the polygons defined
			std::pair<Ogre::uint, Ogre::uint> dimensions;


			// getMaterialName(face_infos[polyNum].txt, lightMaps[polyNum]->getAtlasIndex(), dimensions, face_maps[polyNum].flags)
			MaterialPtr mat = mMaterialService->getWRMaterialInstance(mFaceInfos[polyNum].txt,
			        -1, mFaceMaps[polyNum].flags); // mLightService->getAtlasForCellPolygon(cellNum, polyNum)

			dimensions = mMaterialService->getTextureDimensions(mFaceInfos[polyNum].txt);

			// begin inserting one polygon
			manual->begin(mat->getName());

			// temporary array of indices (for polygon triangulation)
			Vector2 displacement = calcLightmapDisplacement(polyNum);


			// Hmm. I use a SceneNode centered at the polygon's center. Otherwise I get huge radius (from 0,0,0 of the sceneNode).
			const Vector3& nodeCenter = mFaceInfos[polyNum].center;

			// insertTexturedVertex(manual, polyNum, zero, displacement, dimensions, nodeCenter);

			// for each vertex, insert into the model
			for (int vert = 0; vert < mFaceMaps[polyNum].count; vert++) {
				const Vector3& vrelative = mVertices[mPolyIndices[polyNum][vert]];

				insertTexturedVertex(manual, polyNum, vrelative, displacement, dimensions, nodeCenter);
			}

			// now feed the indexes
			for (int t = 1; t < mFaceMaps[polyNum].count - 1; t++) {
				// push back the indexes
				manual->index(0);
				manual->index(t);

				manual->index(t + 1);
			}

			manual->end();

			LOG_DEBUG("Attaching cell water portal %d geometry...", mCellNum);
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

			LOG_DEBUG("   - Attaching cell water portal %d geometry : done", mCellNum);
		}
	}


	//------------------------------------------------------------------------------------
	int WRCell::attachPortals(DarkSceneManager* smgr) {
		assert(mBSPNode);
		assert(!mPortalsDone);

		// The textured portals are both texturing source and portals. This solves the problems of that approach.
		int PortalOffset = mHeader.numPolygons - mHeader.numPortals;


		// Number of removed vertices (This number indicates the count of vertices removed due to the
		// fact that the vector to the next vertex is the same (to some degree) as from the last one to here)
		int optimized = 0;

		for (int portalNum = PortalOffset; portalNum < mHeader.numPolygons; portalNum++) {
			Ogre::Plane portalPlane;

			portalPlane = getPlane(mFaceMaps[portalNum].plane);

			Portal *portal = smgr->createPortal(smgr->getBspLeaf(mCellNum), smgr->getBspLeaf(
			        mFaceMaps[portalNum].tgtCell), portalPlane);

			for (int vert = 0; vert < mFaceMaps[portalNum].count; vert++) {
				// for each vertex of that poly
				portal->addPoint(mVertices[mPolyIndices[portalNum][vert]]);
			} // for each vertex

			// Debbuging portal order ID
			portal->setPortalID(portalNum - PortalOffset);

			// refresh the polygon bounding sphere... for clipping optimisation
			portal->refreshBoundingVolume();

			// Optimize the portal
			optimized += portal->optimize();

			// insert to the plane->(portal set) map
			std::pair<Ogre::BspNode::PlanePortalMap::iterator, bool> ptset = mPortalMap.insert(
			        Ogre::BspNode::PlanePortalMap::value_type(mFaceMaps[portalNum].plane, Ogre::PortalList()));
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
		int faceCount = mHeader.numPolygons - mHeader.numPortals;

		for (int polyNum = 0; polyNum < faceCount; polyNum++) {
			total += mFaceMaps[polyNum].count;
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
		int faceCount = getFaceCount();

		for (int polyNum = 0; polyNum < faceCount; polyNum++) {
			int pvc = mFaceMaps[polyNum].count;
			assert(pvc > 2);
			total += 3 * (pvc - 2);
		}

		return total;
	}


	//------------------------------------------------------------------------------------
	int WRCell::getFaceCount() {
		// this one is simple in the above mentioned scenario

		return mHeader.numPolygons - mHeader.numPortals;
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
		int faceCount = mHeader.numTextured;


		// Cell is re-centered with this
		// const Vector3& nodeCenter = mHeader.center;

		// Now let's iterate over the materials
		// Prepare the object's name
		StringUtil::StrStreamType modelName;
		modelName << "cell_" << mCellNum;

		// Attach the resulting object to the node with the center in the center vertex of the mesh...
		if (faceCount <= 0) {
			LOG_INFO("A geometry - less cell encountered, skipping the mesh generation");
			return;
		}

		// Step one. Map materials and polygons to iterate over
		for (int polyNum = 0; polyNum < faceCount; polyNum++) {
			std::pair<Ogre::uint, Ogre::uint> dimensions;

			MaterialPtr mat = mMaterialService->getWRMaterialInstance(mFaceInfos[polyNum].txt,
			        mLightService->getAtlasForCellPolygon(mCellNum, polyNum), mFaceMaps[polyNum].flags);


			// insert the poly index into the list of that material
			std::pair<std::map<std::string, std::vector<int> >::iterator, bool> res = matToPolys.insert(make_pair(
			        mat->getName(), std::vector<int>()));

			res.first->second.push_back(polyNum);

			dimensions = mMaterialService->getTextureDimensions(mFaceInfos[polyNum].txt);


			// Could be rather per material. But well. just for test anyway
			polyToDim.insert(make_pair(polyNum, dimensions));
		}

		std::map<std::string, std::vector<int> >::iterator it = matToPolys.begin();

		for (; it != matToPolys.end(); it++) {
			DarkFragment* frag = mLevelGeometry->createFragment(mCellNum, MaterialManager::getSingleton().getByName(
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
				uint32_t *idxmap = new uint32_t[mFaceMaps[polyNum].count];

				for (int vert = 0; vert < mFaceMaps[polyNum].count; vert++) {
					const Vector3& vrelative = mVertices[mPolyIndices[polyNum][vert]];


					// insertTexturedVertex(manual, polyNum, vrelative, displacement, dimensions, nodeCenter);
					BspVertex vtx;
					constructBspVertex(polyNum, vrelative, displacement, dimensions, &vtx);

					idxmap[vert] = frag->vertex(Vector3(vtx.position[0], vtx.position[1], vtx.position[2]), Vector3(
					        vtx.normal[0], vtx.normal[0], vtx.normal[0]), Vector2(vtx.texcoords[0], vtx.texcoords[1]),
					        Vector2(vtx.lightmap[0], vtx.lightmap[1]));
				}

				// now feed the indexes
				for (int t = 1; t < mFaceMaps[polyNum].count - 1; t++) {
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
		mBSPNode = tgtNode;
		mBSPNode->setIsLeaf(true);
		
		mBSPNode->_setCellFlags(mHeader.cellFlags);
	}


	//------------------------------------------------------------------------------------
	Ogre::BspNode* WRCell::getBspNode() {
		return mBSPNode;
	}


	//------------------------------------------------------------------------------------
	Ogre::Vector3 WRCell::getCenter() {
		Vector3 center(mHeader.center.x, mHeader.center.y, mHeader.center.z);

		return center;
	}

}
