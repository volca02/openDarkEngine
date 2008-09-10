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
 
#ifndef __DARKPORTALFRUSTUM_H
#define __DARKPORTALFRUSTUM_H

#include "config.h"

#include "DarkBspPrerequisites.h"

#include <OgrePlane.h>
#include <OgreVector3.h>
#include <OgreCamera.h>

namespace Ogre {

	class Portal;
	
	typedef std::vector< Plane > FrustumPlanes;
	
	/** A Multiple-planed frustum. Defined by either a camera, or a camera and a polygon defining the boundaries of the frustum. 
	* @deprecated This class should be deprecated. DarkCamera class should be able to handle all the visibility updates alone */
	class OPDELIB_EXPORT PortalFrustum {
		private:
			FrustumPlanes planes;
		
		public:
			// construct a new Frustum out of a camera and a Portal (which has been clipped as needed previously)
			PortalFrustum(const Camera *cam, Portal* poly);
		
			// new frustum, based solely on camera... This is the variant we use to start rendering... 
			// each portal is then rendered using frustum constructed with the other constructor
			PortalFrustum(const Camera *cam);
			
			// Constructs the portalfrustum from a positional point and a portal to look through
			PortalFrustum(const Vector3& point, Portal* poly);
			
			// Constructs an N sided portal frustum as an aproximation of cone with inner angle ang
			PortalFrustum(const Vector3& point, const Vector3& direction, Radian ang, size_t sides = 4);
			
			void addPlane(const Plane& a);
			
			const FrustumPlanes& getPlanes() const;
			
			/** 
				Classify Portal by it's bounding volume:
					-1 all outside
					 0 intersects
					 1 all inside
			*/
			int getPortalClassification(Portal* src) const;
		
			/** 
			* Clips the ginven Portal by frustum planes.
			* returns a Portal pointer (not necesi necessarily an new one), or NULL if clipped away
			*/
			Portal *clipPortal(Portal* src, bool &didClip) const;
	};

}

#endif
