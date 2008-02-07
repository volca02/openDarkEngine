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
 
#include "DarkConvexPolygon.h"
#include "DarkBspNode.h"
#include "DarkSceneManager.h"

#include <OgreManualObject.h>

#define MAX(a,b) ((a>b)?a:b)
#define MIN(a,b) ((a<b)?a:b)


// This Angle defines the tolerance for comparision of the normalised edge directions (testing whether those are equal) - dropping of useless portal vertices
#define EQUALITY_ANGLE 0.0000000001 

namespace Ogre {

	// ---------------------------------------------------------------------------------
	// ----------------- Polygon Class implementation -----------------------------------
	// ---------------------------------------------------------------------------------
	ConvexPolygon::ConvexPolygon(Plane plane) {
		this->mPlane = plane;
	}	
		
	// ---------------------------------------------------------------------------------
	ConvexPolygon::~ConvexPolygon() {
	}
			
	// ---------------------------------------------------------------------------------
	ConvexPolygon::ConvexPolygon(const ConvexPolygon& src) {
		const PolygonPoints& pnts = src.getPoints();
				
		mPoints.reserve(pnts.size());
		
		for (unsigned int x = 0; x < pnts.size(); x++)
			mPoints.push_back(pnts.at(x));

		this->mPlane = src.getPlane();
	}
			
	// ---------------------------------------------------------------------------------
	void ConvexPolygon::addPoint(float x, float y, float z) {
		mPoints.push_back(Vector3(x,y,z));
	}
	
	// ---------------------------------------------------------------------------------		
	void ConvexPolygon::addPoint(Vector3 a) {
		mPoints.push_back(a);
	}
			
	// ---------------------------------------------------------------------------------
	const PolygonPoints& ConvexPolygon::getPoints() const {
		return mPoints;
	}
			
	// ---------------------------------------------------------------------------------
	int ConvexPolygon::getPointCount() {
		return mPoints.size();
	}
	
	// ---------------------------------------------------------------------------------
	const Plane& ConvexPolygon::getPlane() const {
		return mPlane;
	}
	
	// ---------------------------------------------------------------------------------
	void ConvexPolygon::setPlane(const Plane& plane) {
		mPlane = plane;
	}
	
	// ---------------------------------------------------------------------------------
	unsigned int ConvexPolygon::getOutCount(const Plane &plane) {
		int negative = 0;
		
		PolygonPoints::const_iterator it = mPoints.begin();
		PolygonPoints::const_iterator end = mPoints.end();
		
		for (; it != end; ++it) {
			Plane::Side side = mPlane.getSide(*it);
			
			if (side == Plane::NEGATIVE_SIDE)
				negative++;
		}
		
		return negative;
	}
	
	// ---------------------------------------------------------------------------------
	int ConvexPolygon::clipByPlane(const Plane &plane, bool &didClip) {
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
			Plane::Side side = plane.getSide(*it);
			sides[idx] = side; // push the side of the vertex into the side buffer...
			
			switch (side) {
				case Plane::POSITIVE_SIDE : positive++; break;
				case Plane::NEGATIVE_SIDE : negative++; break;
				default: ;
			}
		}
		
		// Now that we have the poly's side classified, we can process it...
		
		if (negative == 0) {
			delete[] sides;
			return mPoints.size(); // all the vertices were inside
		}
		
		didClip = true;
		
		if (positive == 0) { // we clipped away the whole poly
			delete[] sides;
			mPoints.clear();
			return 0;
		}
		
		// some vertices were on one side, some on the other
		
		PolygonPoints newpnts;
		
		long prev = pointcount - 1; // the last one
		
		for (idx = 0; idx < pointcount; idx++) {
			const Plane::Side side = sides[idx];
			
			if (side == Plane::POSITIVE_SIDE) { 
				if (sides[prev] == Plane::POSITIVE_SIDE) { 
					newpnts.push_back(mPoints.at(idx));
				} else {
					// calculate a new boundry positioned vertex
					const Vector3& v1 = mPoints.at(prev);
					const Vector3& v2 = mPoints.at(idx);
					Vector3 dv = v2 - v1; // vector pointing from v2 to v1 (v1+dv*0=v2 *1=v1)
					
					// the dot product is there for a reason! (As I have a tendency to overlook the difference)
					float t = plane.getDistance(v2) / (plane.normal.dotProduct(dv));
					
					newpnts.push_back(v2 - (dv * t)); // a new, boundry placed vertex is inserted
					newpnts.push_back(v2);
				}
			} else { 
				if (sides[prev] == Plane::POSITIVE_SIDE) { // if we're going outside
					// calculate a new boundry positioned vertex
					const Vector3 v1 = mPoints[idx];
					const Vector3 v2 = mPoints[prev];
					const Vector3 dv = v2 - v1;
					
					float t = plane.getDistance(v2) / (plane.normal.dotProduct(dv));
					
					newpnts.push_back(v2 - (dv * t)); // a new, boundry placed vertex is inserted
				}
			}
			
			prev = idx;
		}
		
		if (newpnts.size() < 3) { // a degenerate polygon as a result...
			mPoints.clear();

			delete sides;
			return 0;
		}
		
		delete[] sides;

		// Swap the old point list with the new one
		mPoints.swap(newpnts); 
		return mPoints.size();
	}

	// ---------------------------------------------------------------------------------
	int ConvexPolygon::optimize() {
		// Remove vertices not forming an edge break (lying on an edge of previous and next vertex)
		// Is this worth the trouble? It removes ~5-400 vertices in average situation per mission (Often more than 300, sure this depends on EQUALITY_ANGLE value)
		
		// In my opinion, the angles we get between three consecutive vertices are of two kind: 
		//  - Those limited by CSG primitives, 
		//  - Those produced as a unoptimised CSG operations (e.g. an edge point not forming the polygon shape).
		// The latter will likely get nearly ~180 degrees. Not as those made by intention
		// Why the hell didn't the LG guys remove those? (Probably not worth the trouble)
		
		int removed = 0;
		unsigned int idx;
		
		for (idx = 0; idx < mPoints.size(); idx++) {
			int prev = idx - 1;
			prev = (prev < 0) ? mPoints.size()-1 : prev; 
			int next = (idx + 1) % mPoints.size();
			
			Vector3 vprev = mPoints.at(prev);
			Vector3 vact = mPoints.at(idx);
			Vector3 vnxt = mPoints.at(next);
			
			// test if the normalised vectors equal to some degree
			Vector3 vpta = (vact - vprev);
			Vector3 vatn = (vnxt - vact);
			
			vpta.normalise();
			vatn.normalise();
			
			// If the direction TO is the same as direction FROM, the vertex does not form an edge break (to some degree we can tolerate).
			if (vpta.directionEquals(vatn, Radian(EQUALITY_ANGLE))) {
				mPoints.erase(mPoints.begin() + idx);
				
				idx--; // better go to the next, than skip one
				removed++;
			}
		}
		
		return removed;
	}
	
	// ---------------------------------------------------------------------------------
	bool ConvexPolygon::isHitBy(const Ray& ray) const {
		// Create a RayStart->Edge defined plane. 
		// If the intersection of the ray to the polygon's plane is inside for all the planes, it is inside the poly
		// By inside, I mean the point is on positive side of the plane
		std::pair<bool, Real> intersection = ray.intersects(mPlane);
		
		if (!intersection.first) 
			return false;
		
		// intersection point
		Vector3 ip     = ray.getPoint(intersection.second);
		Vector3 origin = ray.getOrigin();
				
		for (unsigned int idx = 0; idx < mPoints.size(); idx++) {
			int iv2 = (idx + 1) % mPoints.size();
			
			Vector3 v1 = mPoints[idx];
			Vector3 v2 = mPoints[iv2];
			
			Plane frp(origin, v1, v2);
			
			// test whether the intersection is inside
			if (frp.getSide(ip) != Plane::POSITIVE_SIDE)
				return false;
		}
		
		return true;
	}
	
	// ---------------------------------------------------------------------------------
	bool ConvexPolygon::enclosesSphere(const Vector3& pos, const Real& radius, const Real& distance) const {
		// calculate the intersection point
		Vector3 ip = pos - distance * mPlane.normal;
		
		if (distance > radius)
			return false;
		
		// intersection circle radius
		Real irad = radius*radius - distance*distance;
				
		for (unsigned int idx = 0; idx < mPoints.size(); idx++) {
			int iv2 = (idx + 1) % mPoints.size();
			
			Vector3 v1 = mPoints[idx];
			Vector3 v2 = mPoints[iv2];
			Vector3 v3 = v1 + mPlane.normal;
			
			Plane frp(v1, v2, v3);
			
			// test whether the intersection point is in distance < than the intersected radius - not totally covered or outside
			if (frp.getDistance(ip) < irad)
				return false;
		}
		
		return true;
	}
} // namespace Ogre
