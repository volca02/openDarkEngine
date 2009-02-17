/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2009 openDarkEngine team
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


#include "DrawBuffer.h"
#include "DrawSheet.h"

#include <OgreMaterialManager.h>
#include <OgreHardwareBufferManager.h>
#include <OgreMaterial.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreRoot.h>


using namespace Ogre;

namespace Opde {

	/*----------------------------------------------------*/
	/*-------------------- DrawBuffer --------------------*/
	/*----------------------------------------------------*/
	DrawBuffer::DrawBuffer(const Ogre::MaterialPtr& material) :
			mMaterial(material),
			mIsDirty(false),
			mIsUpdating(false),
			mVertexData(NULL),
			mIndexData(NULL),
			mQuadCount(0),
			mRenderQueueID(Ogre::RENDER_QUEUE_OVERLAY),
			mParent(NULL) {

		setUseIdentityProjection(true);
		setUseIdentityView(true);
	};

	//------------------------------------------------------
	DrawBuffer::~DrawBuffer() {
		// destroy the material again
		// Ogre::MaterialManager::getSingleton().remove(mMaterial);
		destroyBuffers();
	};

	//------------------------------------------------------
	void DrawBuffer::addDrawOperation(DrawOperation* op) {
		mDrawOpMap.insert(std::make_pair(op->getID(), op));
		mIsDirty = true;
	};

	//------------------------------------------------------
	void DrawBuffer::removeDrawOperation(DrawOperation* op) {
		// find the draw op. mentioned, remove
		mIsDirty = true;
	};

	//------------------------------------------------------
	void DrawBuffer::queueUpdate(DrawOperation* drawOp) {
		mIsDirty = true;
	};

	//------------------------------------------------------
	void DrawBuffer::update() {
		// let's update the queue!
		// We'll set isUpdating to true - _queueDrawQuad can then be used
		mIsUpdating = true;

		// now we call all the render ops to visit this buffer
		DrawOperationMap::iterator iend = mDrawOpMap.end();

		// clear the quad list for this usage
		mQuadList.clear();

		for (DrawOperationMap::iterator it = mDrawOpMap.begin(); it != iend; ++it) {
			DrawOperation* op = it->second; 
			
			// rebuild the drawop if needed
			if (op->isDirty())
				op->rebuild();
			
			// visit!
			it->second->visitDrawBuffer(this);
		}

		mIsUpdating = false;

		// done. We now have the list full
		// sort with stable sort (to leave the order of draw quads the same draw op requested)
		std::stable_sort(mQuadList.begin(), mQuadList.end(), QuadLess());

		// Sorted. Now (re)build the buffer
		buildBuffer();

		mIsDirty = false;

	}


	//------------------------------------------------------
	void DrawBuffer::_queueDrawQuad(const DrawQuad* dq) {
		assert(mIsUpdating);

		mQuadList.push_back(dq);
	};

	//------------------------------------------------------
	void DrawBuffer::_parentChanged(DrawSheet* parent) {
		mParent = parent;
	}

	//------------------------------------------------------
	const MaterialPtr& DrawBuffer::getMaterial(void) const {
		return mMaterial;
	};

	//------------------------------------------------------
	void DrawBuffer::getRenderOperation(Ogre::RenderOperation& op) {
	        op.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;

        	op.vertexData = mVertexData;
	        op.vertexData->vertexStart = 0;
	        op.vertexData->vertexCount = mQuadList.size() * 4;

        	op.useIndexes = true;
	        op.indexData = mIndexData;
	        op.indexData->indexStart = 0;
	        op.indexData->indexCount = mQuadList.size() * 6;
	};

	//------------------------------------------------------
	void DrawBuffer::getWorldTransforms(Ogre::Matrix4* trans) const {
		// Identity
		*trans = mParent->_getParentNodeFullTransform();
	};

	//------------------------------------------------------
	const Ogre::Quaternion& DrawBuffer::getWorldOrientation(void) const {
		if (mParent)
			return mParent->getParentNode()->_getDerivedOrientation();
		else
			return Quaternion::IDENTITY;
	};

	//------------------------------------------------------
	const Ogre::Vector3& DrawBuffer::getWorldPosition(void) const {
		if (mParent)
			return mParent->getParentNode()->_getDerivedPosition();
		else
			return Vector3::ZERO;
	};

	//------------------------------------------------------
	Ogre::Real DrawBuffer::getSquaredViewDepth(const Ogre::Camera* cam) const {
		assert(mParent);
		Node* n = mParent->getParentNode();
		assert(n);
		return n->getSquaredViewDepth(cam);
	};

	//------------------------------------------------------
	const Ogre::LightList& DrawBuffer::getLights() const {
		return mParent->queryLights();
	};

	//------------------------------------------------------
	void DrawBuffer::buildBuffer() {
		// if size differs, we reallocate the buffers
		if (mQuadCount < mQuadList.size()) {
			// raise the buffer, with some padding to avoid frequent reallocations
			mQuadCount = mQuadList.size() * 2;
			destroyBuffers();
		}

		if (!mVertexData) {
			// no vertex data, let's reallocate some!
			mVertexData = new Ogre::VertexData();
			mVertexData->vertexStart = 0;
			mVertexData->vertexCount = mQuadList.size() * 4;

			Ogre::VertexDeclaration* decl = mVertexData->vertexDeclaration;
			Ogre::VertexBufferBinding* binding = mVertexData->vertexBufferBinding;

			size_t offset = 0;
			decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
			offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
			decl->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, 0);
			offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);
			decl->addElement(0, offset, Ogre::VET_COLOUR, Ogre::VES_DIFFUSE);

			mBuffer = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
					decl->getVertexSize(0), mVertexData->vertexCount, Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

			binding->setBinding(0, mBuffer);
		}

		if (!mIndexData) {
			// no index data, so let's rebuilt it.
			mIndexData = new Ogre::IndexData();
			mIndexData->indexStart = 0;
			mIndexData->indexCount = mQuadCount * 6; // quad count, so we have a reserve

			// As canvas does it - build the IBO statically, we don't need no per-update updates
			mIndexData->indexBuffer = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
					Ogre::HardwareIndexBuffer::IT_16BIT,
					mIndexData->indexCount,
					Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE, false);

			// now we'll fill the buffer with indices of triangles (0,2,1; 1,2,3)
			unsigned short* iData = reinterpret_cast<unsigned short*>(mIndexData->indexBuffer->lock(0, mIndexData->indexBuffer->getSizeInBytes(),
					Ogre::HardwareBuffer::HBL_DISCARD));

			// Inspired by Canvas. It's true we don't need to do this per frame,
			// we'll just set the mIndexData->indexCount to the proper value after building
			for (size_t iindex = 0, ivertex = 0, iquad = 0; iquad < mQuadCount; ++iquad, ivertex += 4) {
				iindex = iquad * 6;
				// tri 1
				iData[iindex++] = (unsigned short)(ivertex);
				iData[iindex++] = (unsigned short)(ivertex + 2);
				iData[iindex++] = (unsigned short)(ivertex + 1);
				// tri 2
				iData[iindex++] = (unsigned short)(ivertex + 1);
				iData[iindex++] = (unsigned short)(ivertex + 2);
				iData[iindex++] = (unsigned short)(ivertex + 3);
			}

			mIndexData->indexBuffer->unlock();
		};

		// now we'll build the vertex part - we are already sorted so we'll just need quad rewritten
		// to the vertex part
		float* buf = reinterpret_cast<float*>(mBuffer->lock(0, mQuadList.size() * mBuffer->getVertexSize() * 4, Ogre::HardwareBuffer::HBL_DISCARD));

		Ogre::RGBA* colptr;

		for (DrawQuadList::iterator it = mQuadList.begin(); it != mQuadList.end(); ++it) {
			// all the vertices
			const DrawQuad* dq = *it;

			/// Top Left corner
			*buf++ = dq->positions.topleft.x;
			*buf++ = dq->positions.topleft.y;
			*buf++ = dq->positions.topleft.z;

			*buf++ = dq->texCoords.topleft.x;
			*buf++ = dq->texCoords.topleft.y;

			colptr = reinterpret_cast<Ogre::RGBA*>(buf);
			Ogre::Root::getSingleton().convertColourValue(dq->colors.topleft, colptr);
			colptr++;
			buf = reinterpret_cast<float*>(colptr);

			/// Top right corner
			*buf++ = dq->positions.topright.x;
			*buf++ = dq->positions.topright.y;
			*buf++ = dq->positions.topright.z;

			*buf++ = dq->texCoords.topright.x;
			*buf++ = dq->texCoords.topright.y;

			colptr = reinterpret_cast<Ogre::RGBA*>(buf);
			Ogre::Root::getSingleton().convertColourValue(dq->colors.topright, colptr);
			colptr++;
			buf = reinterpret_cast<float*>(colptr);

			/// Bottom left corner
			*buf++ = dq->positions.bottomleft.x;
			*buf++ = dq->positions.bottomleft.y;
			*buf++ = dq->positions.bottomleft.z;

			*buf++ = dq->texCoords.bottomleft.x;
			*buf++ = dq->texCoords.bottomleft.y;

			colptr = reinterpret_cast<Ogre::RGBA*>(buf);
			Ogre::Root::getSingleton().convertColourValue(dq->colors.bottomleft, colptr);
			colptr++;
			buf = reinterpret_cast<float*>(colptr);

			/// Bottom right corner
			*buf++ = dq->positions.bottomright.x;
			*buf++ = dq->positions.bottomright.y;
			*buf++ = dq->positions.bottomright.z;

			*buf++ = dq->texCoords.bottomright.x;
			*buf++ = dq->texCoords.bottomright.y;

			colptr = reinterpret_cast<Ogre::RGBA*>(buf);
			Ogre::Root::getSingleton().convertColourValue(dq->colors.bottomright, colptr);
			colptr++;
			buf = reinterpret_cast<float*>(colptr);
		}

		// ibo length to the number of quads times six
		mIndexData->indexCount = mQuadList.size() * 6;

		mBuffer->unlock();
	};

	//------------------------------------------------------
	void DrawBuffer::destroyBuffers() {
		delete mVertexData;
		mVertexData = NULL;

		delete mIndexData;
		mIndexData = NULL;

		mBuffer.setNull();
	};

}

