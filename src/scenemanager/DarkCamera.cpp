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
 
#include "OgreRoot.h"
#include "OgreTimer.h"
#include "DarkCamera.h"
#include "DarkBspNode.h"
#include "DarkBspTree.h"
#include "DarkSceneManager.h"

namespace Ogre {
	
	// ----------------------------------------------------------------------
	DarkCamera::DarkCamera(const String& name, SceneManager* sm) : Camera(name, sm), mIsDirty(false) {
		mBspTree = static_cast<DarkSceneManager*>(mSceneMgr)->getBspTree();
		mLastFrameNum = -1;
	}
	
	// ----------------------------------------------------------------------		
	DarkCamera::~DarkCamera() {
	}
	
	// ----------------------------------------------------------------------
	void DarkCamera::updateFrustumImpl(void) const {
		// first call the parent update, then do our stuff
		Camera::updateFrustumImpl();
		
		// If we're here, camera changed, so we need to update the cell list
		mIsDirty = true;
	}
	
	// ----------------------------------------------------------------------
	void DarkCamera::updateViewImpl(void) const {
			// first call the parent update, then do our stuff
		Camera::updateViewImpl();
		
		// If we're here, camera changed, so we need to update the cell list
		mIsDirty = true;
	}
	
	// ----------------------------------------------------------------------
	void DarkCamera::_notifyMoved(void) {
		Camera::_notifyMoved();
		
		mIsDirty = true;
	}
	
	// ----------------------------------------------------------------------
	const BspNodeList& DarkCamera::_getVisibleNodes(void) const {
		return mVisibleCells;
	}
	
	// ----------------------------------------------------------------------
	void DarkCamera::updateVisibleCellList() const {
		unsigned long startTime = Root::getSingleton().getTimer()->getMilliseconds();
		
		// Clear the cache of visible cells
		int frameNum = static_cast<DarkSceneManager*>(mSceneMgr)->mFrameNum;
		
		if (frameNum == mLastFrameNum && !mIsDirty) // nothing to do...
			return;
		
		// Look for our root cell
		BspNode* root = mBspTree->findLeaf(getDerivedPosition());
		
		if (root == NULL)  // out of world
			return;

		const Matrix4& viewM = getViewMatrix();
		const Matrix4& projM = getProjectionMatrix();

		Matrix4 toScreen = projM * viewM;
		
		// cut plane, with a small correction to avoid projection singularities
		Plane cutPlane(getRealDirection(), getRealPosition() + getRealDirection() * 0.01f);

		PortalFrustum cameraFrustum = PortalFrustum(this);

		root->invalidateScreenRect(frameNum);

		// Root cell gets whole screen visibility
		root->refreshScreenRect(this, toScreen, cameraFrustum);

		// the view rect is set to fill the screen
		root->mViewRect.setToScreen();

		// Root exists. traverse the portal tree
		BspNodeQueue cell_queue;
		
		cell_queue.push_back(root);

		unsigned int currentPos = 0;
		unsigned int finishedCells = 0;
		
		mVisibleCells.clear();
		mVisibleCells.push_back(root);
		
		while (finishedCells < cell_queue.size()) {
		    BspNode* cell = NULL;
		    
		    /*
		    float dist = 1e50;
		    
		    // select the cell with the least available distance from camera (to avoid cell re-evaluations)
		    for(unsigned int cell_idx = finishedCells; cell_idx < cell_queue.size(); ++cell_idx) {
				BspNode* c = cell_queue[cell_idx];
				
				if (c == NULL)
					continue;
				
				if (c->getScreenDist() < dist) {
					dist = c->getScreenDist();
					currentPos = cell_idx;
					cell = c;
				}
		    }
		    
		    // No matter which cell we'll process, the cell in the current finished position should be processed too
		    BspNode* s = cell_queue[finishedCells];
			cell_queue[currentPos] = s;
			cell_queue[finishedCells] = cell;
			finishedCells++;
			
		    if (cell == NULL) 
				break; // no more cells to process
				
		    */

		    cell = cell_queue[finishedCells++];
		    currentPos = finishedCells;
		    
		    if (cell == NULL)
				continue;
		        
		    if (cell->isVisBlocked())
				continue;
	
			
		    assert(cell->mInitialized && cell->mFrameNum == frameNum);
		    
		    // (Re-)evaluate the vision through all portals. If some of those changed, add the cells to the queue (or move those to top if already present)
		    BspNode::PortalIterator pi = cell->outPortalBegin();
		    BspNode::PortalIterator pend = cell->outPortalEnd();
		    
		    while (pi != pend) { // for all portals
				Portal* p = *(pi++);
				
				// Backface cull
				if (p->mPortalCull)
					continue;
					
				PortalRect tgt;
				bool changed = false;
				
				BspNode* target_cell = p->mTarget;	
				
				// Cell was not considered yet this frame, so invalidate the view inside
				if (target_cell->mFrameNum != frameNum)
					target_cell->invalidateScreenRect(frameNum);
				
				if (p->intersectByRect(cell->mViewRect, tgt)) { // Portal visible throught the cell's in-view
					if (p->unionActualWithRect(tgt)) { // A change in the view happened
						// only report change if the updated view changed
						changed = target_cell->updateScreenRect(tgt);
					}
				}
			
				// If a change happened, update the queue
				// Possible situation is that two cells are connected with two portals
				// In that case, we would end up having the cell added/moved twice
				// Because of this, the mInitialized flag exists, that differentiates the state
				// So the addition will update the frame num, and the next portal will detect the position is > current, and do nothing
				// or, in case the cell was in queue already, but moved, the queue position is then > current, and nothing is done
				if (changed) {
					// Update the queue
					if (!target_cell->mInitialized) {
						target_cell->refreshScreenRect(this, toScreen, cutPlane);

						// insert to the top
						cell_queue.push_back(target_cell);
						target_cell->mListPosition = cell_queue.size() - 1;
						
						mVisibleCells.push_back(target_cell);
					} else { 
					// move to the top if below current position
						if (finishedCells > target_cell->mListPosition) {
							// invalidate the current position, move to the new one
							cell_queue.push_back(target_cell);
							cell_queue[target_cell->mListPosition] = NULL; // NULL means a moved item (so we need not reorganize)
							target_cell->mListPosition = cell_queue.size() - 1;
						}
					}
				} // if changed
		    } // for all portals
		}
		
		mLastFrameNum = frameNum;
		mIsDirty = false;
		mCellCount = mVisibleCells.size();

		// All done!
		mTraversalTime = Root::getSingleton().getTimer()->getMilliseconds() - startTime;
	}
	
};
