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

Rewritten to use in the openDarkEngine project by Filip Volejnik <f.volejnik@centrum.cz>

$Id$

*/

#include "DarkBspNode.h"
#include "DarkLight.h"

#include <OgreException.h>
#include <OgreLogManager.h>


// cell flags for doorways
#define CELL_IS_DOORWAY 16
#define CELL_VIS_BLOCK 8
// ... fogging is bit 6, wireframe bit 1


namespace Ogre {

    //-----------------------------------------------------------------------
    BspNode::BspNode(SceneManager* owner, int id, int leafID, bool isLeaf) :
			mID(id),
			mLeafID(leafID),
			mFrameNum(0x0FFFF),
			mViewRect(PortalRect::EMPTY) {

        mOwner = owner;
        mIsLeaf = isLeaf;
		mSceneNode = NULL;

		// update the fragment as wellBspNode
		mCellFragment.fragmentType = SceneQuery::WFT_CUSTOM_GEOMETRY;
		mCellFragment.geometry = this;

		mInitialized = false;

		mFront = NULL;
		mBack = NULL;
    }

    //-----------------------------------------------------------------------
    BspNode::BspNode() : mViewRect(PortalRect::EMPTY) {
		mOwner = NULL;
		mIsLeaf = false;
		mSceneNode = NULL;

		// update the fragment as wellBspNode
		mCellFragment.fragmentType = SceneQuery::WFT_CUSTOM_GEOMETRY;
		mCellFragment.geometry = this;

		mFront = NULL;
		mBack = NULL;
    }

    //-----------------------------------------------------------------------
    BspNode::~BspNode()
    {
    }

    //-----------------------------------------------------------------------
    const Plane& BspNode::getSplitPlane(void) const
    {
        if (mIsLeaf)
            throw Exception(Exception::ERR_INVALIDPARAMS,
                "This method is not valid on a leaf node.",
                "BspNode::getSplitPlane");

        return mSplitPlane;

    }

    //-----------------------------------------------------------------------
    const AxisAlignedBox& BspNode::getBoundingBox(void) const
    {
        if (!mIsLeaf)
            throw Exception(Exception::ERR_INVALIDPARAMS,
                "This method is only valid on a leaf node.",
                "BspNode::getBoundingBox");

        return mBounds;
    }

    //-----------------------------------------------------------------------
    Plane::Side BspNode::getSide (const Vector3& point) const
    {
        if (mIsLeaf)
            throw Exception(Exception::ERR_INVALIDPARAMS,
                "This method is not valid on a leaf node.",
                "BspNode::getSide");

        return mSplitPlane.getSide(point);
    }

    //-----------------------------------------------------------------------
    BspNode* BspNode::getNextNode(const Vector3& point) const
    {

        if (mIsLeaf)
            throw Exception(Exception::ERR_INVALIDPARAMS,
                "This method is not valid on a leaf node.",
                "BspNode::getNextNode");

        Plane::Side sd = getSide(point);

        if (sd == Plane::NEGATIVE_SIDE) {
            return getBack();
        } else {
            return getFront();
        }
    }

    //-----------------------------------------------------------------------
    void BspNode::_addMovable(const MovableObject* mov)
    {
        mMovables.insert(mov);
    }

    //-----------------------------------------------------------------------
    void BspNode::_removeMovable(const MovableObject* mov)
    {
        mMovables.erase(mov);
    }

    //-----------------------------------------------------------------------
    Real BspNode::getDistance(const Vector3& pos) const
    {
        if (mIsLeaf)
            throw Exception(Exception::ERR_INVALIDPARAMS,
                "This method is not valid on a leaf node.",
                "BspNode::getSide");

        return mSplitPlane.getDistance(pos);

    }

    //-----------------------------------------------------------------------
    std::ostream& operator<< (std::ostream& o, BspNode& n)
    {
        o << "BspNode(";
        if (n.mIsLeaf)
        {
            o << "leaf, bbox=" << n.mBounds << ")";
        }
        else
        {
            o <<  "splitter, plane=" << n.mSplitPlane << ")";
        }
        return o;

    }

    //-----------------------------------------------------------------------
    SceneNode* BspNode::getSceneNode() {
	    if (!mIsLeaf)
			throw Exception(Exception::ERR_INVALIDPARAMS,
				"This method is not valid on a non-leaf node.",
				"BspNode::getSceneNode");

	    return mSceneNode;
    }

    //-----------------------------------------------------------------------
    void BspNode::setSceneNode(SceneNode *node) {
	    if (!mIsLeaf)
			throw Exception(Exception::ERR_INVALIDPARAMS,
				"This method is not valid on a non-leaf node.",
				"BspNode::setSceneNode");

	    mSceneNode = node;
    }

    //-----------------------------------------------------------------------
    void BspNode::setIsLeaf(bool isLeaf) {
	    mIsLeaf = isLeaf;
    }

    //-----------------------------------------------------------------------
    void BspNode::setSplitPlane(const Plane& splitPlane) {
    	if (mIsLeaf)
			throw Exception(Exception::ERR_INVALIDPARAMS,
				"This method only valid on a non-leaf node.",
				"BspNode::setSplitPlane");

	    mSplitPlane = splitPlane;
    }

    //-----------------------------------------------------------------------
    void BspNode::setFrontChild(BspNode* frontChild) {
    	if (mIsLeaf)
			throw Exception(Exception::ERR_INVALIDPARAMS,
				"This method is only valid on a non-leaf node.",
				"BspNode::setFrontChild");

	    mFront = frontChild;
    }

    //-----------------------------------------------------------------------
    void BspNode::setBackChild(BspNode* backChild) {
    	if (mIsLeaf)
			throw Exception(Exception::ERR_INVALIDPARAMS,
				"This method is only valid on a non-leaf node.",
				"BspNode::setBackChild");

	    mBack = backChild;
    }

    //-----------------------------------------------------------------------
    void BspNode::setOwner(SceneManager *owner) {
	    mOwner = owner;
    }

	//-------------------------------------------------------------------------
	void BspNode::attachOutgoingPortal(Portal *portal) {
		if (!mIsLeaf)
			throw Exception(Exception::ERR_INVALIDPARAMS,
				"This method is not valid on a non-leaf node.",
				"BspNode::attachOutgoingPortal");

		mDstPortals.insert(portal);
	}

	//-------------------------------------------------------------------------
	void BspNode::attachIncommingPortal(Portal *portal) {
		if (!mIsLeaf)
			throw Exception(Exception::ERR_INVALIDPARAMS,
				"This method is not valid on a non-leaf node.",
				"BspNode::attachIncommingPortal");

		mSrcPortals.insert(portal);
	}

	//-------------------------------------------------------------------------
	void BspNode::detachPortal(Portal* portal) {
	    mSrcPortals.erase(portal);
	    mDstPortals.erase(portal);
	}

    	//-------------------------------------------------------------------------
	void BspNode::setCellNum(unsigned int cellNum) {
		if (!mIsLeaf)
			throw Exception(Exception::ERR_INVALIDPARAMS,
				"This method is not valid on a non-leaf node.",
				"BspNode::getSceneNode");

		mCellNum = cellNum;
	}

	//-------------------------------------------------------------------------
	unsigned int BspNode::getCellNum() const {
		return mCellNum;
	}


	//-------------------------------------------------------------------------
	void BspNode::setPlaneList(BspNode::CellPlaneList& planes, BspNode::PlanePortalMap& portalmap) {
		mPlaneList = planes;
		mPortalMap = portalmap;
	}


	//-------------------------------------------------------------------------
	void BspNode::blockVision(bool block) {
		unsigned int mask;
		unsigned int addition;
		
		if (!block) {
			mask = ~0;
			addition = CELL_VIS_BLOCK;
		} else { // unblock
			mask = ~CELL_VIS_BLOCK;
			addition = 0;
		}
		
		testAndSetDistributed(CELL_IS_DOORWAY, mask, addition);
	}

	//-------------------------------------------------------------------------
	bool BspNode::isVisBlocked() {
		return mCellFlags & CELL_VIS_BLOCK;
	}

	//-------------------------------------------------------------------------
	void BspNode::_setCellFlags(unsigned int flags) {
		mCellFlags = flags;
	}
	
	//-------------------------------------------------------------------------	
	void BspNode::testAndSetDistributed(unsigned int prereq, unsigned int mask, unsigned int addition) {
		if (!(mCellFlags & prereq))
			return;
			
		unsigned int prev = mCellFlags;
		
		mCellFlags &= mask;
		mCellFlags |= addition;
		
		if (mCellFlags != prev) { // flags changed
			PortalListConstIterator outPortal_it = mDstPortals.begin();
			PortalListConstIterator end = mDstPortals.end();
	
			for (; outPortal_it != end; outPortal_it++) {
				Portal *out_portal = (*outPortal_it);
	
				out_portal->getTarget()->testAndSetDistributed(prereq, mask, addition);
			}
		}
	}


	//-------------------------------------------------------------------------
	void BspNode::refreshScreenRect(const Camera* cam, const Matrix4& toScreen, const PortalFrustum& frust) {
		mInitialized = true;

		// Iterate all portals and refresh the view rect of those
		PortalListConstIterator outPortal_it = mDstPortals.begin();
		PortalListConstIterator end = mDstPortals.end();

		for (; outPortal_it != end; outPortal_it++) {
			Portal *out_portal = (*outPortal_it);

			out_portal->refreshScreenRect(cam, toScreen, frust);
		}
	}

	//-------------------------------------------------------------------------
	void BspNode::refreshScreenRect(const Camera* cam, const Matrix4& toScreen, const Plane& cutp) {
		mInitialized = true;

	    // Iterate all portals and refresh the view rect of those
	    PortalListConstIterator outPortal_it = mDstPortals.begin();
		PortalListConstIterator end = mDstPortals.end();

		for (; outPortal_it != end; outPortal_it++) {
			Portal *out_portal = (*outPortal_it);

			out_portal->refreshScreenRect(cam, toScreen, cutp);
		}
	}

	//-------------------------------------------------------------------------
	void BspNode::addAffectingLight(DarkLight* light) {
		mAffectingLights.insert(light);

		if (light->isDynamic())
			mDynamicLights.insert(light);
	}

	//-------------------------------------------------------------------------
	void BspNode::removeAffectingLight(DarkLight* light) {
		mAffectingLights.erase(light);
		mDynamicLights.erase(light);
	}
}
