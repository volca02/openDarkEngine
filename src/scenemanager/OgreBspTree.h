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

Rewritten to be used in the openDarkEngine project by Filip Volejnik <f.volejnik@centrum.cz>
*/

#ifndef _BspTree_H__
#define _BspTree_H__

#include "OgreBspPrerequisites.h"
#include "OgreResource.h"
#include "OgreSceneManager.h"
#include "OgreBspNode.h"

namespace Ogre {

	/** BspTree handling class */
	class BspTree {
		friend class DarkSceneManager;
	public:
		/** Constructor. */
		BspTree();
	
		/** Destructor. Does unallocate the BSP tree */
		~BspTree();
	
		/** Returns a pointer to the root node (BspNode) of the BSP tree. */
		const BspNode* getRootNode(void);
	
		/** Walks the entire BSP tree and returns the leaf
		    which contains the given point.
		*/
		BspNode* findLeaf(const Vector3& point) const;
	
		/** Ensures that the MovableObject is attached to the right leaves of the 
		    BSP tree.
		*/
		void _notifyObjectMoved(const MovableObject* mov, 
		    const Vector3& pos);
		
		/** Internal method, makes sure an object is removed from the leaves when detached from a node. */
		void _notifyObjectDetached(const MovableObject* mov);
		
	protected:
		/** Pointer list for the nodes...
		*/
		BspNode* mRootNode;
		size_t	mNumNodes;
		
		typedef std::map<const MovableObject*, std::list<BspNode*> > MovableToNodeMap;
		/// Map for locating the nodes a movable is currently a member of
		MovableToNodeMap mMovableToNodeMap;

		void tagNodesWithMovable(BspNode* node, const MovableObject* mov, const Vector3& pos);
	
		/** Sets the BspTree root node to the tree root specified by root rootNode */
		void setBspTree(BspNode* rootNode);
	};
}

#endif
