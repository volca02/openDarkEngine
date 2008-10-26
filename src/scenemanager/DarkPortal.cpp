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

// Helping 'infinity' value for PortalRect coords
#define INF 100000
#define F_INF 1E37

namespace Ogre {
	#define POSITION_BINDING 0
	
	const PortalRect PortalRect::EMPTY = PortalRect(INF, -INF, INF, -INF, F_INF);

	// Some default values for screen
	const PortalRect PortalRect::SCREEN = PortalRect(0, 1024, 0, 768);

	// The static window resolution
	int PortalRect::sScreenWidth2 = 512;
	int PortalRect::sScreenHeight2 = 384;

	
	// ---------------------------------------------------------------------------------
	std::ostream& operator<< (std::ostream& o, PortalRect& r) {
		o << "RECT [top:" << r.top << ", left:" << r.left << ", bottom: " << r.bottom << ", right:" << r.right << "]";
		return o;
	}
	
	// ---------------------------------------------------------------------------------
	// ----------------- Portal Class implementation -----------------------------------
	// ---------------------------------------------------------------------------------
	
	
	// ---------------------------------------------------------------------------------	
	Portal::Portal(BspNode* source, BspNode* target, Plane plane) : 
			ConvexPolygon(plane), 
			mScreenRect(PortalRect::EMPTY),
			mActualRect(PortalRect::EMPTY) {
				
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
		
		unsigned int pointcount = mPoints.size();
		if (pointcount == 0) {
			mCenter = Vector3(0,0,0);
			mRadius = -1;
			return;
		}
		// first get the center.
		Vector3 center(0,0,0);		
		
		for (unsigned int x = 0; x < pointcount; x++)
			center += mPoints[x];
				
		center /= pointcount;
		
		mCenter = center;
		
		// now the maximal radius
		float radius = 0;
		
		for (unsigned int x = 0; x < pointcount; x++) {
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
	
	// ---------------------------------------------------------------------------------
	bool Portal::refreshScreenRect(const Camera *cam, const Matrix4& toScreen, const Plane &cutp) {
		// modified version of the ConvexPolygon::clipByPlane which does two things at once:
		// cuts by the specified plane, and projects the result to screen (then, updates the rect to contain this point)
		
		mScreenRect = PortalRect::EMPTY;
		mActualRect = PortalRect::EMPTY;
		
		// Backface cull. The portal won't be culled if a vector camera-vertex dotproduct normal will be greater than 0
		Vector3 camToV0 = mPoints[0] - cam->getDerivedPosition(); 
				
		float dotp = camToV0.dotProduct(mPlane.normal);
		
		mPortalCull = (dotp > 0);
		
		// skip these expensive operations if we encounter a backface cull
		if (mPortalCull)
			return false;

		// portal points plane cutting and to-screen proj.
		int positive = 0;
		int negative = 0;
		
		unsigned int pointcount = mPoints.size();
		
		if (pointcount == 0)
		    return 0;
		
		//first we mark the vertices
		Plane::Side *sides = new Plane::Side[pointcount];
		
		unsigned int idx;
		
		PolygonPoints::const_iterator it = mPoints.begin();
		PolygonPoints::const_iterator end = mPoints.end();
		
		for (idx = 0; it != end; ++it, ++idx) {
			Plane::Side side = cutp.getSide(*it);
			sides[idx] = side; // push the side of the vertex into the side buffer...
			
			switch (side) {
				case Plane::POSITIVE_SIDE : positive++; break;
				case Plane::NEGATIVE_SIDE : negative++; break;
				default: ;
			}
		}
		
		// Now that we have the poly's side classified, we can process it...
		if (positive == 0) { 
			// we clipped away the whole portal. No need to cut
			delete[] sides;
			return false;
		}
		
		// some vertices were on one side, some on the other
		long prev = pointcount - 1; // the last one
		
		for (idx = 0; idx < pointcount; idx++) {
			const Plane::Side side = sides[idx];
			
			if (side == Plane::POSITIVE_SIDE) { 
				if (sides[prev] == Plane::POSITIVE_SIDE) { 
					mScreenRect.enlargeToContain(toScreen * mPoints.at(idx));
				} else {
					// calculate a new boundry positioned vertex
					const Vector3& v1 = mPoints.at(prev);
					const Vector3& v2 = mPoints.at(idx);
					Vector3 dv = v2 - v1; // vector pointing from v2 to v1 (v1+dv*0=v2 *1=v1)
					
					// the dot product is there for a reason! (As I have a tendency to overlook the difference)
					float t = cutp.getDistance(v2) / (cutp.normal.dotProduct(dv));
					
					mScreenRect.enlargeToContain(toScreen * (v2 - (dv * t))); // a new, boundry placed vertex is inserted
					mScreenRect.enlargeToContain(toScreen * v2);
				}
			} else { 
				if (sides[prev] == Plane::POSITIVE_SIDE) { // if we're going outside
					// calculate a new boundry positioned vertex
					const Vector3 v1 = mPoints[idx];
					const Vector3 v2 = mPoints[prev];
					const Vector3 dv = v2 - v1;
					
					float t = cutp.getDistance(v2) / (cutp.normal.dotProduct(dv));
					
					mScreenRect.enlargeToContain(toScreen * (v2 - (dv * t))); // a new, boundry placed vertex is inserted
				}
			}
			
			prev = idx;
		}
		
		delete[] sides;
		return true;
	}
} // namespace Ogre
