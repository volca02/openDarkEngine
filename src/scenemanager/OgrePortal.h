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
 
#ifndef _OgrePortal_H__
#define _OgrePortal_H__

#include "OgreVector3.h"
#include "OgrePlane.h"
#include "OgrePortalFrustum.h"
#include <iostream>

namespace Ogre {

	class PortalFrustum;
	class BspNode;
		
	/** @brief An int-type based portal rectangle struct 
	*
	* This structure is used by Portal class when calculating the on-screen bounding rectangle.
	* It is ment as a Ogre::Rect replacement, because that one uses Real as a base type, which is slower to use
	*/
	struct PortalRect {
			int left, right, bottom, top;
	};
	

	std::ostream& operator<< (std::ostream& o, PortalRect& r);
	
	/** A vector of Vertices used for Portal shape definition */
	typedef std::vector< Vector3 > PortalPoints;
	
	/** @brief A Portal class used for SceneNode to SceneNode visibility testing
	*
	* Implements One direction portal, defined by the edge points. 
	* visibility determination is done using a to-screen projected portal vertices bounding rectangles PortalRects
	* @note Please note that the direction of the plane's normal has to comply with the 
	* portals vertex order derived normal. Also note that no check that the points actualy lie on the plane is done. */
	class Portal {
		friend class DarkSceneManager;
		friend class PortalFrustum;
			
		private:
			/** A vector containing the Vector3 values - Portal/portal edge vertices */
			PortalPoints 	*points;
			/** The plane on which the portal lies */
			Plane		plane;
			/** Screen - space bounding rectangle of the portal */
			PortalRect	screenRect;  
		
		protected:
			/** target Scene Node (Cell) for this Portal */
			BspNode *mTarget;

			/** source Cell for this Portal */
			BspNode *mSource;
			
			/** Helping value, used to determine the validity of the Portal's screen rectangle */
			unsigned int 	mFrameNum;
			
			/** for debugging - portal id (e.g. order number) */
			int	mPortalID;
			
			/** Portal back-face cull - true if should be culled */ 
			bool mPortalCull;
			
			/** Actual view rectangle */
			PortalRect	mActualRect;  
			
			/** Number of mentions (e.g. nonzero if this portal was reevaluated) */
			int	mMentions;
			
			/** Portal's center vertex */
			Vector3 mCenter;
			
			/** deprecated: Portals bounding sphere radius. Should use the built-in bounding volumes */
			float mRadius;
		public:
			/** Default constructor. Defines an empty geometry, and source and destination SceneNodes that are parameters
			* @param source Source SceneNode
			* @param target Target SceneNode
			* @param plane Portal plane (Normal direction does matter) */
			Portal(BspNode* source, BspNode* target, Plane plane);
		
			~Portal();
			
			/** Constructs the class as a copy of already existing portal */
			Portal(Portal *src);
			
			void addVertex(float x, float y, float z);
			
			void addVertex(Vector3 a);
			
			const PortalPoints& getPoints();
		
			int getPointCount();
			
			/** Returns the target DarkSceneNode for this portal */
			BspNode* getTarget();
			
			/** Returns the source DarkSceneNode for this portal */
			BspNode* getSource();
		
			const Plane& getPlane();
			
			void setPlane(Plane plane);
			
			/** get the number of Portal's vertices outside a given plane
			* @return the count of points which lie outside (negative distance from the plane)
			*/
			unsigned int getOutCount(Plane &plane);
			
			// is the Portal seen by camera frustum at all?
			bool isSeen(Camera *cam);
			
			void refreshBoundingVolume();
			
			// Portal copy operator
			// Portal& operator= (Portal *src);
			
			/** Clips the poly using a plane (and replaces instances vertices after success)
			* @return number of vertices in the new poly
			*/
			int clipByPlane(const Plane &plane, bool &didClip);
			
			/**
			* Refreshes screen projection bounding Rectangle
			* @param cam Camera which is used by the projection
			* @param toScreen A precomputed Projection*View matrix matrix4 which is used to project the vertices to screen
			* @param frust PortalFrustum used to cut away non visible parts of the portal
			*/
			void refreshScreenRect(Camera *cam, Matrix4& toScreen, PortalFrustum *frust);
			
			/**
			* Intersects the Portals bounding rectangle by the given rectangle, and writes the result to the target parameter
			*/
			bool intersectByRect(PortalRect &boundry, PortalRect &target);
			
			/**
			* Union the actual view rectangle with addition rectangle. Returns true on a view change.
			*/
			bool unionActualWithRect(PortalRect &addition);

			/**
			* Debugging portal id setter
			*/
			void setPortalID(int id);
			
			/** Attaches the portal to the source and destination DarkSceneNodes */
			void attach();
			
			/** Optimizes the portal. Removes unneeded, unnecessary vertices which slow down the visibility evaluation.
			* @return int Number of vertices removed */
			int optimize();
	};

	/** A bunch of the portal pointers. Is used in the DarkSceneNode class. */
	typedef std::set< Portal *> PortalList;
	
	/** An iterator over a PortalList */
	typedef std::set< Portal *>::const_iterator PortalListConstIterator;
}

#endif
