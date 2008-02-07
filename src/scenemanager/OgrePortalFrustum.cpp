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
 
#include <OgrePlane.h>

#include "DarkPortalFrustum.h"
#include "DarkPortal.h"

namespace Ogre {

	/*---------------------------------------------------------*/	
	// construct a new Frustum out of a camera and a Portal (which has been clipped as needed previously)
	PortalFrustum::PortalFrustum(const Camera *cam, Portal* poly) {
		// we create a new frustum planes using a 3*Vector3 constructor
		const PortalPoints& pnts = poly->getPoints();
		
		if (pnts.size() < 3) 
			// we have a degenerated poly!!!
			return;
		
		Vector3 cam_pos = cam->getDerivedPosition();
		
		planes.clear();
		
		// should we check whether the normal is faced to the center of the poly?
		PortalPoints::const_iterator previous = pnts.end()--;
		
		PortalPoints::const_iterator it = pnts.begin();
		PortalPoints::const_iterator pend = pnts.end();
		
		for (; it != pend; it++) {
			Plane plane;
			
			plane.redefine(*previous, *it, cam_pos); // Dunno why, the constructor was failing to intialize the values directly...
			
			planes.push_back(plane);
			previous = it;
		}
	}

	/*---------------------------------------------------------*/
	PortalFrustum::PortalFrustum(const Camera *cam) {
		// read out the LEFT, RIGHT, TOP and BOTTOM planes from the camera, and push them into our planes var...
		planes.clear();

		planes.push_back(cam->getFrustumPlane(FRUSTUM_PLANE_BOTTOM));
		planes.push_back(cam->getFrustumPlane(FRUSTUM_PLANE_LEFT));
		planes.push_back(cam->getFrustumPlane(FRUSTUM_PLANE_TOP));
		planes.push_back(cam->getFrustumPlane(FRUSTUM_PLANE_RIGHT));
	}
	
	/*---------------------------------------------------------*/
	PortalFrustum::PortalFrustum(const Vector3& point, Portal* poly) {
		// we create a new frustum planes using a 3*Vector3 constructor
		const PortalPoints& pnts = poly->getPoints();
		
		if (pnts.size() < 3) 
			// we have a degenerated poly!!!
			return;
		
		planes.clear();
		
		// should we check whether the normal is faced to the center of the poly?
		PortalPoints::const_iterator previous = pnts.end()--;
		
		PortalPoints::const_iterator it = pnts.begin();
		PortalPoints::const_iterator pend = pnts.end();
		
		for (; it != pend; it++) {
			Plane plane;
			
			plane.redefine(*previous, *it, point);
			
			planes.push_back(plane);
			previous = it;
		}
	}
	/*---------------------------------------------------------*/
	PortalFrustum::PortalFrustum(const Vector3& point, const Vector3& direction, Radian ang, size_t sides) {
		// The planes will be generated as a cone approximation

		// An up vector to be used
		Vector3 up = direction.perpendicular();

		Quaternion q;
			
		// Rotate the up vector by the desired amount
		q.FromAngleAxis( Radian(0), direction );
        
		Vector3 current = q * up;
 
		// Now rotate by our desired angle
		q.FromAngleAxis( ang, current );
			
		// We have the first result to use
		current = q * current;
			
		for (size_t o = 0; o < sides; ++o) {
			// Define the rotation now, as a rotated variant of the axis
	        q.FromAngleAxis( Radian(2 * Math::PI * (o + 1) / sides), direction );
        
			Vector3 next = q * up;
 
			// Now rotate by our desired angle
			q.FromAngleAxis( ang, next );
			
			// This is the resulting normal
			next = q * next;

			Plane plane;
			
			plane.redefine(point + current, point + next, point);

			planes.push_back(plane);

			current = next;
		}
	}
	
	/*---------------------------------------------------------*/
	void PortalFrustum::addPlane(const Plane& a) {
		planes.push_back(a);
	}
	
	/*---------------------------------------------------------*/
	const FrustumPlanes& PortalFrustum::getPlanes() const {
		return planes;
	}
	
	/*---------------------------------------------------------*/
	int PortalFrustum::getPortalClassification(Portal* src) const {
		// test the absolute distance of the Portal center to each plane.
		// if it is equal or smaller than radius, we intersect 
		// otherwise if the distance is below zero, we are away, then end up immedietally telling so
		const Vector3 center = src->mCenter;
		float radius = src->mRadius;
		
		FrustumPlanes::const_iterator it = planes.begin();
		FrustumPlanes::const_iterator pend = planes.end();
		
		for (; it != pend; it++) {
			float dist = it->getDistance(center);
			
			if (fabs(dist) < radius) // intersection
				return 0;
			
			if (dist < -radius) // totally outside, so we can safely say we do not see the poly by our frustum
				return -1;
		}
		
		return 1;
	}
	
	/*---------------------------------------------------------*/
	Portal *PortalFrustum::clipPortal(Portal* src, bool &didClip) const {
		// todo: test if the bounding box (sphere) is completely: inside/outside, or intersecting.
		// todo: only clip the poly if intersecting.
		
		int cl = getPortalClassification(src);
		
		if (cl ==  1) // all inside
			return src;
		
		if (cl == -1) // all outside
			return NULL;
		
		
		Portal *new_poly = new Portal(*src);
		
		// The poly intersects with its bounding volume, so clip it using all planes we have
		FrustumPlanes::const_iterator it = planes.begin();
		FrustumPlanes::const_iterator pend = planes.end();
		
		for (; it != pend; it++) {
			if (new_poly->clipByPlane(*it, didClip) <= 2) {
				// was totally clipped away
				delete new_poly;
				return NULL;
			}
		}
		
		return new_poly;
	}
}
