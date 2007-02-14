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
 
#ifndef _OgrePortalFrustum_H__
#define _OgrePortalFrustum_H__

#include "OgreBspPrerequisites.h"
#include <OgrePlane.h>
#include <OgreVector3.h>
#include <OgreCamera.h>

namespace Ogre {

	class Portal;
	
	typedef std::vector< Plane > FrustumPlanes;
	
	/** A Multiple-planed frustum. Defined by either a camera, or a camera and a polygon defining the boundaries of the frustum. */
	class PortalFrustum {
		private:
			FrustumPlanes planes;
		
		public:
			// construct a new Frustum out of a camera and a Portal (which has been clipped as needed previously)
			PortalFrustum(Camera *cam, Portal *poly);
		
			// new frustum, based solely on camera... This is the variant we use to start rendering... 
			// each portal is then rendered using frustum constructed with the other constructor
			PortalFrustum(Camera *cam);
			
			void addPlane(Plane a);
			
			FrustumPlanes& getPlanes();
			
			/** 
				Classify Portal by it's bounding volume:
					-1 all outside
					 0 intersects
					 1 all inside
			*/
			int getPortalClassification(Portal *src);
		
			/** 
			* Clips the ginven Portal by frustum planes.
			* returns a Portal pointer (not necesi necessarily an new one), or NULL if clipped away
			*/
			Portal *clipPortal(Portal *src, bool &didClip);
	};

}

#endif
