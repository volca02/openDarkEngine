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
 
#ifndef __DARKPORTAL_H
#define __DARKPORTAL_H

#include "config.h"

#include <OgreCamera.h>
#include <OgreSimpleRenderable.h>
#include <OgreVector3.h>
#include <OgrePlane.h>

#include "DarkPortalFrustum.h"
#include "DarkConvexPolygon.h"
#include "DarkBspPrerequisites.h"

#include <iostream>

#define MAX(a,b) ((a>b)?a:b)
#define MIN(a,b) ((a<b)?a:b)

namespace Ogre {
	/** @brief An int-type based portal rectangle struct 
	*
	* This structure is used by Portal class when calculating the on-screen bounding rectangle.
	* It is ment as a Ogre::Rect replacement, because that one uses Real as a base type, which is slower to use
	*/
	struct OPDELIB_EXPORT PortalRect {
			/// Static screen width size half
			static int sScreenWidth2;
			/// Static screen height size half
			static int sScreenHeight2;
		
			int left, right, bottom, top;
			
			// indicates a distance. is set to a minimal one on any operation
			float distance;
			
			static const PortalRect EMPTY;
			static const PortalRect SCREEN;
			
			PortalRect(int l, int r, int b, int t, float dist = 0) {
				left = l;
				right = r;
				bottom = b;
				top = t;
				distance = dist;
			}
			
			PortalRect() {
				left = EMPTY.left;
				right = EMPTY.right;
				bottom = EMPTY.bottom;
				top = EMPTY.top;
				distance = EMPTY.distance;
			}
			
			void setToScreen(void) {
			    left = SCREEN.left;
			    right = SCREEN.right;
			    top = SCREEN.top;
			    bottom = SCREEN.bottom;
			    distance = 0;
			}
			
			/// merges the portal rects to contain both this and given rect
			bool merge(const PortalRect& b) {
				bool changed = false;
					
				if (b.left < left) {
					left = b.left;
					changed = true;
				}
				
				if (b.right > right) {
					right = b.right;
					changed = true;
				}
				
				if (b.bottom < bottom) {
					bottom = b.bottom;
					changed = true;
				}
				
				if (b.top > top) {
					top = b.top;
					changed = true;
				}
				
				return changed;
			}
			
			/// intersects this and b rects, and returns true if intersection produced non-empty rect
			bool intersect(const PortalRect& b, PortalRect& result) const {
				result.top = MIN(b.top, top);
				result.bottom = MAX(b.bottom, bottom);
				result.left = MAX(b.left, left);
				result.right = MIN(b.right, right);
				// result.distance = MIN(b.distance, distance);
				// distance has to stay ours, otherwise the dist would propagate zero everywhere
				result.distance = distance;
				
				if (result.top < result.bottom)
					return false;
			
				if (result.right < result.left)
					return false;
				
				return true;
			}
			
			/// enlarges the rect to contain the given (screen projected) point
			void enlargeToContain(const Vector3 pnt) {
				// TODO: Parametrise the Screen width/height
				// NOTE: The X coord is flipped. This is because of the transform matrix, but it is not a problem for us
				int x = ((int)(pnt.x * sScreenWidth2) + sScreenWidth2); 
				int y = ((int)(pnt.y * sScreenHeight2) + sScreenHeight2); 
				
				top    = std::max(top, y);
				bottom = std::min(bottom, y);
				right  = std::max(right, x);
				left   = std::min(left, x);
				distance = std::min(distance, pnt.z);
			}
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
	class OPDELIB_EXPORT Portal : public ConvexPolygon { // , public SimpleRenderable 
		friend class DarkCamera;
		friend class PortalFrustum;
			
		protected:
       		/* The SimpleRenderable methods override. */
			void getWorldTransforms( Matrix4* xform ) const;
        		const Quaternion& getWorldOrientation(void) const;
        		const Vector3& getWorldPosition(void) const;
	
			void refreshPortalRenderable();
			/** Screen - space bounding rectangle of the portal */
			PortalRect	mScreenRect;  
		
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
			Portal(const Portal &src);
			
			/** Returns the target DarkSceneNode for this portal */
			BspNode* getTarget() const;
			
			/** Returns the source DarkSceneNode for this portal */
			BspNode* getSource() const;
		
			/** Refresh the center and radius bounding sphere parameters */
			void refreshBoundingVolume();
			
			/** Returns true if the portal is backface-culled for viewer on position pos */
			bool isBackfaceCulledFor(Vector3 pos) {
				Vector3 pos2p = mPoints[0] - pos; 
						
				float dotp = pos2p.dotProduct(mPlane.normal);
				
				return (dotp > 0);
			}
			
			/** Returns a distance this portal has from a given point */
			Real getDistanceFrom(Vector3 pos) {
				Vector3 diff = (pos - mCenter);
				return diff.length();
			}
			
			/**
			* Refreshes screen projection bounding Rectangle
			* @param cam Camera which is used by the projection
			* @param toScreen A precomputed Projection*View matrix matrix4 which is used to project the vertices to screen
			* @param cutp Plane to be used to cut the portal (cameras near plane, or better camera's view plane)
			* @return true if the result is non-empty
			*/
			bool refreshScreenRect(const Camera *cam, const Matrix4& toScreen, const Plane &cutp);
			
			/**
			* Refreshes screen projection bounding Rectangle
			* @param cam Camera which is used by the projection
			* @param toScreen A precomputed Projection*View matrix matrix4 which is used to project the vertices to screen
			* @param frust PortalFrustum used to cut away non visible parts of the portal
			*/
			void refreshScreenRect(const Camera *cam, const Matrix4& toScreen, const PortalFrustum &frust) {
				// inverse coords to let the min/max initialize
				mScreenRect = PortalRect::EMPTY;
				
				// Erase the actual rect
				mActualRect = mScreenRect;
				
				// Backface cull. The portal won't be culled if a vector camera-vertex dotproduct normal will be greater than 0
				Vector3 camToV0 = mPoints[0] - cam->getDerivedPosition(); 
						
				float dotp = camToV0.dotProduct(mPlane.normal);
				
				mPortalCull = (dotp > 0);
				
				// skip these expensive operations if we encounter a backface cull
				if (mPortalCull)
					return;
		
				// We also can cull away the portal if it is behind the camera's near plane. Can we? This needs distance calculation with the surrounding sphere
				
				// We have to cut the Portal using camera's frustum first, as this should solve the to screen projection problems...
				// the reason is that the coords that are at the back side or near the view point do not get projected right (that is right for our purpose)
				// it should be sufficient to clip by camera's plane (not near plane, too far away, just the plane that comes throught the camera's origin and has normal == view vector of camera)
				
				bool didc;
				
				Portal *onScreen = frust.clipPortal(this, didc);
				
				// If we have a non-zero cut result
				if (onScreen) {
					const PortalPoints& scr_points = onScreen->getPoints();
					
					PortalPoints::const_iterator it = scr_points.begin();
					PortalPoints::const_iterator pend = scr_points.end();
					// project all the vertices to screen space
					for (; it != pend; it++) {
						// This is one time-consuming line... I wonder how big eater this line is.
						Vector3 hcsPosition = toScreen * (*it);
						
						mScreenRect.enlargeToContain(hcsPosition);
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
				return mScreenRect.intersect(boundry, target);
			}
			
			/**
			* Union the actual view rectangle with addition rectangle. Returns true on a view change.
			*/
			bool unionActualWithRect(PortalRect &addition) {
				return mActualRect.merge(addition);
			}

			/**
			* Debugging portal id setter
			*/
			void setPortalID(int id);
			
			/** Attaches the portal to the source and destination DarkSceneNodes */
			void attach();
			
			/** Detaches the portal from the BSPNodes to which it was attached */
			void detach();
			
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
