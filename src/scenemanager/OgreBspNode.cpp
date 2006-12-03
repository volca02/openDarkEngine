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
*/

#include "OgreBspNode.h"
#include "OgreException.h"
#include "OgreLogManager.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    BspNode::BspNode(SceneManager* owner, bool isLeaf)
    {
        mOwner = owner;
        mIsLeaf = isLeaf;
	mSceneNode = NULL;
    }

    //-----------------------------------------------------------------------
    BspNode::BspNode() {
	mOwner = NULL;
        mIsLeaf = false;
	mSceneNode = NULL;
    }
    //-----------------------------------------------------------------------
    BspNode::~BspNode()
    {
    }

    //-----------------------------------------------------------------------
    bool BspNode::isLeaf(void) const
    {
        return mIsLeaf;
    }

    //-----------------------------------------------------------------------
    BspNode* BspNode::getFront(void) const
    {
        if (mIsLeaf)
            throw Exception(Exception::ERR_INVALIDPARAMS,
                "This method is not valid on a leaf node.",
                "BspNode::getFront");
        return mFront;
    }

    //-----------------------------------------------------------------------
    BspNode* BspNode::getBack(void) const
    {
        if (mIsLeaf)
            throw Exception(Exception::ERR_INVALIDPARAMS,
                "This method is not valid on a leaf node.",
                "BspNode::getBack");
        return mBack;
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
        if (sd == Plane::NEGATIVE_SIDE)
        {
            return getBack();
        }
        else
        {
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
    void BspNode::setSplitPlane(Plane splitPlane) {
	    mSplitPlane = splitPlane;
	    
    }
    
    //-----------------------------------------------------------------------
    void BspNode::setFrontChild(BspNode* frontChild) {
	    mFront = frontChild;
    }
    
    //-----------------------------------------------------------------------
    void BspNode::setBackChild(BspNode* backChild) {
	    mBack = backChild;
    }
    
    //-----------------------------------------------------------------------
    void BspNode::setOwner(SceneManager *owner) {
	    mOwner = owner;
    }
    
	//-------------------------------------------------------------------------
	void BspNode::attachOutgoingPortal(Portal *portal) {
		mDstPortals.insert(portal);
	}
			
	//-------------------------------------------------------------------------
	void BspNode::attachIncommingPortal(Portal *portal) {
		mSrcPortals.insert(portal);
	}
    
    	//-------------------------------------------------------------------------
	void BspNode::setCellNum(unsigned int cellNum) {
		mCellNum = cellNum;
	}
			
	//-------------------------------------------------------------------------
	unsigned int BspNode::getCellNum() {
		return mCellNum;
	}
	
	//-------------------------------------------------------------------------
	void BspNode::setFaceGroupStart(int fgs) {
		mFaceGroupStart = fgs;
	}
		
	//-------------------------------------------------------------------------
	void BspNode::setFaceGroupCount(int fgc) {
		mNumFaceGroups = fgc;
	}
}
