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

Rewritten to be used by the openDarkEngine project by Filip Volejnik <f.volejnik@centrum.cz>
*/
#ifndef __DarkSceneNode_H__
#define __DarkSceneNode_H__

#include "OgreBspPrerequisites.h"
#include "OgreSceneNode.h"
#include "OgrePortal.h"

namespace Ogre {
	
	/** Specialisation of SceneNode for the BspSceneManager.
	@remarks
	This specialisation of SceneNode is to enable information about the
	leaf node in which any attached objects are held is stored for
	use in the visibility determination. 
	@par
	Do not confuse this class with BspNode, which reflects nodes in the
	BSP tree itself. This class is just like a regular SceneNode, except that
	it should be locating BspNode leaf elements which objects should be included
	in. Note that because objects are movable, and thus may very well be overlapping
	the boundaries of more than one leaf, that it is possible that an object attached
	to one DarkSceneNode may actually be associated with more than one BspNode.
	*/
	class DarkSceneNode : public SceneNode {
		// So I'll save some time not calling methods to retrieve data...
		friend class DarkSceneManager;
		
		protected:
			/// Overridden from SceneNode
			void setInSceneGraph(bool inGraph);

			/** A vector of portals leading into this cell */
			PortalList mSrcPortals;
			/** A vector of portals leading out of this cell */
			PortalList mDstPortals;
		
			
			/* the last frame number this SceneNode was rendered. */
			unsigned int mFrameNum;
		
			/** Indicates the state of the cell. if the mFrameNum is not actual, the value of this boolean is not valid */
			bool mInitialized;
			
			/** Indicates the actual order position of this cell in the ActiveCells list. Invalid if the mFrameNum is not actual */
			size_t mListPosition;
		
			/** Cell ID. For Debugging purposes. */
			unsigned int mCellNum;
		
			/** Visitor map. Contains visitors not directly attached to this node, but seeable when this node gets rendered.
			* @note Please note that the setUserAny method of the MovableObject is used for the storage of the last frame number,
			* as this stops the _addToRenderQueue method from adding the movable twice (or even more) during one frame. */
			Ogre::SceneNode::ObjectMap mVisitorsByName;
		
		public:
			DarkSceneNode(SceneManager* creator);
			DarkSceneNode(SceneManager* creator, const String& name);
			
			/// Overridden from Node
			void _update(bool updateChildren, bool parentHasChanged);
			
			/** Detaches the indexed object from this scene node.
			@remarks
			    Detaches by index, see the alternate version to detach by name. Object indexes
			    may change as other objects are added / removed.
			*/
			MovableObject* detachObject(unsigned short index);

			/** Detaches the named object from this node and returns a pointer to it. */
			MovableObject* detachObject(const String& name);

			/** Detaches all objects attached to this node.
			*/
			void detachAllObjects(void);
					
			void _addToRenderQueue( Camera* cam, RenderQueue *queue, bool onlyShadowCasters );
			
			/** Attaches a Portal as a outgoing portal of this sceneNode
			*/
			void attachOutgoingPortal(Portal *portal);
			
			/** Attaches a Portal as a incomming portal of this sceneNode
			*/
			void attachIncommingPortal(Portal *portal);
			
			/** Sets the Cell number. For debugging */
			void setCellNum(unsigned int cellNum);
			
			/** gets the Cell number. For debugging */
			unsigned int getCellNum();
	};
}

#endif
