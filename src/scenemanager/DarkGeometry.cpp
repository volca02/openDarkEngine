/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 * http://www.gnu.org/copyleft/lesser.txt.
 *
 *
 *	$Id$
 *
 *****************************************************************************/

#include "DarkGeometry.h"
#include "OgreRenderQueue.h"
#include "OgreRenderOperation.h"
#include "OgreVector2.h"
#include "OgreHardwareBufferManager.h"
#include "OgreHardwareIndexBuffer.h"
#include "OgreHardwareVertexBuffer.h"

#include "DarkCamera.h"
#include "DarkBspNode.h"
#include "DarkLight.h"

#define THRESHOLD_FOR_16_BIT_IDX 64000

namespace Ogre {
	// Helper comparator for DarkVertexDefinition
	bool operator==(const DarkFragmentBuilder::VertexDefinition &a, const DarkFragmentBuilder::VertexDefinition b) {
        return ((a.pos_idx == b.pos_idx) && (a.norm_idx == b.norm_idx) && (a.txt0_idx == b.txt0_idx) && (a.txt1_idx == b.txt1_idx));
    }

	// -----------------------------------------------------------------------
	// ------------------------- DarkGeometry --------------------------------
	// -----------------------------------------------------------------------
	DarkGeometry::DarkGeometry(const String& name, uint8 defaultRenderQueueID)  : mName(name), mDefaultRenderQueueID(defaultRenderQueueID), mBuilt(false) {

	}

	// -----------------------------------------------------------------
	DarkGeometry::~DarkGeometry() {
		// dispose all the geometries
		DarkSubGeometryMap::iterator it = mSubGeometryMap.begin();
		DarkSubGeometryMap::iterator end = mSubGeometryMap.end();

		mVisibleSubGeometries.clear();

		while (it != end) {
			DarkSubGeometry* sg = it->second; it++;

			delete sg;
		}

		mSubGeometryMap.clear();
	}

	// -----------------------------------------------------------------
	void DarkGeometry::setCellCount(size_t cellCount) {
		mCellCount = cellCount;
	}

	// -----------------------------------------------------------------
	DarkSubGeometry * DarkGeometry::getSubGeometryForMaterial(const MaterialPtr& mat, bool createIfNotFound) {
		DarkSubGeometryMap::iterator it = mSubGeometryMap.find(mat->getName());

		if (it != mSubGeometryMap.end()) {
			return it->second;
		} else {
			if (createIfNotFound) {
				DarkSubGeometry *newg = new DarkSubGeometry(mat, mCellCount, mDefaultRenderQueueID, mLightList);
				mSubGeometryMap.insert(std::make_pair(mat->getName(), newg));

				return newg;
			} else {
				return NULL;
			}
		}
	}

	// -----------------------------------------------------------------
	void DarkGeometry::queueForRendering(RenderQueue *queue) {
		assert(mBuilt);

		DarkSubGeometryList::iterator it = mVisibleSubGeometries.begin();
		DarkSubGeometryList::iterator end = mVisibleSubGeometries.end();

		while (it != end) {
			DarkSubGeometry* sg = *it++;

			queue->addRenderable(sg, sg->getRenderQueueID());
		}
	}

	// -----------------------------------------------------------------
	void DarkGeometry::updateFromCamera(const DarkCamera *cam) {
		assert(mBuilt);

		// 1. populate the list of dynamic lights
		std::set<Light*> slights;

		const BspNodeList& nodeList = cam->_getVisibleNodes();

		BspNodeList::const_iterator nit = nodeList.begin();
		BspNodeList::const_iterator endit = nodeList.end();

		mLightList.clear();

		for ( ; nit != endit; ++nit) {
			BspNode* node = *nit;
			/// add dynamic lights from the leaf to our light list
			BspNode::LightIterator lit = node->dynamicLightsBegin();
			BspNode::LightIterator lend = node->dynamicLightsEnd();

			while (lit != lend) {
				Light* l = *lit++;

				// if it is new to the set, insert into list
				if (slights.insert(l).second)
					mLightList.push_back(l);
			}
		}

		// 2. build the geometry
		// Call all the sub geoms to update ibufs based on cam's visibility
		DarkSubGeometryMap::iterator it = mSubGeometryMap.begin();
		DarkSubGeometryMap::iterator end = mSubGeometryMap.end();

		mVisibleSubGeometries.clear();

		while (it != end) {
			DarkSubGeometry* sg = it->second; it++;

			int indices = sg->updateFromCamera(cam);

			if (indices) {
				mVisibleSubGeometries.push_back(sg);
			}
		}
	}

	// -----------------------------------------------------------------
	void DarkGeometry::build(void) {
		// call all the sub-geometries to build
		DarkSubGeometryMap::iterator it = mSubGeometryMap.begin();
		DarkSubGeometryMap::iterator end = mSubGeometryMap.end();

		mVisibleSubGeometries.clear();

		for (;it != end; ++it) {
			DarkSubGeometry* sg = it->second;

			sg->build();
		}

		mBuilt = true;
	}

	// -----------------------------------------------------------------
	DarkFragment* DarkGeometry::createFragment(size_t cellID, const MaterialPtr& mat) {
		DarkSubGeometry* subg = getSubGeometryForMaterial(mat, true);

		if (!subg)
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Cannot create a new cell fragment!", "DarkGeometry::createFragment");

		// TODO: Some sort of acceleration struct (cell list -> material list) would be nice

		return subg->createFragment(cellID);
	}

	// -----------------------------------------------------------------
	DarkFragment* DarkGeometry::getFragment(size_t cellID, const MaterialPtr& mat) {
		DarkSubGeometry* subg = getSubGeometryForMaterial(mat, false);

		if (!subg)
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Cannot find a subgeometry for material " + mat->getName(), "DarkGeometry::createFragment");

		return subg->getFragment(cellID);
	}

	// --------------------------------------------------------------------------
	// ------------------------- DarkSubGeometry --------------------------------
	// --------------------------------------------------------------------------
	DarkSubGeometry::DarkSubGeometry(const MaterialPtr& material, size_t cellCount, uint8 renderQueueID, LightList& centralLightList) :
				m16BitIndices(false),
				mMaterial(material),
				mRenderQueueID(renderQueueID),
				mBuilt(false),
				mCellCount(cellCount),
				mLightList(centralLightList) {

		assert(mCellCount > 0);

		// Vertex data. For now, those are per-subgeom. Can be changed later (I'm trying to get 16-bit indices if possible)
		mRenderOp.vertexData = new VertexData();

		mRenderOp.indexData = new IndexData();

		mRenderOp.indexData->indexStart = 0;
		mRenderOp.indexData->indexCount = 0;

		// Will be filled later, by the filler
		// mRenderOp.indexData->indexBuffer = NULL;

		mRenderOp.operationType = RenderOperation::OT_TRIANGLE_LIST;
		mRenderOp.useIndexes = true;

		mFragmentList = new DarkFragment*[mCellCount];

		// sane default
		memset(mFragmentList, 0, mCellCount * sizeof(DarkFragment*));

		mAllocationList = NULL;
		mAllocationEnd = NULL;
	}

	// -----------------------------------------------------------------
	DarkSubGeometry::~DarkSubGeometry() {
		delete mRenderOp.indexData;
		mRenderOp.indexData = NULL;

		delete mRenderOp.vertexData;
		mRenderOp.vertexData = NULL;

		// delete all the fragments
		for (size_t s = 0; s < mCellCount; ++s)
			delete mFragmentList[s];

		delete[] mFragmentList;

		// get rid of the rest of allocations
		DarkBufferAllocation* aloc = mAllocationList;

		while (aloc) {
			DarkBufferAllocation* cur = aloc;
			aloc = aloc->next;
			delete cur;
		}
	}

	// -----------------------------------------------------------------
	size_t	DarkSubGeometry::updateFromCamera(const DarkCamera* cam) {
		assert(mBuilt);

		// first, clear the IBUF
		HardwareIndexBufferSharedPtr ibuf = mRenderOp.indexData->indexBuffer;

		const BspNodeList& nodeList = cam->_getVisibleNodes();

		BspNodeList::const_iterator nit = nodeList.begin();
		BspNodeList::const_iterator endit = nodeList.end();

		uint32 indices = 0;

		// clear the affecting lights for our geometry (normally only dynamic lights are considered here)
		mLightList.clear();

		// those fragments that are visible will get into the ibuf

		if (m16BitIndices) {
			uint16 *pidx = static_cast<uint16*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));

			for ( ; nit != endit; ++nit) {
				uint16 size = 0;
				DarkFragment* frag = mFragmentList[(*nit)->getLeafID()];

				if (frag) {
					size = frag->cacheIndices(pidx, m16BitIndices);
					pidx += size;
				}

				indices += size;
			}
		} else {
			uint32 *pidx = static_cast<unsigned int*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));

			for ( ; nit != endit; ++nit) {
				uint32 size = 0;
				DarkFragment* frag = mFragmentList[(*nit)->getLeafID()];

				if (frag) {
					size = frag->cacheIndices(pidx, m16BitIndices);
					pidx += size;
				}

				indices += size;
			}
		}
		// unlock the ibuf
		ibuf->unlock();

		mRenderOp.indexData->indexStart = 0;
		mRenderOp.indexData->indexCount = indices;

		return indices;
	}

	// -----------------------------------------------------------------
	void DarkSubGeometry::build(void) {
		assert(!mBuilt);

		// do the needed steps... - call the fragments to build up the VBO and init index lists
		// First, we estimate the vertex (and index) count by asking all the fragments
		mVtxCount = 0;
		mIdxCount = 0;

		for (size_t s = 0; s < mCellCount; ++s) {
			DarkFragment* frag = mFragmentList[s];
			if (frag) {
				mVtxCount += frag->getVertexCount();
				mIdxCount += frag->getIndexCount();
			}
		}

		// Build the alloc. list
		mAllocationList = new DarkBufferAllocation;

		mAllocationList->last = NULL;
		mAllocationList->next = NULL;

		mAllocationList->pos = 0;
		mAllocationList->size = mVtxCount * sizeof(DarkVertex);
		mAllocationList->free = true;

		mAllocationEnd = mAllocationList;

		// now, we initialize all the buffers (TODO: put in some surplus for TXT changes)
		// mRenderOp.vertexData->vertexBufferBinding
		m16BitIndices = false;

		if (mIdxCount < THRESHOLD_FOR_16_BIT_IDX) {
			m16BitIndices = true;

			mRenderOp.indexData->indexBuffer = HardwareBufferManager::getSingleton()
				.createIndexBuffer(
					HardwareIndexBuffer::IT_16BIT,
					mIdxCount,
					HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE, false);
		} else {
			// For now, we only use 32BIT. TODO: estimate the possibility to move to 16bit
			mRenderOp.indexData->indexBuffer = HardwareBufferManager::getSingleton()
				.createIndexBuffer(
					HardwareIndexBuffer::IT_32BIT,
					mIdxCount,
					HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE, false);
		}
		VertexDeclaration* decl = mRenderOp.vertexData->vertexDeclaration;


		size_t offset = 0;
		decl->addElement(0, offset, VET_FLOAT3, VES_POSITION);
        offset += VertexElement::getTypeSize(VET_FLOAT3);
        decl->addElement(0, offset, VET_FLOAT3, VES_NORMAL);
        offset += VertexElement::getTypeSize(VET_FLOAT3);
        decl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
        offset += VertexElement::getTypeSize(VET_FLOAT2);
        decl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 1);

        // 12 + 12 + 8 + 8 = 40 bytes per Vertex
        HardwareVertexBufferSharedPtr vbuf =  HardwareBufferManager::getSingleton()
            .createVertexBuffer(
                sizeof(DarkVertex),
                mVtxCount,
                HardwareBuffer::HBU_STATIC_WRITE_ONLY);

		mVertexBuffer = vbuf;

		mRenderOp.vertexData->vertexBufferBinding->setBinding(0, vbuf);
        // Set other data
        mRenderOp.vertexData->vertexStart = 0;
        mRenderOp.vertexData->vertexCount = mVtxCount;

   		// now we can proceed to build the SG's
		for (size_t s = 0; s < mCellCount; ++s) {
			DarkFragment* frag = mFragmentList[s];
			if (frag) {
				frag->build();
			}
		}

		mBuilt = true;
	}

	// -----------------------------------------------------------------
	DarkFragment* DarkSubGeometry::createFragment(size_t cellID) {
		if (mFragmentList[cellID] != NULL)
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Fragment already defined for the given cell and material combination", "DarkSubGeometry::createFragment");

		if (cellID >= mCellCount)
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Cell id beyond the specified maximum", "DarkSubGeometry::createFragment");

		DarkFragment* frag = new DarkFragment(this);

		mFragmentList[cellID] = frag;

		return frag;
	}

	// -----------------------------------------------------------------
	DarkFragment* DarkSubGeometry::getFragment(size_t cellID) {
		if (cellID >= mCellCount)
			return NULL; // let the caller cope

		return mFragmentList[cellID];
	}

	// -----------------------------------------------------------------
	DarkBufferAllocation* DarkSubGeometry::allocateVBOSpace(size_t size) {
		// iterate through our list, find the first free space with a size >= requested
		DarkBufferAllocation* pos = mAllocationList, *cur = NULL;

		assert(pos);

		while (pos) {
			cur = pos;
			pos = pos->next;

			if ((cur->size >= size) && cur->free) {
				cur->free = false;

				// if there is any space left, split
				if (cur->size > size)
				{
					DarkBufferAllocation* rest = new DarkBufferAllocation;
					rest->size = cur->size - size;
					rest->free = true;
					rest->pos = cur->pos + size;
					rest->last = cur;
					rest->next = pos;

					// move the end pointer if needed
					if (cur == mAllocationEnd)
						mAllocationEnd = rest;

					// fixup the next on current also
					cur->next = rest;
					cur->size = size;
				}

				return cur;
			}
		}

		// if we're here, we ran out of memory for the buf. We will grow at the end
		OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Cannot allocate more space in VBO!", "DarkSubGeometry::allocateVBOSpace");
		return NULL;

		// TODO: Should have a grow ibuf method, thus saving us a trouble when implementing SG reskining
	}

	// -----------------------------------------------------------------
	void DarkSubGeometry::freeVBOSpace(DarkBufferAllocation* alloc) {
		if (alloc == NULL)
			return;

		// try to merge with prev and/or next
		DarkBufferAllocation* next = alloc->next;
		DarkBufferAllocation* last = alloc->last;

		// there, the region is free now
		alloc->free = true;

		// see if we can join with last...
		if (last) {
			if (last->free) {
				last->next = next;
				last->size += alloc->size;

				// last should skip alloc, as we're freeing it
				if (next)
					next->last = last;
			}

			// there. Merged with last.

			// is the end pointing at alloc? If so, correction has to be made
			if (mAllocationEnd == alloc)
				mAllocationEnd = last;

			// get rid of the allocated buf
			delete alloc;
			alloc = NULL;
		}

		// see if we can join with next
		if (next) {
			// will take care of the rest
			if (next->free)
				freeVBOSpace(next);
		}
	}

	// -----------------------------------------------------------------
	DarkVertex* DarkSubGeometry::lock(DarkBufferAllocation* alloc, HardwareBuffer::LockOptions lockOptions) {
		// lock the buffer according to the params
		assert(alloc);
		assert(!alloc->free);

		return static_cast<DarkVertex*>(mVertexBuffer->lock(alloc->pos, alloc->size, lockOptions));
	}

	// -----------------------------------------------------------------
	void DarkSubGeometry::unlock(void) {
		mVertexBuffer->unlock();
	}

	// -----------------------------------------------------------------
	const LightList & DarkSubGeometry::getLights(void) const {
		return mLightList;
	}

	// -----------------------------------------------------------------
	const Vector3 & DarkSubGeometry::getWorldPosition(void) const {
		return Vector3::ZERO;
	}

	// -----------------------------------------------------------------
	const Quaternion & DarkSubGeometry::getWorldOrientation(void) const {
		return Quaternion::IDENTITY;
	}

	// -----------------------------------------------------------------
	void DarkSubGeometry::getWorldTransforms(Matrix4 *xform) const {
		// TODO: Is this copy op. mandatory?
		*xform = Matrix4::IDENTITY;
	}

	// -----------------------------------------------------------------
	Real DarkSubGeometry::getSquaredViewDepth(const Camera *cam) const {
		return 0;
	}

	// -----------------------------------------------------------------
	void DarkSubGeometry::getRenderOperation(RenderOperation &op) {
		// TODO
		op = mRenderOp;
	}

	// -----------------------------------------------------------------
	const MaterialPtr & DarkSubGeometry::getMaterial(void) const {
		return mMaterial;
	}

	// -----------------------------------------------------------------------
	// ------------------------- DarkFragment --------------------------------
	// -----------------------------------------------------------------------
	DarkFragment::DarkFragment(DarkSubGeometry* owner) : mOwner(owner), mCurrent(NULL), mBuilt(false), mIdxCount(0), mIndexList(NULL)  {
		mBuilder = new DarkFragmentBuilder();
	}


	// -----------------------------------------------------------------
	DarkFragment::~DarkFragment(void) {
		mOwner->freeVBOSpace(mCurrent);
		mCurrent = NULL;

		delete mBuilder;
		mBuilder = NULL;

		delete[] mIndexList;
	}

	// -----------------------------------------------------------------
	void DarkFragment::move(DarkSubGeometry* newOwner)	{
		// TODO: Impl
	}

	// -----------------------------------------------------------------
	void DarkFragment::build(void)	{
		assert(!mBuilt);

		// build
		mVtxCount = mBuilder->getVertexCount();

		mCurrent = mOwner->allocateVBOSpace(mVtxCount * sizeof(DarkVertex));

		mIdxCount = mBuilder->getIndexCount();

		mIndexList = new uint32[mIdxCount];

		// lock the target (just to be sure)
		DarkVertex* tgt = mOwner->lock(mCurrent, HardwareBuffer::HBL_NORMAL);

		// transfer the vertices
		mBuilder->build(tgt, mIndexList);

		// unlock again
		mOwner->unlock();

		mBuilt = true;

		// free the builder
		delete mBuilder;
		mBuilder = NULL;
	}

	// -----------------------------------------------------------------
	size_t DarkFragment::vertex(const Vector3& pos, const Vector3& normal, const Vector2& txt1, const Vector2& txt2)	{
		assert(!mBuilt);
		assert(mBuilder);

		return mBuilder->vertex(pos, normal, txt1, txt2);
	}

	// -----------------------------------------------------------------
	void DarkFragment::index(size_t idx) {
		assert(!mBuilt);
		assert(mBuilder);

		mBuilder->index(idx);
	}

	// -----------------------------------------------------------------
	size_t DarkFragment::cacheIndices(void* bufPtr, bool use16Bit) {
		// TODO: Member variable
		uint32 pos3 = mCurrent->pos / sizeof(DarkVertex);

		if (use16Bit) {
			uint16* buf = reinterpret_cast<uint16*>(bufPtr);

			for (uint16 i = 0; i < mIdxCount; ++i) {
				*(buf++) = static_cast<uint16>(mIndexList[i]) + static_cast<uint16>(pos3);
			}
		} else {
			uint32* buf = reinterpret_cast<uint32*>(bufPtr);

			for (uint32 i = 0; i < mIdxCount; ++i) {
				*(buf++) = mIndexList[i] + pos3;
			}
		}
		return mIdxCount;
	}

	// -----------------------------------------------------------------
	uint32 DarkFragment::getVertexCount(void) {
		if (mBuilt) {
			return mVtxCount;
		} else
			return mBuilder->getVertexCount();
	}

	// -----------------------------------------------------------------
	uint32 DarkFragment::getIndexCount(void) {
		if (mBuilt) {
			return mIdxCount;
		} else
			return mBuilder->getIndexCount();
	}


	// ------------------------------------------------------------------------------
	// ------------------------- DarkFragmentBuilder --------------------------------
	// ------------------------------------------------------------------------------
	size_t DarkFragmentBuilder::indexTxtCoord(TxtCoordList& list, const Vector2& coord) {
		TxtCoordList::iterator it = list.begin();

		size_t idx = 0;

		for ( ; it != list.end(); ++it, ++idx ) {
			if (coord == (*it))
				return idx;
		}

		list.push_back(coord);
		return idx;
	}

	// -----------------------------------------------------------------
	size_t DarkFragmentBuilder::indexCoord(CoordList& list, const Vector3& coord) {
		CoordList::iterator it = list.begin();

		size_t idx = 0;

		for (;it != list.end();++it, ++idx) {
			if (*it == coord)
				return idx;
		}

		list.push_back(coord);
		return idx;
	}

	// -----------------------------------------------------------------
	uint32 DarkFragmentBuilder::getIndexCount(void) {
		return mIndexQueue.size();
	}

	// -----------------------------------------------------------------
	uint32 DarkFragmentBuilder::getVertexCount(void) {
		return mVertexQueue.size();
	}

	// -----------------------------------------------------------------
	void DarkFragmentBuilder::build(DarkVertex* vdest, uint32* ilist) {
		// transfer all the indices and vertices
		VertexDefinitionQueue::iterator vit = mVertexQueue.begin();

		for ( ; vit != mVertexQueue.end(); ++vit ) {
			// position
			Vector3 v = mVertices[vit->pos_idx];
			vdest->pos[0] = v.x;
			vdest->pos[1] = v.y;
			vdest->pos[2] = v.z;

			// normal
			v = mVertices[vit->norm_idx];
			vdest->norm[0] = v.x;
			vdest->norm[1] = v.y;
			vdest->norm[2] = v.z;

			// tx coord 1
			Vector2 tx = mTxtCoords0[vit->txt0_idx];
			vdest->txtcoord[0] = tx.x;
			vdest->txtcoord[1] = tx.y;

			// tx coord 2 (LMAP)
			tx = mTxtCoords1[vit->txt1_idx];
			vdest->lmcoord[0] = tx.x;
			vdest->lmcoord[1] = tx.y;

			vdest++;
		}

		// now transfer the indices
		IndexQueue::iterator iit = mIndexQueue.begin();

		for ( ; iit != mIndexQueue.end(); ++iit) { *(ilist++) = *iit; };
	}

	// -----------------------------------------------------------------
	void DarkFragmentBuilder::index(size_t index) {
		// index out of range? I think not!
		assert(index < mVertexQueue.size());

		mIndexQueue.push_back(index);
	}

	// -----------------------------------------------------------------
	size_t DarkFragmentBuilder::vertex(const Vector3& pos, const Vector3& norm, const Vector2& txt0, const Vector2& txt1) {
		VertexDefinition def;

		def.pos_idx = indexCoord(mVertices, pos);
		def.norm_idx = indexCoord(mNormals, norm);
		def.txt0_idx = indexTxtCoord(mTxtCoords0, txt0);
		def.txt1_idx = indexTxtCoord(mTxtCoords1, txt1);

		// now search for a definition, or insert a new one
		VertexDefinitionQueue::iterator it = mVertexQueue.begin();

		size_t idx = 0;

		for ( ; it != mVertexQueue.end(); ++it, ++idx) {
			if (def == *it)
				return idx;
		}

		mVertexQueue.push_back(def);
		return idx;
	}


};
