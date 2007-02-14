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

#include <OgreCamera.h>
#include <OgreSimpleRenderable.h>
#include <OgreVector3.h>
#include <OgrePlane.h>

#include "OgrePortalFrustum.h"
#include "OgrePolygon.h"
#include "OgreBspPrerequisites.h"

#include <iostream>

#define MAX(a,b) ((a>b)?a:b)
#define MIN(a,b) ((a<b)?a:b)

// Helping 'infinity' macro for PortalRect coords
#define INF 100000

namespace Ogre {
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
	typedef Ogre::PolygonPoints PortalPoints;
	
	/** @brief A Portal class used for SceneNode to SceneNode visibility testing
	*
	* Implements One direction portal, defined by the edge points. 
	* visibility determination is done using a to-screen projected portal vertices bounding rectangles PortalRects
	* @note Please note that the direction of the plane's normal has to comply with the 
	* portals vertex order derived normal. Also note that no check that the points actualy lie on the plane is done. */
	class Portal : public Polygon, public SimpleRenderable {
		friend class DarkSceneManager;
		friend class PortalFrustum;
			
		protected:
			/// Static screen width size half
			static int mScreenWidth2;
			/// Static screen height size half
			static int mScreenHeight2;
			
        		/* The SimpleRenderable methods override. */
			void getWorldTransforms( Matrix4* xform ) const;
        		const Quaternion& getWorldOrientation(void) const;
        		const Vector3& getWorldPosition(void) const;
	
			void Portal::refreshPortalRenderable();
			/** Screen - space bounding rectangle of the portal */
			PortalRect	screenRect;  
		
			/** On-demand (lazy) generated movable object for debug rendering */
			ManualObject* mMovableObject;
			
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
			
			/** Portals bounding sphere radius. */
			float mRadius;
		public:
			/** Default constructor. Defines an empty geometry, and source and destination SceneNodes that are parameters
			* @param source Source SceneNode
			* @param target Target SceneNode
			* @param plane Portal plane (Normal direction does matter) */
			Portal(BspNode* source, BspNode* target, Plane plane);
		
			~Portal();
			
			/** Copy ctor */
			Portal(Portal *src);
			
			/** Returns the target DarkSceneNode for this portal */
			BspNode* getTarget();
			
			/** Returns the source DarkSceneNode for this portal */
			BspNode* getSource();
		
			/** Refresh the center and radius bounding sphere parameters */
			void refreshBoundingVolume();
			/**
			* Refreshes screen projection bounding Rectangle
			* @param cam Camera which is used by the projection
			* @param toScreen A precomputed Projection*View matrix matrix4 which is used to project the vertices to screen
			* @param frust PortalFrustum used to cut away non visible parts of the portal
			*/
			void refreshScreenRect(Camera *cam, Matrix4& toScreen, PortalFrustum *frust) {
				// inverse coords to let the min/max initialize
				screenRect.top = -INF;
				screenRect.right = -INF;
				screenRect.left = INF;
				screenRect.bottom = INF;
				
				// Erase the actual rect
				mActualRect = screenRect;
				
				// Backface cull. The portal won't be culled if a vector camera-vertex dotproduct normal will be greater than 0
				Vector3 camToV0 = mPoints->at(0) - cam->getDerivedPosition(); 
						
				float dotp = camToV0.dotProduct(mPlane.normal);
				
				mPortalCull = (dotp > 0);
				
				// skip these expensive operations if we encounter a backface cull
				if (mPortalCull) 
					return;
		
				// We also can cull away the portal if it is behind the camera's near plane. Can we? This needs distance calculation with the surrounding sphere
				
				// We have to cut the Portal using camera's frustum first, as this should solve the to screen projection problems...
				// the reason is that the coords that are at the back side or near the view point do not get projected right (that is right for our purpose)
				// it should be sufficient to clip by near plane only though
				
				bool didc;
				Portal *onScreen = frust->clipPortal(this, didc);
				
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
						// NOTE: The X coord is flipped. This is because of the transform matrix, but it is not a problem for us
						x = ((int)(hcsPosition.x * mScreenWidth2) + mScreenWidth2); 
						y = ((int)(hcsPosition.y * mScreenHeight2) + mScreenHeight2); 
						
						screenRect.top    = MAX(screenRect.top, y);
						screenRect.bottom = MIN(screenRect.bottom, y);
						screenRect.right  = MAX(screenRect.right, x);
						screenRect.left   = MIN(screenRect.left, x);
					}
			
					// release only if clip produced a new poly
					if (onScreen != this)
						delete onScreen;
				}
			}
				
			
			
			/**
			* Intersects the Portals bounding rectangle by the given rectangle, and writes the result to the target parameter
			*/
			bool intersectByRect(PortalRect &boundry, PortalRect &target)  {
				target.top = MIN(screenRect.top, boundry.top);
				target.bottom = MAX(screenRect.bottom, boundry.bottom);
				target.left = MAX(screenRect.left, boundry.left);
				target.right = MIN(screenRect.right, boundry.right);
				
				if (target.top < target.bottom)
					return false;
			
				if (target.right < target.left)
					return false;
				
				return true;
			}
			
			/**
			* Union the actual view rectangle with addition rectangle. Returns true on a view change.
			*/
			bool unionActualWithRect(PortalRect &addition) {
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

			/**
			* Debugging portal id setter
			*/
			void setPortalID(int id);
			
			/** Attaches the portal to the source and destination DarkSceneNodes */
			void attach();
			
			/** Mandatory override from SimpleRenderable */
			Real getSquaredViewDepth(const Camera* cam) const;
			/** Mandatory override from SimpleRenderable */
			virtual Real getBoundingRadius(void) const;
	};

	/** A bunch of the portal pointers. Is used in the DarkSceneNode class. */
	typedef std::set< Portal *> PortalList;
	
	/** An iterator over a PortalList */
	typedef std::set< Portal *>::const_iterator PortalListConstIterator;
}

#endif
