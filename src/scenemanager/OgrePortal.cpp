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
#include "OgreBspNode.h"

// #define debug

// I have a feeling that this is slightly speedier than std::max / std::min calls
// Not that much to be celebrated though
#define MAX(a,b) ((a>b)?a:b)
#define MIN(a,b) ((a<b)?a:b)

// This Angle defines the tolerance in comparing the normalised edge directions (testing whether those are equal)
#define EQUALITY_ANGLE 0.0000000001

namespace Ogre {

	// Helping 'infinity' macro for PortalRect coords
	#define INF 100000
	
	// ---------------------------------------------------------------------------------
	std::ostream& operator<< (std::ostream& o, PortalRect& r) {
		o << "RECT [top:" << r.top << ", left:" << r.left << ", bottom: " << r.bottom << ", right:" << r.right << "]";
		return o;
	}
	
	// ---------------------------------------------------------------------------------
	// ----------------- Portal Class implementation -----------------------------------
	// ---------------------------------------------------------------------------------
	Portal::Portal(BspNode* source, BspNode* target, Plane plane) {
		points = new PortalPoints();
		points->clear();
		mFrameNum = 0xFFFFF;
		mMentions = 0;
		
		mSource = source;
		mTarget = target;
		
		this->plane = plane;
	}	
		
	// ---------------------------------------------------------------------------------
	Portal::~Portal() {
		delete points;
	}
			
	// ---------------------------------------------------------------------------------
	Portal::Portal(Portal *src) {
		const PortalPoints& pnts = src->getPoints();
				
		points = new PortalPoints();
		points->clear();
		
		points->reserve(pnts.size());
		
		for (unsigned int x = 0; x < pnts.size(); x++)
			points->push_back(pnts.at(x));

		this->mTarget = src->getTarget();
		this->mSource = src->getSource();
		this->plane = src->getPlane();
		mFrameNum = src->mFrameNum;
	}
			
	// ---------------------------------------------------------------------------------
	void Portal::addVertex(float x, float y, float z) {
		points->push_back(Vector3(x,y,z));
	}
	
	// ---------------------------------------------------------------------------------		
	void Portal::addVertex(Vector3 a) {
		points->push_back(a);
	}
			
	// ---------------------------------------------------------------------------------
	const PortalPoints& Portal::getPoints() {
		return *points;
	}
			
	// ---------------------------------------------------------------------------------
	int Portal::getPointCount() {
		return points->size();
	}
	
	// ---------------------------------------------------------------------------------		
	BspNode* Portal::getTarget() {
		return mTarget;
	}
	
	// ---------------------------------------------------------------------------------
	BspNode* Portal::getSource() {
		return mSource;
	}
	
	// ---------------------------------------------------------------------------------
	const Plane& Portal::getPlane() {
		return plane;
	}
	
	// ---------------------------------------------------------------------------------
	void Portal::setPlane(Plane plane) {
		this->plane = plane;
	}
	
	// ---------------------------------------------------------------------------------
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
	
	
	// ---------------------------------------------------------------------------------
	/** @todo remove This, outdated */
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
	
	// ---------------------------------------------------------------------------------
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
	
	// ---------------------------------------------------------------------------------
	int Portal::clipByPlane(const Plane &plane, bool &didClip) {
		int positive = 0;
		int negative = 0;
		
		int pointcount = points->size();
		
		//first we mark the vertices
		Plane::Side *sides = new Plane::Side[pointcount];
		//std::vector< Plane::Side > sides;
		
		unsigned int idx;
		
		for (idx = 0; idx < pointcount; idx++) {
			Plane::Side side = plane.getSide(points->at(idx));
			sides[idx] = side; // push the side of the vertex into the side buffer...
			
			switch (side) {
				case Plane::POSITIVE_SIDE : positive++; break;
				case Plane::NEGATIVE_SIDE : negative++; break;
				default: ;
			}
		}
		
		// Now that we have the poly's side classified, we can process it...
		
		if (negative == 0) {
			delete sides;
			return points->size(); // all the vertices were inside
		}
		
		didClip = true;
		
		if (positive == 0) { // we clipped away the whole poly
			delete sides;
			points->clear();
			return 0;
		}
		
		// some vertices were on one side, some on the other
		
		PortalPoints *newpnts =  new PortalPoints();
		
		long prev = pointcount - 1; // the last one
		
		for (idx = 0; idx < pointcount; idx++) {
			const Plane::Side side = sides[idx];
			
			if (side == Plane::POSITIVE_SIDE) { 
				if (sides[prev] == Plane::POSITIVE_SIDE) { 
					newpnts->push_back(points->at(idx));
				} else {
					// calculate a new boundry positioned vertex
					const Vector3& v1 = points->at(prev);
					const Vector3& v2 = points->at(idx);
					Vector3 dv = v2 - v1; // vector pointing from v2 to v1 (v1+dv*0=v2 *1=v1)
					
					// the dot product is there for a reason! (As I have a tendency to overlook the difference)
					float t = plane.getDistance(v2) / (plane.normal.dotProduct(dv));
					
					newpnts->push_back(v2 - (dv * t)); // a new, boundry placed vertex is inserted
					newpnts->push_back(v2);
				}
			} else { 
				if (sides[prev] == Plane::POSITIVE_SIDE) { // if we're going outside
					// calculate a new boundry positioned vertex
					const Vector3 v1 = points->at(idx);
					const Vector3 v2 = points->at(prev);
					const Vector3 dv = v2 - v1;
					
					float t = plane.getDistance(v2) / (plane.normal.dotProduct(dv));
					
					newpnts->push_back(v2 - (dv * t)); // a new, boundry placed vertex is inserted
				}
			}
			
			prev = idx;
		}
		
		if (newpnts->size() < 3) { // a degenerate polygon as a result...
			points->clear();
			delete newpnts;
			delete sides;
			return 0;
		}
		
		delete sides;
		delete points;
		points = newpnts; 
		return points->size();
	}


	// ---------------------------------------------------------------------------------
	void Portal::refreshScreenRect(Camera *cam, Matrix4& toScreen, PortalFrustum *frust) {
		// inverse coords to let the min/max initialize
		screenRect.top = -INF;
		screenRect.right = -INF;
		screenRect.left = INF;
		screenRect.bottom = INF;
		
		// Erase the actual rect
		mActualRect = screenRect;
		
		// Calculate the backface cull. This is not good: (!)
		// mPortalCull = (plane.getSide(cam->getDerivedPosition()) == Plane::NEGATIVE_SIDE);
		
		// Backface cull. Hmm. The portal won't be culled if a vector camera-vertex dotproduct normal will be greater than 0
		// The mistake I was doing here was that I used camera's center direction, which gave <=0 dotp even for visible portals.
		Vector3 camToV0 = points->at(0) - cam->getDerivedPosition(); 
		// HMM. The get derived position here is realy not a timesaver call. Param? It should be implemented to cache the value.
		
		float dotp = camToV0.dotProduct(plane.normal);
		
		mPortalCull = (dotp > 0);
		
		// skip these expensive operations if we encounter a backface cull
		if (mPortalCull) 
			return;

		// We also can cull away the portal if it is behind the camera's near plane. Can we? This needs distance calculation with the surrounding sphere
		
		
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
				int x, y;
				
				// This is one time-consuming line... I wonder how big eater this line is.
				Vector3 hcsPosition = toScreen * scr_points.at(idx);
				
				// TODO: Parametrise the Screen width/height
				// Notice that the X coord is flipped. This is because of the transform matrix, but it is not a problem for us
				// HMM. Better convert to int as soon as possible. * screen_size/2 + screen_size/2
				x = ((int)(hcsPosition.x * 512) + 512); 
				y = ((int)(hcsPosition.y * 384) + 384); 
				
				screenRect.top    = MAX(screenRect.top, y);
				screenRect.bottom = MIN(screenRect.bottom, y);
				screenRect.right  = MAX(screenRect.right, x);
				screenRect.left   = MIN(screenRect.left, x);
			}
	
			#ifdef debug
			std::cerr << "      * Portal " << mPortalID << "  rect " << screenRect << std::endl;
			#endif
			
			// release only if clip produced a new poly
			if (onScreen != this)
				delete onScreen;
		}
		
		/* EXPERIMENTAL version. Seems to work pretty the same
		bool didc;
		Portal *onScreen = new Portal(this);
		
		onScreen->clipByPlane(cam->getFrustumPlane(FRUSTUM_PLANE_NEAR), didc);
		
		// If we have a non-zero cut result
		if (onScreen) {
			const PortalPoints& scr_points = onScreen->getPoints();
			
			unsigned int idx;
			
			// project all the vertices to screen space
			for (idx = 0; idx < scr_points.size(); idx++) {
				int x, y;
				
				// This is time consuming line, for sure. Maybe if the calculation could be done without the Z coord...
				// Also a rollout of these equations could show some time saving optimisations
				Vector3 hcsPosition = cam->getProjectionMatrix() * (cam->getViewMatrix() * scr_points.at(idx));
				
				// TODO: Parametrise the Screen width/height; The inverting "2.0f -" for x coord was removed. Time saver
				x = (int)((hcsPosition.x + 1.0f) * (1024.0f/2.0f)); 
				y = (int)((hcsPosition.y + 1.0f) * ( 768.0f/2.0f)); 
				
				screenRect.top    = MAX(screenRect.top, y);
				screenRect.bottom = MIN(screenRect.bottom, y);
				screenRect.right  = MAX(screenRect.right, x);
				screenRect.left   = MIN(screenRect.left, x);
			}
	
			screenRect.top    = MIN(screenRect.top, 768);
			screenRect.bottom = MAX(screenRect.bottom, 0);
			screenRect.right  = MIN(screenRect.right, 1024);
			screenRect.left   = MAX(screenRect.left, 0);

			
			#ifdef debug
			std::cerr << "      * Portal " << mPortalID << "  rect " << screenRect << std::endl;
			#endif
			
			// release only if clip produced a new poly
			if (onScreen != this)
				delete onScreen;
		}
		*/
	}
	
	// ---------------------------------------------------------------------------------
	bool Portal::intersectByRect(PortalRect &boundry, PortalRect &target) {
		#ifdef debug
		std::cerr << "    * intersection " << boundry << " with " << screenRect << std::endl;
		#endif
		
		// now do the intersection
		/*
		target.top    = (screenRect.top    < boundry.top)    ? screenRect.top    : boundry.top;
		target.bottom = (screenRect.bottom > boundry.bottom) ? screenRect.bottom : boundry.bottom;
		target.right  = (screenRect.right  < boundry.right)  ? screenRect.right  : boundry.right;
		target.left   = (screenRect.left   > boundry.left)   ? screenRect.left   : boundry.left;
		*/
		
		target.top = MIN(screenRect.top, boundry.top);
		target.bottom = MAX(screenRect.bottom, boundry.bottom);
		target.left = MAX(screenRect.left, boundry.left);
		target.right = MIN(screenRect.right, boundry.right);
		
		
		#ifdef debug
		std::cerr << "    * intersection  result " << target << std::endl;
		#endif
		
		if (target.top < target.bottom)
			return false;
	
		if (target.right < target.left)
			return false;
		
		return true;
	}
	
	// ---------------------------------------------------------------------------------
	bool Portal::unionActualWithRect(PortalRect &addition) {
		# ifdef debug
		std::cerr << "    * union " << mActualRect << " with " << addition << std::endl;
		#endif
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
		
		#ifdef debug
		std::cerr << "    * union result " << mActualRect << " changed " << changed << std::endl;
		#endif
		
		return changed;
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
	int Portal::optimize() {
		// step one: Remove vertices not forming an edge break (lying on an edge of previous and next vertex)
		// Is this worth the trouble? It removes ~5-400 vertices in average situation per mission (Often more than 300, sure this depends on EQUALITY_ANGLE value)
		
		// In my opinion, the angles we get between three consecutive vertices are of two kind: 
		//  - Those limited by CSG primitives, 
		//  - Those produced as a unoptimised CSG operations (e.g. an edge point not forming the polygon shape).
		// The latter will likely get nearly ~180 degrees. Not as those made by intention
		// Why the hell didn't the LG guys remove those?
		
		int removed = 0;
		int idx;
		
		for (idx = 0; idx < points->size(); idx++) {
			int prev = idx - 1;
			prev = (prev < 0) ? points->size()-1 : prev; 
			int next = (idx + 1) % points->size();
			
			Vector3 vprev = points->at(prev);
			Vector3 vact = points->at(idx);
			Vector3 vnxt = points->at(next);
			
			// test if the normalised vectors equal to some degree
			Vector3 vpta = (vact - vprev);
			Vector3 vatn = (vnxt - vact);
			
			vpta.normalise();
			vatn.normalise();
			
			// If the direction TO is the same as direction FROM, the vertex does not form an edge break (to some degree we can tolerate).
			if (vpta.directionEquals(vatn, Radian(EQUALITY_ANGLE))) {
				points->erase(points->begin() + idx);
				
				idx--; // better go to the next, than skip one
				removed++;
			}
		}
		
		// The previous operation was quite cheap and simple. What other can we do to simplify the portal before rendering loop?
		// The reduction must leave the resulting polygon not smaller in any way than the previous (identical, or bigger, not crossing the old edges with new)
		
		// We could do a polygon reduction by removing vertices till sufficient, those which form a triangle not too big:
		// This would work like this: previous - actual - next edges would be taken, and actual edge would be removed if the new vertex 
		// (previous - next edge intersection) to actual edge triangle would be the smallest from the whole polygon.
		// The condition to succeed is sure that previous-actual and actual-next have a inner angle sum > 180
		// Someone wanting to write this?
		
		// Update the bounding volume
		refreshBoundingVolume();
		
		return removed;
	}
} // namespace Ogre
