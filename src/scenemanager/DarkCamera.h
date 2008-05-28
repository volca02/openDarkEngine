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
 
 
#ifndef __DARKCAMERA_H
#define __DARKCAMERA_H

#include "DarkBspPrerequisites.h"

#include <OgreCamera.h>


namespace Ogre {

	/** Camera specialized for BSP/Portal combination */
	class DarkCamera : public Camera {
			friend class DarkSceneManager;
		
		public:
			DarkCamera(const String& name, SceneManager* sm);
			
			~DarkCamera();
			
			virtual void _notifyMoved(void);
			
			/// internal method used to retrieve the visible node list
			const BspNodeList& _getVisibleNodes(void) const;
			
		protected:
			/// The camera's position has changed and need's a recalc (overriden from Frustum::updateFrustumImpl)
			virtual void updateFrustumImpl(void) const;
			
			virtual void updateViewImpl(void) const;
			
			void updateVisibleCellList() const;
		
			
			// The BSP tree used to update the visible cells
			BspTree* mBspTree;
			
			mutable BspNodeList mVisibleCells;
			
			mutable int mLastFrameNum;
	};
	
};

#endif
