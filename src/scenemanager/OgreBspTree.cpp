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
*/


#include "OgreBspTree.h"
#include "OgreException.h"
#include "OgreMovableObject.h"
#include "OgreSceneManager.h"
#include "OgreMath.h"
#include "OgreStringVector.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreSceneManagerEnumerator.h"
#include "OgreIteratorWrappers.h"


namespace Ogre {

	//-----------------------------------------------------------------------
	BspTree::BspTree() : mRootNode(0) {
		// Nothing. Set-Null somethings!
	}

	//-----------------------------------------------------------------------
	BspTree::~BspTree() {
		delete[] mRootNode;
	}

	//-----------------------------------------------------------------------
	const BspNode* BspTree::getRootNode(void)
	{
		return mRootNode;
	}
    
	//-----------------------------------------------------------------------
	BspNode* BspTree::findLeaf(const Vector3& point) const
	{
		BspNode* node = mRootNode;

		while (!node->isLeaf())
		{
			node = node->getNextNode(point);
			
			// if node is null we have a problem! We're out off world this time.
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
			Real dist = node->getDistance(pos); // The problem is that pos = (0,0,0)

			if (Math::Abs(dist) < mov->getBoundingRadius())
			{
				// Bounding sphere crosses the plane, do both
				if (node->getBack() != NULL)
					tagNodesWithMovable(node->getBack(), mov, pos);

				if (node->getFront() != NULL)
					tagNodesWithMovable(node->getFront(), mov, pos);
			}
			else if (dist < 0)
			{    //-----------------------------------------------------------------------
				// Do back
				if (node->getBack() != NULL)
					tagNodesWithMovable(node->getBack(), mov, pos);
			}
			else
			{
				// Do front
				if (node->getFront() != NULL)
					tagNodesWithMovable(node->getFront(), mov, pos);
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
	
	//----------------------------------------------------------------------
	void BspTree::setBspTree(BspNode* rootNode) {
		// TODO: This should only be allowed in the certain parts of the level initialization. Oh well. Maybe SceneManager's params would be the right place
		mRootNode = rootNode;
	}
}

