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

Rewritten from the BSPLevel class to use in the openDarkEngine project by Filip Volejnik <f.volejnik@centrum.cz>

$Id$

*/


#include "DarkBspTree.h"
#include "DarkSceneManager.h"

#include <OgreException.h>
#include <OgreMovableObject.h>
#include <OgreSceneManager.h>
#include <OgreMath.h>
#include <OgreStringVector.h>
#include <OgreStringConverter.h>
#include <OgreLogManager.h>
#include <OgreSceneManagerEnumerator.h>
#include <OgreIteratorWrappers.h>


namespace Ogre {

	//-----------------------------------------------------------------------
	BspTree::BspTree(DarkSceneManager* owner) : mRootNode(NULL), mOwner(owner) {
		// Nothing. Set-Null somethings!
	}

	//-----------------------------------------------------------------------
	BspTree::~BspTree() {
		clear();
	}

	//-----------------------------------------------------------------------
	const BspNode* BspTree::getRootNode(void)
	{
		return mRootNode;
	}
	
	//-----------------------------------------------------------------------
	void BspTree::setRootNode(BspNode* node)
	{
		mRootNode = node;
	}
	
	//-----------------------------------------------------------------------
	void BspTree::setRootNode(int id) {
		mRootNode = getNode(id);
	}
    
	//-----------------------------------------------------------------------
	BspNode* BspTree::findLeaf(const Vector3& point) const
	{
		BspNode* node = mRootNode;
		
		if (node == NULL)
			return NULL;

		while (!node->isLeaf()) {
			node = node->getNextNode(point);
			
			if (node == NULL) 
				return NULL;
		}

		return node;
	}
    
	//-----------------------------------------------------------------------
	void BspTree::_notifyObjectMoved(const MovableObject* mov, 
            const Vector3& pos)
	{

		// Locate any current nodes the object is supposed to be attached to
		MovableToNodeMap::iterator i = mMovableToNodeMap.find(mov);
		if (i != mMovableToNodeMap.end())
		{
			std::list<BspNode*>::iterator nodeit, nodeitend;
			nodeitend = i->second.end();
			for (nodeit = i->second.begin(); nodeit != nodeitend; ++nodeit)
			{
				// Tell each node
				(*nodeit)->_removeMovable(mov);
			}
			// Clear the existing list of nodes because we'll reevaluate it
			i->second.clear();
		}

		tagNodesWithMovable(mRootNode, mov, pos);
	}
    
	//-----------------------------------------------------------------------
	void BspTree::tagNodesWithMovable(BspNode* node, const MovableObject* mov,
		const Vector3& pos)
	{
	
		if (node->isLeaf())
		{
			// Add to movable->node map
			// Insert all the time, will get current if already there
			std::pair<MovableToNodeMap::iterator, bool> p = 
			mMovableToNodeMap.insert(
			MovableToNodeMap::value_type(mov, std::list<BspNode*>()));

			p.first->second.push_back(node);
			
			// Add movable to node
			node->_addMovable(mov);

		}
		else
		{
			// Find distance to dividing plane
			Real dist = node->getDistance(pos); 
			
			BspNode* front = node->getFront();
			BspNode* back = node->getBack();


			if (Math::Abs(dist) < mov->getBoundingRadius())
			{
				// Bounding sphere crosses the plane, do both
				if (back != NULL)
					tagNodesWithMovable(back, mov, pos);

				if (front != NULL)
					tagNodesWithMovable(front, mov, pos);
			}
			else if (dist < 0)
			{    //-----------------------------------------------------------------------
				// Do back
				if (back != NULL)
					tagNodesWithMovable(back, mov, pos);
			}
			else
			{
				// Do front
				if (front != NULL)
					tagNodesWithMovable(front, mov, pos);
			}
		}
	}
	
	//-----------------------------------------------------------------------
	void BspTree::_notifyObjectDetached(const MovableObject* mov)	
	{
		// Locate any current nodes the object is supposed to be attached to
		MovableToNodeMap::iterator i = mMovableToNodeMap.find(mov);
		if (i != mMovableToNodeMap.end())
		{
			std::list<BspNode*>::iterator nodeit, nodeitend;
			nodeitend = i->second.end();
			for (nodeit = i->second.begin(); nodeit != nodeitend; ++nodeit)
			{
				// Tell each node
				(*nodeit)->_removeMovable(mov);
			}
		// delete the entry for this MovableObject
		mMovableToNodeMap.erase(i);
		}
	}
	
	//-----------------------------------------------------------------------
	BspNode* BspTree::createNode(int id, int leafID) {
		BspNodeMap::const_iterator it = mBspNodeMap.find(id);
		
		if (it != mBspNodeMap.end()) 
			OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
                "BSPNode with the given id already exists!",
                "BspTree::createNode");
                
                
		BspNode* newNode = new BspNode(mOwner, id, leafID >= 0);
		
		mBspNodeMap.insert(std::make_pair(id, newNode));
		
		if (leafID >= 0) {
			mLeafNodeMap.insert(std::make_pair(leafID, newNode));
			newNode->setCellNum(leafID);
		}
		
		return newNode;
	}
		

	//-----------------------------------------------------------------------
	BspNode* BspTree::getNode(int id) {
		BspNodeMap::const_iterator it = mBspNodeMap.find(id);
		
		if (it != mBspNodeMap.end()) 
			return it->second;
		else
			OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
                "BSPNode with the given id does not exist!",
                "BspTree::getNode");
	}
	
	//-----------------------------------------------------------------------
	BspNode* BspTree::getLeafNode(int leafid) {
		BspNodeMap::const_iterator it = mLeafNodeMap.find(leafid);
		
		if (it != mLeafNodeMap.end()) 
			return it->second;
		else
			OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
                "BSPNode with the given leaf id does not exist!",
                "BspTree::getLeafNode");
	}

	//-----------------------------------------------------------------------
	void BspTree::clear() {
		mMovableToNodeMap.clear();
		
		BspNodeMap::const_iterator it = mBspNodeMap.begin();
		
		for (; it != mBspNodeMap.end(); ++it) {
			delete it->second;
		}
		
		mBspNodeMap.clear();
		mLeafNodeMap.clear();
		
		mRootNode = NULL;		
	}

	//-----------------------------------------------------------------------
	void BspTree::findLeafsForSphere(BspNodeList& destList, const Vector3& pos, Real radius) {
		findLeafsForSphereFromNode(mRootNode, destList, pos, radius);
	}
	
	//-----------------------------------------------------------------------
	void BspTree::findLeafsForSphereFromNode(BspNode* node, BspNodeList& destList, const Vector3& pos, Real radius) {
		// Much simmilar to TagNodesWithMovable
		if (node->isLeaf()) {
			destList.push_back(node);
		} else {
			// Find distance to dividing plane
			Real dist = node->getDistance(pos); 
			
			BspNode* front = node->getFront();
			BspNode* back = node->getBack();


			if (Math::Abs(dist) < radius)
			{
				// Bounding sphere crosses the plane, do both
				if (back != NULL)
					findLeafsForSphereFromNode(back, destList, pos, radius);

				if (front != NULL)
					findLeafsForSphereFromNode(front, destList, pos, radius);
			}
			else if (dist < 0)
			{    //-----------------------------------------------------------------------
				// Do back
				if (back != NULL)
					findLeafsForSphereFromNode(back, destList, pos, radius);
			}
			else
			{
				// Do front
				if (front != NULL)
					findLeafsForSphereFromNode(front, destList, pos, radius);
			}
		}
	}
}

