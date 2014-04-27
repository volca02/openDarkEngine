/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------

Rewritten to be used in openDarkEngine project by Filip Volejnik <f.volejnik@centrum.cz>

$Id$
*/

#ifndef __DARKBSPNODE_H
#define __DARKBSPNODE_H

#include "config.h"

#include "DarkBspPrerequisites.h"
#include "DarkPortal.h"

#include <OgrePlane.h>
#include <OgreAxisAlignedBox.h>
#include <OgreSceneQuery.h>

#include <vector>

namespace Ogre {

    /** Encapsulates a node in a BSP tree.
        A BSP tree represents space partitioned by planes . The space which is
        partitioned is either the world (in the case of the root node) or the space derived
        from their parent node. Each node can have elements which are in front or behind it, which are
        it's children and these elements can either be further subdivided by planes,
        or they can be undivided spaces or 'leaf nodes' - these are the nodes which actually contain
        objects and world geometry. The leaves of the tree are the stopping point of any tree walking algorithm,
        both for rendering and collision detection etc.</p>
        Ogre chooses not to represent splitting nodes and leaves as separate structures, but to merge the two for simplicity
        of the walking algorithm. If a node is a leaf, the isLeaf() method returns true and both getFront() and
        getBack() return null pointers. If the node is a partitioning plane isLeaf() returns false and getFront()
        and getBack() will return the corresponding BspNode objects.

	This version of BspNode, updated to be used by DarkSceneManager, implements Portals as members. It should be considered as a 'Cell' if it is a leaf.
    */
    class OPDELIB_EXPORT BspNode {
   		friend class DarkSceneManager;
		friend class DarkCamera;
		friend class BspRaySceneQuery;
		friend class BspIntersectionSceneQuery;
		friend class BspTree;

	public:
		BspNode(SceneManager* owner, int id, int leafID, bool isLeaf);

		BspNode();
		~BspNode();

		/** Returns true if this node is a leaf (i.e. contains geometry) or false if it is a splitting plane.
		    A BspNode can either be a splitting plane (the typical representation of a BSP node) or an undivided
		    region contining geometry (a leaf node). Ogre represents both using the same class for simplicity
		    of tree walking. However it is important that you use this method to determine which type you are dealing
		    with, since certain methods are only supported with one of the subtypes. Details are given in the individual methods.
		    Note that I could have represented splitting / leaf nodes as a class hierarchy but the
		    virtual methods / run-time type identification would have a performance hit, and it would not make the
		    code much (any?) simpler anyway. I think this is a fair trade-off in this case.
		*/
		inline bool isLeaf(void) const { return mIsLeaf; };

		/** Returns a pointer to a BspNode containing the subspace on the positive side of the splitting plane.
		    This method should only be called on a splitting node, i.e. where isLeaf() returns false. Calling this
		    method on a leaf node will throw an exception.
		*/
		inline BspNode* getFront(void) const { assert(!mIsLeaf); return mFront; };

		/** Returns a pointer to a BspNode containing the subspace on the negative side of the splitting plane.
		    This method should only be called on a splitting node, i.e. where isLeaf() returns false. Calling this
		    method on a leaf node will throw an exception.
		*/
		BspNode* getBack(void) const { assert(!mIsLeaf); return mBack; };

		/** Determines which side of the splitting plane a worldspace point is.
		    This method should only be called on a splitting node, i.e. where isLeaf() returns false. Calling this
		    method on a leaf node will throw an exception.
		*/
		Plane::Side getSide (const Vector3& point) const;

		/** Gets the next node down in the tree, with the intention of
		    locating the leaf containing the given point.
		    This method should only be called on a splitting node, i.e. where isLeaf() returns false. Calling this
		    method on a leaf node will throw an exception.
		*/
		BspNode* getNextNode(const Vector3& point) const;


		/** Returns details of the plane which is used to subdivide the space of his node's children.
		    This method should only be called on a splitting node, i.e. where isLeaf() returns false. Calling this
		    method on a leaf node will throw an exception.
		*/
		const Plane& getSplitPlane(void) const;

		/** Returns the axis-aligned box which contains this node if it is a leaf.
		    This method should only be called on a leaf node. It returns a box which can be used in calls like
		    Camera::isVisible to determine if the leaf node is visible in the view.
		*/
		const AxisAlignedBox& getBoundingBox(void) const;

		friend std::ostream& operator<< (std::ostream& o, BspNode& n);

		/// Internal method for telling the node that a movable intersects it
		void _addMovable(const MovableObject* mov);

		/// Internal method for telling the node that a movable no longer intersects it
		void _removeMovable(const MovableObject* mov);

		/// Gets the signed distance to the dividing plane
		Real getDistance(const Vector3& pos) const;

		/** A set of MovableObjects intersecting this Node(e.g. visible in the cell) */
		typedef std::set< const MovableObject* > IntersectingObjectSet;

		/** Returns a reference to the scene node representing this leaf. Throws exception on the non-leaf nodes. */
		SceneNode* getSceneNode();

		/** Sets the associated scene node. That is the scene node represented by this leaf */
		void setSceneNode(SceneNode *node);

		// BspNode parameter settings. As we want to construct the BspNodes Externally...
		/** Sets the Node to be leaf or not */
		void setIsLeaf(bool isLeaf);

		/** Sets the splitting plane of this node */
		void setSplitPlane(const Plane& splitPlane);

		/** Sets the Front child of this node */
		void setFrontChild(BspNode* frontChild);

		/** Sets the back child of this node */
		void setBackChild(BspNode* backChild);

		/** Sets the owning SceneManager instance */
		void setOwner(SceneManager *owner);

		/** Attaches a Portal as an outgoing portal of this BSPNode */
		void attachOutgoingPortal(Portal *portal);

		/** Attaches a Portal as an incomming portal of this BSPNode */
		void attachIncommingPortal(Portal *portal);

		/** Dettaches a Portal from this BSPNode	*/
		void detachPortal(Portal *portal);

		/** Informs this node a light affect's it */
		void addAffectingLight(DarkLight* light);

		/** Informs this node a light stopped affecting it */
		void removeAffectingLight(DarkLight* light);

		/** Sets the Cell number. For debugging */
		void setCellNum(unsigned int cellNum);

		/** gets the Cell number. For debugging */
		unsigned int getCellNum() const;

		/** Plane list. For Scene queries */
		typedef std::list<Plane> CellPlaneList;

		/** Plane portal map (index of the CellPlaneList) to the portal set */
		typedef std::map<int, PortalList > PlanePortalMap;


		/** sets the plane list for scene queries */
		void setPlaneList(CellPlaneList& planes, PlanePortalMap& portalmap);

		void refreshScreenRect(const Camera* cam, const Matrix4& toScreen, const PortalFrustum& frust);
		void refreshScreenRect(const Camera* cam, const Matrix4& toScreen, const Plane& cutp);

		typedef PortalList::iterator PortalIterator;

		PortalIterator outPortalBegin() { return mDstPortals.begin(); };
		PortalIterator outPortalEnd() { return mDstPortals.end(); };

		// Invalidates the screen projection info (so refreshScreenRect will pass)
		void invalidateScreenRect(int frameNum) { mInitialized = false; mFrameNum = frameNum; mViewRect = PortalRect::EMPTY; };

		inline int getLeafID(void) const { return mLeafID; };


		/// set of affecting lights for this cell (leaf only)
		typedef std::set< DarkLight* > AffectingLights;
		typedef AffectingLights::iterator LightIterator;

		LightIterator lightsBegin() { return mAffectingLights.begin(); };
		LightIterator lightsEnd() { return mAffectingLights.end(); };

		LightIterator dynamicLightsBegin() { return mDynamicLights.begin(); };
		LightIterator dynamicLightsEnd() { return mDynamicLights.end(); };

		// VisBlocking code follows
		void blockVision(bool block);

		bool isVisBlocked();

		/** Internal routine for flag settings - only to be called when initializing the cell */
		void _setCellFlags(unsigned int flags);

		/** Unique sequential BSP node id */
		int getID() const { return mID; }

	protected:
		/** Sets and distributes the given cell flag change across portals.
		* This method will
		* a) test the precondition, and if in does not apply, exit immediately
		* a) Test if the given flag is already set or not
		* b) if not, it will be set, and all portal connected targets will be called with the same parameters
		*
		* @param prereq The prerequisite for the test to happen - the and operation of the cell's flags with this must be nonzero
		* @param mask The mask to apply - cell flags will be and-ed with this parameter
		* @param addition The additional bits to set - cell flags will be or-ed with this parameter after the prev. masking
		*/
		void testAndSetDistributed(unsigned int prereq, unsigned int mask, unsigned int addition);

		/// ID of the BSP row (order)
		int mID;
		/// ID of the leaf (cell id)
		int mLeafID;

		/// Owner Scene Manager - Back-reference to SceneManager which owns this Node
		SceneManager* mOwner;

		/// Leaf indicator - true on non-split cell-containing nodes
		bool mIsLeaf;

		/** Pointer to the scene node which is represented by the leaf */
		SceneNode *mSceneNode;

		// Node-only members
		/** The plane which splits space in a non-leaf node.
		    Note that nodes do not allocate the memory for other nodes - for simplicity and bulk-allocation
		    of memory the BspLevel is responsible for assigning enough memory for all nodes in one go.
		*/
		Plane mSplitPlane;

		/** Pointer to the node in front of this non-leaf node. */
		BspNode* mFront;

		/** Pointer to the node behind this non-leaf node. */
		BspNode* mBack;

		// Leaf-only members

		/** The axis-aligned box which bounds node if it is a leaf. */
		AxisAlignedBox mBounds;

		/** The set of movables that could be visible in this BSP node, and thus have to be rendered when this node is visible */
		IntersectingObjectSet mMovables;


		/// all lights affecting this cell
		AffectingLights mAffectingLights;

		/// dynamic lights affecting this cell
		AffectingLights mDynamicLights;

		// ----------- Portal based rendering stuff - leaf only

		/** A vector of portals leading into this cell */
		PortalList mSrcPortals;

		/** A vector of portals leading out of this cell */
		PortalList mDstPortals;

		/** the last frame number this (leaf) node was rendered. */
		unsigned int mFrameNum;

		/** Indicates the state of the cell. if the mFrameNum is not actual, the value of this boolean is not valid */
		bool mInitialized;

		/** Indicates the actual order position of this cell in the ActiveCells list. Invalid if the mFrameNum is not actual */
		size_t mListPosition;

		/** Cell ID. For Debugging purposes. */
		unsigned int mCellNum;

		/** Current view rectangle to this cell */
		PortalRect mViewRect;

		/** cell's plane list for Leaf nodes */
		CellPlaneList mPlaneList;

		/** The Plane index to Portal set map */
		PlanePortalMap mPortalMap;

		// For acceleration, we prepare a world fragment too (WFT_PLANE_BOUNDED_REGION)
		/** World fragment if someone wants the cell as a result from the query. - a pre-prepared fragment containing the cell */
		SceneQuery::WorldFragment mCellFragment;

		/** Cell Flags - fogging, vis blocking, doorways, wireframe settings */
		unsigned int mCellFlags;

		/** Enlarge the view rect to the cell to accompany the given view rect */
		inline bool updateScreenRect(const PortalRect& tgt) {
			// merge the view rect to accompany the new rect
			if (mViewRect.merge(tgt)) {
				// if a view changed, reconsider the minimal distance
				mViewRect.distance = std::min(mViewRect.distance, tgt.distance);
				return true;
			}

			return false;
		};

		public:
			const IntersectingObjectSet& getObjects(void) const { return mMovables; }
			const CellPlaneList& getPlaneList() const { return mPlaneList; }
			float getScreenDist(void) const { return mViewRect.distance; };
    };

}

#endif
