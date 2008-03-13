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
 
#include <OgreHardwareBufferManager.h>
#include <OgreDefaultHardwareBufferManager.h>
#include <OgreSimpleRenderable.h>

#include "DarkPortal.h"
#include "DarkPortalFrustum.h"
#include "DarkSceneManager.h"
#include "DarkBspNode.h"

namespace Ogre {
	#define POSITION_BINDING 0
	
	const PortalRect PortalRect::EMPTY = PortalRect(INF, -INF, INF, -INF);

	// Some default values for screen
	const PortalRect PortalRect::SCREEN = PortalRect(0, 1024, 0, 768);
	
	// ---------------------------------------------------------------------------------
	std::ostream& operator<< (std::ostream& o, PortalRect& r) {
		o << "RECT [top:" << r.top << ", left:" << r.left << ", bottom: " << r.bottom << ", right:" << r.right << "]";
		return o;
	}
	
	// ---------------------------------------------------------------------------------
	// ----------------- Portal Class implementation -----------------------------------
	// ---------------------------------------------------------------------------------
	
	// The static window resolution
	int Portal::mScreenWidth2 = 512;
	int Portal::mScreenHeight2 = 384;
	
	// ---------------------------------------------------------------------------------	
	Portal::Portal(BspNode* source, BspNode* target, Plane plane) : ConvexPolygon(plane) {
		mFrameNum = 0xFFFFF;
		mMentions = 0;
		
		mSource = source;
		mTarget = target;
		
		mMovableObject = NULL;
		
		// Setup the render op. for portal debug display.
		// This should be conditional to the DEBUG builds to stop eating precious memory
		// (10-30000 portals * few 100 bytes of class data is not as much though)
		// mRenderOp.vertexData = NULL;
	}	
		
	// ---------------------------------------------------------------------------------
	Portal::~Portal() {
		/*if (mRenderOp.vertexData)
			delete mRenderOp.vertexData;*/
			
		detach();
	}
			
	// ---------------------------------------------------------------------------------
	Portal::Portal(const Portal& src) : ConvexPolygon(src) {
		this->mTarget = src.getTarget();
		this->mSource = src.getSource();
	}
		
	// ---------------------------------------------------------------------------------
	BspNode* Portal::getTarget() const {
		return mTarget;
	}
	
	// ---------------------------------------------------------------------------------
	BspNode* Portal::getSource() const {
		return mSource;
	}
	
	// ---------------------------------------------------------------------------------
	void Portal::refreshPortalRenderable() {
		// delete the previous
/*		if (mRenderOp.vertexData)
			delete mRenderOp.vertexData;
		
		mRenderOp.vertexData = new VertexData();

        	mRenderOp.indexData = 0;
		mRenderOp.vertexData->vertexCount = mPoints.size() + 1;
		mRenderOp.vertexData->vertexStart = 0; 
		mRenderOp.operationType = RenderOperation::OT_LINE_LIST; 
		mRenderOp.useIndexes = false; 

		VertexDeclaration* decl = mRenderOp.vertexData->vertexDeclaration;
		VertexBufferBinding* bind = mRenderOp.vertexData->vertexBufferBinding;
	
		decl->addElement(POSITION_BINDING, 0, VET_FLOAT3, VES_POSITION);
	
	
		HardwareVertexBufferSharedPtr vbuf = 
			HardwareBufferManager::getSingleton().createVertexBuffer(
			decl->getVertexSize(POSITION_BINDING),
			mRenderOp.vertexData->vertexCount,
			HardwareBuffer::HBU_STATIC_WRITE_ONLY);

        	// Bind buffer
        	bind->setBinding(POSITION_BINDING, vbuf);

        	// set basic white material
        	this->setMaterial("BaseWhiteNoLighting");
		
        	float* pPos = static_cast<float*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
	
		// Throw in the vertices (no. 0 goes twice - 1. and last)
		for (unsigned int x = 0; x <= mPoints.size(); x++) {
			Vector3 v = mPoints.at(x % mPoints.size());
			(*pPos++) = v.x;
			(*pPos++) = v.y;
			(*pPos++) = v.z;
		}
		
		vbuf->unlock();
		*/
	}
	
	
	// ---------------------------------------------------------------------------------
	void Portal::refreshBoundingVolume() {
		if (mPoints.size() == 0) {
			mCenter = Vector3(0,0,0);
			mRadius = -1;
			return;
		}
		// first get the center.
		Vector3 center(0,0,0);
		
		for (unsigned int x = 0; x < mPoints.size(); x++)
			center += mPoints[x];
				
		center /= mPoints.size();
		
		mCenter = center;
		
		// now the maximal radius
		float radius = 0;
		
		for (unsigned int x = 0; x < mPoints.size(); x++) {
			Vector3 vdist = mPoints[x] - center;
			
			float len = vdist.squaredLength();
			
			if (len > radius)
				radius = len;
		}
		
		mRadius = sqrt(radius);
		
		// Refresh the portal renderable too.
		refreshPortalRenderable();
	}
	
	// ---------------------------------------------------------------------------------
	void Portal::setPortalID(int id) {
		mPortalID = id;
	}
	
	// ---------------------------------------------------------------------------------
	void Portal::attach() {
		mSource->attachOutgoingPortal(this);
		mTarget->attachIncommingPortal(this);
	}
	
	// ---------------------------------------------------------------------------------
	void Portal::detach() {
		mSource->detachPortal(this);
		mTarget->detachPortal(this);
	}
	
    	
	// ---------------------------------------------------------------------------------
	void Portal::getWorldTransforms( Matrix4* xform ) const {
		// return identity matrix to prevent parent transforms
        	*xform = Matrix4::IDENTITY;
    	}
	
	// ---------------------------------------------------------------------------------
    	const Quaternion& Portal::getWorldOrientation(void) const {
        	return Quaternion::IDENTITY;
    	}
    	
	// ---------------------------------------------------------------------------------
    	const Vector3& Portal::getWorldPosition(void) const {
    		return Vector3::ZERO;
    	}
	
	// ---------------------------------------------------------------------------------
	Real Portal::getSquaredViewDepth(const Camera* cam) const {
		Vector3 dist = cam->getDerivedPosition() - mCenter;

		return dist.squaredLength();
	}
	
	// ---------------------------------------------------------------------------------
	Real Portal::getBoundingRadius(void) const {
		return mRadius;
	}
} // namespace Ogre
