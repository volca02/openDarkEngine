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

#include <OgreException.h>
#include <OgreManualObject.h>
#include <OgreMaterial.h>
#include <OgreMaterialManager.h>
#include <OgrePrerequisites.h>
#include <OgreSceneManager.h>
#include <OgreTextureManager.h>

#include "LightsForCell.h"
#include "OpdeException.h"
#include "OpdeServiceManager.h"
#include "WRCommon.h"
#include "WorldRepService.h"
#include "light/LightService.h"
#include "logger.h"
#include "material/MaterialService.h"

#include "DarkBspNode.h"
#include "DarkGeometry.h"
#include "DarkPortal.h"
#include "DarkSceneManager.h"

namespace Opde {

//------------------------------------------------------------------------------------
WRCell::WRCell()
    : mCellNum(-1),
      mVertices(),
      mFaceMaps(),
      mFaceInfos(),
      mPolyIndices(NULL),
      mPlanes(),
      mLoaded(false),
      mPortalsDone(false),
      mLights()
{
    mBSPNode = NULL;

    mMaterialService = GET_SERVICE(MaterialService);
    mLightService = GET_SERVICE(LightService);
}

//------------------------------------------------------------------------------------
WRCell::~WRCell() {
    if (mLoaded) {

        mVertices.clear();
        mFaceMaps.clear();
        mFaceInfos.clear();

        for (int i = 0; i < mHeader.numPolygons; i++)
            delete[] mPolyIndices[i];

        delete[] mPolyIndices;

        mPlanes.clear();
    }
}

//------------------------------------------------------------------------------------
void WRCell::loadFromChunk(unsigned int _cell_num, FilePtr &chunk,
                           int lightSize) {
    assert(!mLoaded);

    // Copy the Cell id
    mCellNum = _cell_num;

    // check the lightmap pixel size
    assert((lightSize >= 1) && (lightSize <= 2));

    // The number in the comment is used by me to check if I deallocate what I
    // allocated... load the header
    *chunk >> mHeader;

    // 1. load the vertices
    mVertices.resize(mHeader.numVertices);
//    for (auto &vertex : mVertices) *chunk >> vertex;
    for (size_t i = 0; i < mHeader.numVertices; ++i)
        *chunk >> mVertices[i];

    // 2. load the cell's polygon mapping
    mFaceMaps.resize(mHeader.numPolygons);
    for (size_t i = 0; i < mHeader.numPolygons; ++i)
        *chunk >> mFaceMaps[i];

    // 3. load the cell's texturing infos
    mFaceInfos.resize(mHeader.numTextured);
    for (size_t i = 0; i < mHeader.numTextured; ++i)
        *chunk >> mFaceInfos[i];

    // polygon mapping struct.. len and data

    // load the polygon indices map (indices to the vertex data for this cell)
    // skip the total count of the indices
    uint32_t num_indices;
    *chunk >> num_indices;

    // 4.
    mPolyIndices = new uint8_t *[mHeader.numPolygons];

    // 5. for each polygon there is
    for (int x = 0; x < mHeader.numPolygons; x++) {
        mPolyIndices[x] = new uint8_t[mFaceMaps[x].count];
        chunk->read(&(mPolyIndices[x][0]), mFaceMaps[x].count);
    }

    // 6. load the planes
    mPlanes.resize(mHeader.numPlanes);
    for (size_t i = 0; i < mHeader.numPlanes; ++i)
        *chunk >> mPlanes[i];

    // and load it's light info
    mLights.reset(new LightsForCell(chunk, mHeader.numAnimLights,
                                    mHeader.numTextured, lightSize,
                                    mFaceInfos));

    mLoaded = true;
}

//------------------------------------------------------------------------------------
const Ogre::Plane &WRCell::getPlane(int index) {

    assert(mLoaded);

    if (index > mHeader.numPlanes)
        OPDE_EXCEPT("Plane index is out of bounds", "WRCell::getPlane");

    return mPlanes[index];
}

//------------------------------------------------------------------------------------
void WRCell::insertTexturedVertex(
    Ogre::ManualObject *manual, int faceNum, const Vector3 &coord,
    const std::pair<Ogre::uint, Ogre::uint> &dimensions, Vector3 origin) {

    uint pln = mFaceMaps[faceNum].plane;

    // Normal copy
    Vector3 normal = mPlanes[pln].normal;
    normal.normalise();

    // texturing coordinates
    float tx, ty;

    // normalised versions of the texturing axes
    Vector3 nax_u, nax_v;

    // Texturing axes
    const Vector3 &ax_u = mFaceInfos[faceNum].axisU;
    const Vector3 &ax_v = mFaceInfos[faceNum].axisV;

    // Lengths of the axes - contains scale projected to the axis somewhat.
    // somehow.
    float mag2_u = ax_u.squaredLength(); // the length of the axis U
    float mag2_v = ax_v.squaredLength(); // -"- V

    // UV shifts
    float sh_u, sh_v;

    // dot. p of the texturing axes
    float dotp = ax_u.dotProduct(ax_v);

    // The UV shifts can't simply be center vertex based! It all seems to be 0
    // vertex based.
    sh_u = mFaceInfos[faceNum].u / 4096.0;
    sh_v = mFaceInfos[faceNum].v / 4096.0;

    // The texturing origin is substracted
    Vector3 tmp =
        coord -
        mVertices[mPolyIndices[faceNum][mFaceInfos[faceNum].originVertex]];

    // relative pixel sizes (float) - seems that the texture mapper scales the
    // whole thing with this
    float rs_x = dimensions.first / 64.0;
    float rs_y = dimensions.second / 64.0;

    if (dotp == 0.0f) {
        // Projected to the UV space.
        tx = (ax_u.dotProduct(tmp) / mag2_u + sh_u) /
             rs_x; // we divide by scale to normalize
        ty = (ax_v.dotProduct(tmp) / mag2_v + sh_v) / rs_y;
    } else {
        // calc the coefficients
        float corr = 1.0f / (mag2_u * mag2_v - dotp * dotp);
        float cu = corr * mag2_v;
        float cv = corr * mag2_u;
        float cross = corr * dotp;

        Vector2 pr(ax_u.dotProduct(tmp), ax_v.dotProduct(tmp));

        tx = (pr.x * cu - pr.y * cross + sh_u) / rs_x;
        ty = (pr.y * cv - pr.x * cross + sh_v) / rs_y;
    }

    manual->position(coord - origin);
    manual->textureCoord(tx, ty);
    manual->normal(normal);
}

//------------------------------------------------------------------------------------
void WRCell::constructPortalMeshes(Ogre::SceneManager *sceneMgr) {
    // some checks on the status. These are hard mistakes
    assert(mLoaded);

    int portalStart = getFaceCount();

    for (int polyNum = portalStart; polyNum < mHeader.numTextured; polyNum++) {
        // Prepare the object's name
        Ogre::StringStream modelName;
        modelName << "cell_" << mCellNum << "_portal_" << polyNum;

        // Each portal's mesh gets it's own manual object. This way we minimize
        // the mesh attachments to hopefully minimal set
        Ogre::ManualObject *manual = sceneMgr->createManualObject(modelName.str());

        // Iterate through the faces, and add all the triangles we can construct
        // using the polygons defined
        std::pair<Ogre::uint, Ogre::uint> dimensions;

        // getMaterialName(face_infos[polyNum].txt,
        // lightMaps[polyNum]->getAtlasIndex(), dimensions,
        // face_maps[polyNum].flags)
        Ogre::MaterialPtr mat = mMaterialService->getWRMaterialInstance(
            mFaceInfos[polyNum].txt, -1, mFaceMaps[polyNum].flags); // mLightService->getAtlasForCellPolygon(cellNum,
                                                                    // polyNum)

        dimensions =
            mMaterialService->getTextureDimensions(mFaceInfos[polyNum].txt);

        // begin inserting one polygon
        manual->begin(mat->getName());

        // Hmm. I use a SceneNode centered at the polygon's center. Otherwise I
        // get huge radius (from 0,0,0 of the sceneNode).
        const Vector3 &nodeCenter = mFaceInfos[polyNum].center;

        // insertTexturedVertex(manual, polyNum, zero, displacement, dimensions,
        // nodeCenter);

        // for each vertex, insert into the model
        for (int vert = 0; vert < mFaceMaps[polyNum].count; vert++) {
            const Vector3 &vrelative = mVertices[mPolyIndices[polyNum][vert]];

            insertTexturedVertex(manual, polyNum, vrelative, dimensions,
                                 nodeCenter);
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
        // Attach the resulting object to the node with the center in the center
        // vertex of the mesh...
        Ogre::SceneNode *meshNode = sceneMgr->createSceneNode(modelName.str());
        meshNode->setPosition(nodeCenter);

        meshNode->attachObject(manual);

        // Attach the new scene node to the root node of the scene (this ensures
        // no further transforms, other than we want, take place, and that the
        // geom will be visible only when needed)
        sceneMgr->getRootSceneNode()->addChild(meshNode);

        if (meshNode) {
            meshNode->needUpdate(true);
        }

        LOG_DEBUG("   - Attaching cell water portal %d geometry : done",
                  mCellNum);
    }
}

//------------------------------------------------------------------------------------
int WRCell::attachPortals(Ogre::DarkSceneManager *smgr) {
    assert(mBSPNode);
    assert(!mPortalsDone);

    // The textured portals are both texturing source and portals. This solves
    // the problems of that approach.
    int PortalOffset = mHeader.numPolygons - mHeader.numPortals;

    // Number of removed vertices (This number indicates the count of vertices
    // removed due to the fact that the vector to the next vertex is the same
    // (to some degree) as from the last one to here)
    int optimized = 0;

    for (int portalNum = PortalOffset; portalNum < mHeader.numPolygons;
         portalNum++) {
        Ogre::Plane portalPlane;

        portalPlane = getPlane(mFaceMaps[portalNum].plane);

        Ogre::Portal *portal = smgr->createPortal(
                smgr->getBspLeaf(mCellNum),
                smgr->getBspLeaf(mFaceMaps[portalNum].tgtCell), portalPlane);

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
        std::pair<Ogre::BspNode::PlanePortalMap::iterator, bool> ptset =
            mPortalMap.insert(Ogre::BspNode::PlanePortalMap::value_type(
                mFaceMaps[portalNum].plane, Ogre::PortalList()));
        ptset.first->second.insert(portal);
    }

    mPortalsDone = true;
    return optimized;
}

//------------------------------------------------------------------------------------
int WRCell::getVertexCount() {
    // The total vertex count is gonna be the count of polygon vertices (It
    // would not give a big optimization to do otherwise -
    // material-normal-vertex combinations)
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
    // The total count of indices, following the same scenario as for vertices,
    // will be counted as 3*tri_count For I do triangle fans using first vertex
    // as a base vertex, the tri_count = vertex_count - 2 , index count is
    // triple Note that this method of triangulation is only possible for convex
    // polygons, which we do always have
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
float findWrap(float x) {
    if (x >= 0) {
        return -64.0f * (int)(x / 64.0f);
    } else {
        return -64.0f * (-1 + (int)(x / 64.0f));
    }
}

//------------------------------------------------------------------------------------
void WRCell::findLightmapShifts(Vector2 &tgt, Vector2 origin) {
    tgt.x = findWrap(origin.x);
    tgt.y = findWrap(origin.y);
}

//------------------------------------------------------------------------------------
void WRCell::createCellGeometry(Ogre::DarkGeometry *levelGeometry) {
    // some checks on the status. These are hard mistakes
    assert(mLoaded);

    // Contains material name -> polygon list
    std::map<std::string, std::vector<int>> matToPolys;
    // polygon index to txt Dimensions
    std::map<int, std::pair<uint, uint>> polyToDim;

    // int faceCount = header.num_polygons - header.num_portals;
    int faceCount = mHeader.numTextured;

    // Cell is re-centered with this
    // const Vector3& nodeCenter = mHeader.center;

    // Now let's iterate over the materials
    // Prepare the object's name
    Ogre::StringStream modelName;
    modelName << "cell_" << mCellNum;

    // Attach the resulting object to the node with the center in the center
    // vertex of the mesh...
    if (faceCount <= 0) {
        LOG_INFO(
            "A geometry - less cell encountered, skipping the mesh generation");
        return;
    }

    // Step one. Map materials and polygons to iterate over
    for (int polyNum = 0; polyNum < faceCount; polyNum++) {
        std::pair<Ogre::uint, Ogre::uint> dimensions;

        Ogre::MaterialPtr mat = mMaterialService->getWRMaterialInstance(
            mFaceInfos[polyNum].txt,
            mLightService->getAtlasForCellPolygon(mCellNum, polyNum),
            mFaceMaps[polyNum].flags);

        // insert the poly index into the list of that material
        std::pair<std::map<std::string, std::vector<int>>::iterator, bool> res =
            matToPolys.insert(make_pair(mat->getName(), std::vector<int>()));

        res.first->second.push_back(polyNum);

        dimensions =
            mMaterialService->getTextureDimensions(mFaceInfos[polyNum].txt);

        // Could be rather per material. But well. just for test anyway
        polyToDim.insert(make_pair(polyNum, dimensions));
    }

    std::map<std::string, std::vector<int>>::iterator it = matToPolys.begin();

    for (; it != matToPolys.end(); it++) {
        Ogre::DarkFragment *frag = levelGeometry->createFragment(
            mCellNum, Ogre::MaterialManager::getSingleton().getByName(it->first));

        std::vector<int>::iterator pi = it->second.begin();

        // each of those polygons
        for (; pi != it->second.end(); pi++) {
            // Iterate through the faces, and add all the triangles we can
            // construct using the polygons defined
            std::pair<Ogre::uint, Ogre::uint> dimensions;

            int polyNum = *pi;

            std::map<int, std::pair<uint, uint>>::iterator dimi =
                polyToDim.find(polyNum);

            if (dimi == polyToDim.end())
                OPDE_EXCEPT("Missing polygon texture dimensions!",
                            "WrCell::constructCellMesh");

            dimensions = dimi->second;

            uint32_t *idxmap = new uint32_t[mFaceMaps[polyNum].count];

            if (mFaceMaps[polyNum].count > 32) {
                // Just log error and continue
                LOG_ERROR("WRCell: Cell %d[%d]: Polygon with %d>32 vertices "
                          "encountered, skipping",
                          mCellNum, polyNum, mFaceMaps[polyNum].count);
                continue;
            }

            // pre-calculate the texturing coordinates
            // we need to do this because lightmap UV shifts are a pain in the
            // butt
            Vector2 uv_txt[32]; // 32 is the maximum vertex count
            Vector2 uv_light[32];

            // base vertex - texturing origin
            Vector3 origin =
                mVertices[mPolyIndices[polyNum][mFaceInfos[polyNum].originVertex]];

            // plane id of the polygon
            unsigned int pln = mFaceMaps[polyNum].plane;

            // Normal of the polygon
            const Vector3 &normal = mPlanes[pln].normal;
            // normal.normalise();

            // Texturing axes
            const Vector3 &ax_u = mFaceInfos[polyNum].axisU;
            const Vector3 &ax_v = mFaceInfos[polyNum].axisV;

            // Lengths of the axes squared
            float mag2_u = ax_u.squaredLength();
            float mag2_v = ax_v.squaredLength();

            // dot product of the texturing axes. Two texturing calculations are
            // used based on the outcome
            float dotp = ax_u.dotProduct(ax_v);

            // UV shifts in pixels
            float sh_u = mFaceInfos[polyNum].u / 4096.0;
            float sh_v = mFaceInfos[polyNum].v / 4096.0;

            // relative pixel sizes (float) - seems that the texture mapper
            // scales the whole thing with this
            float rs_x = dimensions.first / 64.0;
            float rs_y = dimensions.second / 64.0;

            const WRLightInfo &li = mLights->getLightInfo(polyNum);

            // lightmap uv shift in pixels. added a half of the pixel to be
            // centered in corners
            float lsh_u = ((0.5f - li.u) + mFaceInfos[polyNum].u / 1024.0f);
            float lsh_v = ((0.5f - li.v) + mFaceInfos[polyNum].v / 1024.0f);

            // are the texturing axes orthogonal?
            if (dotp == 0.0f) {
                // first pass. Calculate the UV texturing coordinates
                for (int vert = 0; vert < mFaceMaps[polyNum].count; vert++) {
                    // Rather straightforward
                    const Vector3 &vrelative =
                        mVertices[mPolyIndices[polyNum][vert]] - origin;

                    Vector2 projected(ax_u.dotProduct(vrelative) / mag2_u,
                                      ax_v.dotProduct(vrelative) / mag2_v);

                    // Finalised texturing coordinates (in 0-1 range already)
                    uv_txt[vert].x = (projected.x + sh_u) / rs_x;
                    uv_txt[vert].y = (projected.y + sh_v) / rs_y;

                    // lightmapping coordinates - pretty close to the texturing
                    // ones (the old renderer had the pixels aligned for surface
                    // cache purposes) These values are in lightmap pixels
                    uv_light[vert].x = (4.0f * projected.x + lsh_u);
                    uv_light[vert].y = (4.0f * projected.y + lsh_v);
                }
            } else { // texture axes not orthogonal. A slightly more complicated
                     // case
                // the texturing outcomes are mixed between axes based on the
                // dotProduct - the projection axes influence each other common
                // denominator
                float corr = 1.0f / (mag2_u * mag2_v - dotp * dotp);

                // projection coefficients
                float cu = corr * mag2_v;
                float cv = corr * mag2_u;
                float cross = corr * dotp;

                // first pass. Calculate the UV texturing coordinates
                for (int vert = 0; vert < mFaceMaps[polyNum].count; vert++) {
                    // Rather straightforward
                    const Vector3 &vrelative =
                        mVertices[mPolyIndices[polyNum][vert]] - origin;

                    Vector2 pr(ax_u.dotProduct(vrelative),
                               ax_v.dotProduct(vrelative));

                    Vector2 projected(pr.x * cu - pr.y * cross,
                                      pr.y * cv - pr.x * cross);

                    // Finalised texturing coordinates (in 0-1 range already)
                    uv_txt[vert].x = (projected.x + sh_u) /
                                     rs_x; // we divide by scale to normalize
                    uv_txt[vert].y = (projected.y + sh_v) / rs_y;

                    // lightmapping coordinates - pretty close to the texturing
                    // ones (the old renderer had the pixels aligned for surface
                    // cache purposes) These values are in lightmap pixels
                    uv_light[vert].x = (4.0f * projected.x + lsh_u);
                    uv_light[vert].y = (4.0f * projected.y + lsh_v);
                }
            }

            // Intermezzo. Unwrapping lightmap U/V
            /*
            EXPLANATION: The original dark wraps the u/v shifts of the lightmaps
            - i.e. there are some crazy values for lightmap uv shifts, as the
            surface cache in the original implementation worked solely with
            64x64 bitmaps for surface cache, so some shifts targetted out of
            0-64 range in the resulting uv. To compensate, we use these cycles
            to find out how to shift the whole polygon to 0-64 and then move all
            the values accordingly.
            */
            Vector2 fv = uv_light[0]; // first vertext

            for (int vert = 1; vert < mFaceMaps[polyNum].count; vert++) {
                fv.x = std::min(fv.x, uv_light[vert].x);
                fv.y = std::min(fv.y, uv_light[vert].y);
            }

            Vector2 lmsh;
            findLightmapShifts(lmsh, fv);

            // second pass, insert into the fragment
            for (int vert = 0; vert < mFaceMaps[polyNum].count; vert++) {
                // Shift into 0-64.0f
                uv_light[vert] += lmsh;

                // normalise the lightmap into 0-1 range
                Vector2 uvl = uv_light[vert];
                uvl.x /= li.lx;
                uvl.y /= li.ly;

                idxmap[vert] =
                    frag->vertex(mVertices[mPolyIndices[polyNum][vert]], normal,
                                 uv_txt[vert], mLights->mapUV(polyNum, uvl));
            }

            // now feed the indices
            for (int t = 1; t < mFaceMaps[polyNum].count - 1; t++) {
                frag->index(idxmap[0]);
                frag->index(idxmap[t + 1]);
                frag->index(idxmap[t]);
            }

            delete[] idxmap;
        }
    }
}

//------------------------------------------------------------------------------------
void WRCell::setBspNode(Ogre::BspNode *tgtNode) {
    mBSPNode = tgtNode;
    mBSPNode->setIsLeaf(true);

    mBSPNode->_setCellFlags(mHeader.cellFlags);
}

//------------------------------------------------------------------------------------
Ogre::BspNode *WRCell::getBspNode() { return mBSPNode; }

//------------------------------------------------------------------------------------
Ogre::Vector3 WRCell::getCenter() {
    Vector3 center(mHeader.center.x, mHeader.center.y, mHeader.center.z);

    return center;
}

} // namespace Opde
