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
 *****************************************************************************/
 
#include "OgrePortal.h"
#include "OgrePortalFrustum.h"
#include "OgreDarkSceneNode.h"

namespace Ogre {

	Portal::Portal(DarkSceneNode* source, DarkSceneNode* target, Plane plane) {
		points = new PortalPoints();
		points->clear();
		mFrameNum = 0xFFFFF;
		mMentions = 0;
		
		mSource = source;
		mTarget = target;
		
		this->plane = plane;
	}	
		
	Portal::~Portal() {
		delete points;
	}
			
	Portal::Portal(Portal *src) {
		const PortalPoints& pnts = src->getPoints();
				
		points = new PortalPoints();
		points->clear();
				
		for (unsigned int x = 0; x < pnts.size(); x++)
			points->push_back(pnts.at(x));

		this->mTarget = src->getTarget();
		this->mSource = src->getSource();
		this->plane = src->getPlane();
		mFrameNum = src->mFrameNum;
	}
			
	void Portal::addVertex(float x, float y, float z) {
		points->push_back(Vector3(x,y,z));
	}
			
	void Portal::addVertex(Vector3 a) {
		points->push_back(a);
	}
			
	const PortalPoints& Portal::getPoints() {
		return *points;
	}
			
	int Portal::getPointCount() {
		return points->size();
	}
			
	DarkSceneNode* Portal::getTarget() {
		return mTarget;
	}
	
	DarkSceneNode* Portal::getSource() {
		return mSource;
	}
	
			
	Plane Portal::getPlane() {
		return plane;
	}
			
	void Portal::setPlane(Plane plane) {
		this->plane = plane;
	}
	
	unsigned int Portal::getOutCount(Plane &plane) {
		unsigned int idx;
		
		int negative = 0;
		
		for (idx = 0; idx < points->size(); idx++) {
			Plane::Side side = plane.getSide(points->at(idx));
			
			if (side == Plane::NEGATIVE_SIDE)
				negative++;
			
			/*switch (side) {
				case Plane::NEGATIVE_SIDE : negative++; break;
				default: ;
			}*/
		}
		
		return negative;
	}
	
	// TODO: Is this quick enough?
	bool Portal::isSeen(Camera *cam) {
		// we look if the camera planes say the Portal is outside.
		unsigned int pCount = points->size();
		
		Plane plane = cam->getFrustumPlane(FRUSTUM_PLANE_BOTTOM);
		
		if (getOutCount(plane) >= pCount )
			return false;
		
		plane = cam->getFrustumPlane(FRUSTUM_PLANE_LEFT);
		
		if (getOutCount(plane) >= pCount )
			return false;
		
		plane = cam->getFrustumPlane(FRUSTUM_PLANE_TOP);
		
		if (getOutCount(plane) >= pCount )
			return false;
		
		plane = cam->getFrustumPlane(FRUSTUM_PLANE_RIGHT);
		
		if (getOutCount(plane) >= pCount )
			return false;
		
		return true; // we are not sure, so say we see it...
	}
	
	/**
	* Refreshes the Portal's bounding volume (Sphere). Used for portal visibility speedup's.
	*/
	void Portal::refreshBoundingVolume() {
		if (points->size() == 0) {
			mCenter = Vector3(0,0,0);
			mRadius = -1;
			return;
		}
		// first get the center.
		Vector3 center(0,0,0);
		
		for (unsigned int x = 0; x < points->size(); x++)
			center += points->at(x);
				
		center /= points->size();
		
		mCenter = center;
		
		// now the maximal radius
		float radius = 0;
		
		for (unsigned int x = 0; x < points->size(); x++) {
			Vector3 vdist = points->at(x) - center;
			
			float len = vdist.squaredLength();
			
			if (len > radius)
				radius = len;
		}
		
		mRadius = sqrt(radius);
	}
	
	/** 
	* Clips the poly using a plane (we replace our vertices after success)
	* returns number of vertices in the new poly
	*/
	int Portal::clipByPlane(Plane *plane, bool &didClip) {
		int positive = 0;
		int negative = 0;
		
		//first we mark the vertices
		std::vector< Plane::Side > sides;
		
		unsigned int idx;
		
		for (idx = 0; idx < points->size(); idx++) {
			Plane::Side side = plane->getSide(points->at(idx));
			sides.push_back(side); // push the side of the vertex into the side buffer...
			
			switch (side) {
				case Plane::POSITIVE_SIDE : positive++; break;
				case Plane::NEGATIVE_SIDE : negative++; break;
				default: ;
			}
		}
		
		// Now that we have the poly's side classified, we can process it...
		
		if (negative == 0)
			return points->size(); // all the vertices were inside
		
		didClip = true;
		
		if (positive == 0) { // we clipped away the whole poly
			points->clear();
			return 0;
		}
		
		// some vertices were on one side, some on the other
		
		PortalPoints *newpnts =  new PortalPoints();
		
		long prev = points->size()-1; // the last one
		
		for (idx = 0; idx < points->size(); idx++) {
			Plane::Side side = sides[idx];
			
			if (side == Plane::POSITIVE_SIDE) { 
				if (sides[prev] == Plane::POSITIVE_SIDE) { 
					newpnts->push_back(points->at(idx));
				} else {
					// calculate a new boundry positioned vertex
					const Vector3& v1 = points->at(prev);
					const Vector3& v2 = points->at(idx);
					Vector3 dv = v2 - v1; // vector pointing from v2 to v1 (v1+dv*0=v2 *1=v1)
					
					// the dot product is there for a reason! (As I have a tendency to overlook the difference)
					float t = plane->getDistance(v2) / (plane->normal.dotProduct(dv));
					
					newpnts->push_back(v2 - (dv * t)); // a new, boundry placed vertex is inserted
					newpnts->push_back(v2);
				}
			} else { 
				if (sides[prev] == Plane::POSITIVE_SIDE) { // if we're going outside
					// calculate a new boundry positioned vertex
					const Vector3 v1 = points->at(idx);
					const Vector3 v2 = points->at(prev);
					const Vector3 dv = v2 - v1;
					
					float t = plane->getDistance(v2) / (plane->normal.dotProduct(dv));
					
					newpnts->push_back(v2 - (dv * t)); // a new, boundry placed vertex is inserted
				}
			}
			
			prev = idx;
		}
		
		if (newpnts->size() < 3) { // a degenerated Portal...
			points->clear();
			delete newpnts;
			return 0;
		}
		
		delete points;
		points = newpnts; 
		return points->size();
	}


	/**
	* Fill's the x, y parameters with the projected coordinates of the Portal (-1 to 1)
	*/
	void Portal::getScreenCoordinates(Camera *cam, const Vector3& position, Real& x, Real& y) {
		Vector3 hcsPosition = cam->getProjectionMatrix() * (cam->getViewMatrix() * position);

		// Why should we remap the x,y to 0-1 if this is only a time expensive operation?
		/*
		x = 1.0f - ((hcsPosition.x * 0.5f) + 0.5f);// 0 <= x <= 1 // left := 0,right := 1
		y = ((hcsPosition.y * 0.5f) + 0.5f);// 0 <= y <= 1 // bottom := 0,top := 1
		*/
		
		x = - hcsPosition.x; // -1 <= x <= 1
		y =   hcsPosition.y; // -1 <= y <= 1
	} 


	void Portal::refreshScreenRect(Camera *cam, PortalFrustum *frust) {
		// inverse coords to let the min/max initialize
		screenRect.top = -1e5;
		screenRect.right = -1e5;
		screenRect.left = 1e5;
		screenRect.bottom = 1e5;
		
		// Erase the actual rect
		mActualRect.top = -1e5;
		mActualRect.right = -1e5;
		mActualRect.left = 1e5;
		mActualRect.bottom = 1e5;
		
		// skip these expensive operations if we encounter a backface cull
		if (mDotProduct > 0) 
			return;

		// We have to cut the Portal using camera's frustum first, as this should solve the to screen projection problems...
		// the reason is that the coords that are at the back side or near the view point do not get projected right
		bool didc;
		Portal *onScreen = frust->clipPoly(this, didc);
		
		// If we have a non-zero cut result
		if (onScreen) {
			const PortalPoints& scr_points = onScreen->getPoints();
			
			unsigned int idx;
			
			// project all the vertices to screen space
			for (idx = 0; idx < scr_points.size(); idx++) {
				Real x, y;
				
				getScreenCoordinates(cam, scr_points.at(idx), x, y);
				
				if (screenRect.top < y)
					screenRect.top = y;
				
				if (screenRect.bottom > y)
					screenRect.bottom = y;
				
				if (screenRect.right < x)
					screenRect.right = x;
				
				if (screenRect.left > x)
					screenRect.left = x;
			}
			
			// release only if clip produced a new poly
			if (onScreen != this)
				delete onScreen;
		}
	}
	
	bool Portal::intersectByRect(Rectangle &boundry, Rectangle &target) {
		// now do the intersection
		target.top    = (screenRect.top    < boundry.top)    ? screenRect.top    : boundry.top;
		target.bottom = (screenRect.bottom > boundry.bottom) ? screenRect.bottom : boundry.bottom;
		target.right  = (screenRect.right  < boundry.right)  ? screenRect.right  : boundry.right;
		target.left   = (screenRect.left   > boundry.left)   ? screenRect.left   : boundry.left;

		if (target.top <= target.bottom)
			return false;
	
		if (target.right <= target.left)
			return false;
		
		return true;
	}
	
	/**
	* Unions the actuall'y visible rect with a new one.
	*/
	bool Portal::unionActualWithRect(Rectangle &addition) {
		bool changed = false;
		
		if (addition.left < mActualRect.left) {
			mActualRect.left = addition.left;
			changed = true;
		}
		
		if (addition.right > mActualRect.right) {
			mActualRect.right = addition.right;
			changed = true;
		}
		
		if (addition.bottom < mActualRect.bottom) {
			mActualRect.bottom = addition.bottom;
			changed = true;
		}
		
		if (addition.top > mActualRect.top) {
			mActualRect.top = addition.top;
			changed = true;
		}
		
		return changed;
	}
	
	bool Portal::getScreenBoundingRectangleIntersection(Camera *cam, unsigned int frameNum, Rectangle &boundry, Rectangle &target) {
		return intersectByRect(boundry, target);
	}
	
	void Portal::calculateDistance(Camera *cam) {
		// This is a hack, I know.... the point is that to determine the DotProduct, we would have to calculate center point - camera view vector
		// And normalise that one, then calculate the dot product of the resulting vector with Portal's normal. This would be far more expensive
		// TODO: Solution is to name the variable mPortalSide or something
		mDotProduct = -1;
		
		if (plane.getSide(cam->getDerivedPosition()) == Plane::NEGATIVE_SIDE)
			mDotProduct = 1;
	}
	
	void Portal::setPortalID(int id) {
		mPortalID = id;
	}
	
	void Portal::attach() {
		mSource->attachOutgoingPortal(this);
		mTarget->attachIncommingPortal(this);
	}
} // namespace Ogre
