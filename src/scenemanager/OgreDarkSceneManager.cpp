/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

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

#include "OgreDarkSceneManager.h"
#include "OgreBspNode.h"
#include "OgreException.h"
#include "OgreRenderSystem.h"
#include "OgreCamera.h"
#include "OgreMaterial.h"
#include "OgrePatchSurface.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreMath.h"
#include "OgreMaterialManager.h"
#include "OgreControllerManager.h"
#include "OgreLogManager.h"
#include "OgreDarkSceneNode.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreTechnique.h"
#include "OgrePass.h"

#include <vector>
#include <fstream>

// Traversal logging to stdout. Change to a ConsoleCommandListener command : traversal_log filename.txt
// #define debug
//#define cellist

namespace Ogre {

	//-----------------------------------------------------------------------
	DarkSceneManager::DarkSceneManager(const String& instanceName) : SceneManager(instanceName) {
		// Set features for debugging render
		mShowNodeAABs = true;

		// No sky by default
		mSkyPlaneEnabled = false;
		mSkyBoxEnabled = false;
		mSkyDomeEnabled = false;

		mActualFrame = 1;
	    
		firstTime = true;
		
		mBspTree = NULL;
	}

	//-----------------------------------------------------------------------
	DarkSceneManager::~DarkSceneManager() {
		// TODO: freeMemory();
	}
    
	//-----------------------------------------------------------------------
	const String& DarkSceneManager::getTypeName(void) const
	{
		return DarkSceneManagerFactory::FACTORY_TYPE_NAME;
	}
	
	//-----------------------------------------------------------------------
	size_t DarkSceneManager::estimateWorldGeometry(const String& filename) {
		return 0; // No internal format
		// return BspLevel::calculateLoadingStages(filename);
	}
    
	//-----------------------------------------------------------------------
	void DarkSceneManager::setWorldGeometry(const String& filename) {
		// As we're not doing any internal loading, we throw unsupported exception
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
			"World geometry is not supported by the DarkSceneManager. Use the supplied methods to inject it for now.",
			"SceneManager::setWorldGeometry");
	}
    
	//-----------------------------------------------------------------------
	void DarkSceneManager::_findVisibleObjects(Camera* cam, bool onlyShadowCasters) {
		// Erase the render queue
		RenderQueue* renderQueue = getRenderQueue();
		
		renderQueue->clear();
		
		// Clear unique list of movables for this frame
		mMovablesForRendering.clear();
	    
		// Walk the tree, tag static geometry, return camera's node (for info only)
		walkTree(cam, renderQueue, onlyShadowCasters);
	}
    
	//-----------------------------------------------------------------------
	/**
	* Prepares a cell for the traversal to happen.
	*/
	void DarkSceneManager::prepareCell(DarkSceneNode *cell, Camera *camera, PortalFrustum *frust) { 
		PortalListConstIterator outPortal_it = cell->mDstPortals.begin();
		
		for (;outPortal_it != cell->mDstPortals.end(); outPortal_it++) {
			Portal *out_portal = (*outPortal_it);

			out_portal->calculateDistance(camera);
			out_portal->refreshScreenRect(camera, frust);
			
			out_portal->mMentions = 0; 
		}
		
		cell->mInitialized = true;
		cell->mFrameNum = mActualFrame;
	}
	
	//-----------------------------------------------------------------------
	/**
	* The core rendering method. Processes the scene for visibility
	* @todo This method needs some refactoring to make it readable
	*/
	void DarkSceneManager::traversePortals(DarkSceneNode *cell, Camera* camera, RenderQueue *queue, Rectangle &viewrect, bool onlyShadowCasters) {
		#ifdef debug
		std::cerr << "traversal Starting" << std::endl;		
		#endif
		
		PortalFrustum *cameraFrustum = new PortalFrustum(camera);
		
		mActiveCells.clear();
		
		// prepare the cell
		prepareCell(cell, camera, cameraFrustum);
		mActiveCells.push_back(cell);
		mActualPosition = 0;
		cell->mListPosition = 0;
		cell->mFrameNum = mActualFrame; 
		
		PortalListConstIterator outPortal_it = cell->mDstPortals.begin();
		
		for (;outPortal_it != cell->mDstPortals.end(); outPortal_it++) {
			Portal *outportal = (*outPortal_it);
			
			outportal->mMentions = 0; 
			
			if  (outportal->mDotProduct > 0) {
				#ifdef debug
				std::cerr << "  * MOTHER CELL: backface cull for outcell " << outportal->mTarget->mCellNum << std::endl;
				#endif				
				
				continue;
			}

			Rectangle tgt;
				
			if (outportal->intersectByRect(viewrect, tgt)) {
				outportal->unionActualWithRect(tgt); // so we have non-zero actual rect for that portal
				
				// mark the visible portal
				outportal->mMentions = 1; 
			} 
			#ifdef debug
			else {
				std::cerr << "  * MOTHER CELL: offscreen for " << outportal->mTarget << std::endl;
			}
			#endif
		}
		
		
		
		while (mActualPosition < mActiveCells.size()) {
			// get one cell
			#ifdef debug
			std::cerr << " * Actual position " << mActualPosition << std::endl;
			#endif
			
			DarkSceneNode *actual = mActiveCells.at(mActualPosition);

			#ifdef debug
			std::cerr << "    - with cell number " << actual->mCellNum << std::endl;
			#endif
			
			if (!actual->mInitialized)
				prepareCell(actual, camera, cameraFrustum);
			
			int chcount = 0; // change count
			
			// now that we have the cell prepared, process the outgoing portals with input views
			PortalListConstIterator tgtPortal_it = actual->mDstPortals.begin();
		
			for (;tgtPortal_it != actual->mDstPortals.end(); tgtPortal_it++) {
				Portal *target_portal = *tgtPortal_it;	

				#ifdef debug
				std::cerr << "  * test output portal " << target_portal->mPortalID << " to cell " << target_portal->mTarget->mCellNum << std::endl;
				#endif				
				
				if  (target_portal->mDotProduct > 0) { // backface cull
					#ifdef debug
					std::cerr << "    * backface cull for " << target_portal->mPortalID << " to cell " << target_portal->mTarget->mCellNum << std::endl;

					#endif				

					continue;
				}

				// now process it with all the input portals mentioned
				PortalListConstIterator src_portal_it = actual->mSrcPortals.begin();
				
				bool changed = false;
				
				for (;src_portal_it != actual->mSrcPortals.end(); src_portal_it++) {
					Portal *source = *src_portal_it;

					#ifdef debug
					std::cerr << "    * against src. " << source->mPortalID << std::endl;
					#endif
					
					// If the source cell was not yet initialized, we'll skip the processing against this portal
					// it should be ok, since we will not lower the mention number, and if the source cell will be traversed
					// we'll get here again with the right mentions situation
					DarkSceneNode *source_cell = source->mSource;
					
					if ((!source_cell->mInitialized) || (source_cell->mFrameNum != mActualFrame)) 
						continue;

					// Ok. The source cell was initialized
					
					if (source->mMentions > 0) { // reevaluation is needed
						#ifdef debug
						std::cerr << "     * Mention on portal, reconsidering" << std::endl; 
						#endif
						
						// clip the src_portals view to the target portal
						
						Rectangle tgt; // the rectangle which represents the actual computed src to target portal intersection
						
						if (target_portal->intersectByRect(source->mActualRect, tgt)) {
							#ifdef debug
							std::cerr << "   - nonzarro portal " << source->mPortalID << " intersection" << std::endl;
							#endif

							
							if (target_portal->unionActualWithRect(tgt)) {
								#ifdef debug
								std::cerr << "   - outgoing portal " << source->mPortalID << " changed." << std::endl;
								#endif
								changed = true; 
								chcount++;
							}
						}
					} // if portal is mentioned
				} // for all mentioned input portals
				
				// end of portal reevaluation
				
				if (changed && target_portal->mMentions < 1) 
					target_portal->mMentions++;
				
				if (target_portal->mMentions > 0) { // the view to the target has changed
					#ifdef debug
					std::cerr << "    * a change in portal view occured " << target_portal->mPortalID << std::endl;
					#endif
					
					DarkSceneNode *target_cell = target_portal->mTarget;
					
					// if the cell was not yet added
					if (target_cell->mFrameNum != mActualFrame) {
						#ifdef debug					
						std::cerr << "   - adding cell " << target_cell->mCellNum << std::endl;
						#endif						
						
						target_cell->mInitialized = false; // invalidate the prepareness state
						
						mActiveCells.push_back(target_cell);
						target_cell->mListPosition = mActiveCells.size()-1;
						
						target_cell->mFrameNum = mActualFrame;
					} else {
						// we'll have to move the cell to the end if it has a position less than the actual cell
						#ifdef debug						
						std::cerr << "   - requeue? " << target_cell->mListPosition <<  " < " << mActualPosition << std::endl;
						#endif	
						
						if (target_cell->mListPosition < mActualPosition && target_cell->mListPosition != 0) { // We do not want to requeue the mother cell (Hey, this is a cycle isn't it?)
							#ifdef debug
							std::cerr << "   - requeueing cell " << target_cell->mCellNum << std::endl;
							#endif
							
							// change the ListPosition for all succesive cells
							std::vector< DarkSceneNode *>::iterator cell_it = mActiveCells.begin() + target_cell->mListPosition + 1;
							
							for (;cell_it != mActiveCells.end(); cell_it++)
								(*cell_it)->mListPosition--;
							
							// requeue the cell
							mActiveCells.erase(mActiveCells.begin() + target_cell->mListPosition);
							
							mActiveCells.push_back(target_cell);
							target_cell->mListPosition = mActiveCells.size() - 1; // we put it to the end	
							
							// testing if the requeue works....
							assert(mActiveCells.at(mActiveCells.size() -1 )->mCellNum == target_cell->mCellNum);
							
							mActualPosition--;
						}
					} // if cell not actual
				} // if changed
			} // for portals
			
			#ifdef debug
			if (chcount == 0)
				std::cerr << "  ! No change occured!" << std::endl;
			#endif
			
			// now unmention all the incoming portals
			// I have to do it here because directly unmentioning would destroy the portal vis testing...
			PortalListConstIterator src_portal_it = actual->mSrcPortals.begin();

			
			for (;src_portal_it != actual->mSrcPortals.end(); src_portal_it++) {
				if ((*src_portal_it)->mMentions > 0)
					(*src_portal_it)->mMentions--;
			}

			
			mActualPosition++;
		} // while mActualPosition

		#ifdef debug		
		std::cerr << "Finished the traversal..." << std::endl;
		#endif
		
		// Get rid of our frustum
		delete cameraFrustum;
		
		// TODO: No need to order back-to-front now, the rendering in underlying sceneManager will handle this
		std::vector<DarkSceneNode *>::reverse_iterator it = mActiveCells.rbegin();
		
		#ifdef cellist
		std::cerr << cell->mCellNum << ":";
		#endif
		
		for (;it != mActiveCells.rend(); it++) {
			#ifdef cellist
			std::cerr << (*it)->mCellNum << " ";
			#endif
			
			// Add the visible scene node to the render queue. This adds all the movables in the sceneNode to the Queue.
			(*it) -> _addToRenderQueue(camera, queue, onlyShadowCasters );
		}
		#ifdef cellist
		std::cerr << std::endl;
		#endif
	}
	
	
	//-----------------------------------------------------------------------
	BspNode* DarkSceneManager::walkTree(Camera* camera, RenderQueue *queue, bool onlyShadowCasters) {
		BspNode* cameraNode = NULL;
		
		if (mBspTree != NULL) // Locate the leaf node where the camera is located
			cameraNode = mBspTree->findLeaf(camera->getDerivedPosition());
		
		mActualFrame++; 
		
		mCellVisited = 0;
		mCellDrawn = 0;
		
		if (cameraNode != NULL) {
			DarkSceneNode *rootSceneNode = static_cast<DarkSceneNode *>(cameraNode->getSceneNode());
			
			// TODO: Check if DirectX renderer has the same screen rect size
			Rectangle screen;
			screen.top = 1;
			screen.right = 1;
			screen.bottom = -1;
			screen.left = -1;
			
			traversePortals(rootSceneNode, camera, queue, screen, onlyShadowCasters);
		}
		
		firstTime = false;
		
		return cameraNode;
	}
    
    
	//-----------------------------------------------------------------------
	void DarkSceneManager::showNodeBoxes(bool show) {
		mShowNodeAABs = show;
	}
	
	
    
	//-----------------------------------------------------------------------
	// TODO: Add methods for the ViewPoint setting if wanted
	ViewPoint DarkSceneManager::getSuggestedViewpoint(bool random) {
		ViewPoint pos;
		pos.position = Vector3(0, 0, 0);
		// pos.position = Vector3(15.79,-0.83,6.58);
		
		return pos;
	    
	    /*
		if (mLevel.isNull() || mLevel->mPlayerStarts.size() == 0) {
			// No level, use default
			return SceneManager::getSuggestedViewpoint(random);
		} else {
		}*/
	}
	
	//-----------------------------------------------------------------------
	SceneNode * DarkSceneManager::createSceneNode( void ) {
		DarkSceneNode * sn = new DarkSceneNode( this );
		mSceneNodes[ sn->getName() ] = sn;
		return sn;
	}
	
	//-----------------------------------------------------------------------
	SceneNode * DarkSceneManager::createSceneNode( const String &name ) {
		DarkSceneNode * sn = new DarkSceneNode( this, name );
		mSceneNodes[ sn->getName() ] = sn;
		return sn;
	}
	
	//-----------------------------------------------------------------------
	void DarkSceneManager::_notifyObjectMoved(const MovableObject* mov, const Vector3& pos) {
		if (mBspTree != NULL) {
			mBspTree->_notifyObjectMoved(mov, pos);
		}
	}
	
	//-----------------------------------------------------------------------
	void DarkSceneManager::_notifyObjectDetached(const MovableObject* mov) {
		if (mBspTree != NULL) {
			mBspTree->_notifyObjectDetached(mov);
		}
	}
	
	//-----------------------------------------------------------------------
	void DarkSceneManager::clearScene(void) {
		SceneManager::clearScene();
		
		if (mBspTree!=NULL) {
			delete mBspTree;
			mBspTree = NULL;
		}
		// TODO: freeMemory();
	}
	
	//-----------------------------------------------------------------------
	void DarkSceneManager::setBspTree(BspNode *rootNode) {
		if (mBspTree == NULL) 
				mBspTree = new BspTree();
		
		
		mBspTree->setBspTree(rootNode);
	}
	
	//-----------------------------------------------------------------------
	/*
	AxisAlignedBoxSceneQuery* DarkSceneManager::createAABBQuery(const AxisAlignedBox& box, unsigned long mask) {
		// TODO
		return NULL;
	}
	
	//-----------------------------------------------------------------------
	SphereSceneQuery* DarkSceneManager::createSphereQuery(const Sphere& sphere, unsigned long mask) {
		// TODO
		return NULL;
	}
	*/
	
	//-----------------------------------------------------------------------
	RaySceneQuery* DarkSceneManager::createRayQuery(const Ray& ray, unsigned long mask) {
		BspRaySceneQuery* q = new BspRaySceneQuery(this);
		q->setRay(ray);
		q->setQueryMask(mask);
		return q;
	}
	
	//-----------------------------------------------------------------------
	IntersectionSceneQuery* DarkSceneManager::createIntersectionQuery(unsigned long mask) {
		BspIntersectionSceneQuery* q = new BspIntersectionSceneQuery(this);
		q->setQueryMask(mask);
		return q;
	}
	
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	BspIntersectionSceneQuery::BspIntersectionSceneQuery(SceneManager* creator) : DefaultIntersectionSceneQuery(creator) {
		// Add bounds fragment type
		mSupportedWorldFragments.insert(SceneQuery::WFT_PLANE_BOUNDED_REGION);
        }
	
	void BspIntersectionSceneQuery::execute(IntersectionSceneQueryListener* listener) {
		/*
		Go through each leaf node in BspLevel and check movables against each other and world
		Issue: some movable-movable intersections could be reported twice if 2 movables
		overlap 2 leaves?
		*/
		
		/* TODO: Seriously this needs to be fixed WELL.
		const BspLevelPtr& lvl = ((DarkSceneManager*)mParentSceneMgr)->getLevel();
		
		if (lvl.isNull()) return;

		BspNode* node = lvl->getNodeStart();
		int numNodes = lvl->getNumNodes();
		
		while (numNodes--) {
			// skip non-leaf node
			if (!node->isLeaf()) { // TODO: this would be better done as iteration through cells
				++node;
				continue;
			}
			
			const BspNode::IntersectingObjectSet& objects = node->getObjects();
			int numObjects = (int)objects.size();
	
			BspNode::IntersectingObjectSet::const_iterator a, b, theEnd;
			
			theEnd = objects.end();
			a = objects.begin();
			
			for (int oi = 0; oi < numObjects; ++oi, ++a) {
				const MovableObject* aObj = *a;
				// Skip this object if collision not enabled
				if (!(aObj->getQueryFlags() & mQueryMask) || !aObj->isInScene())
					continue;
	
				if (oi < (numObjects-1)) {
					// Check object against others in this node
					b = a;
					for (++b; b != theEnd; ++b) {
						const MovableObject* bObj = *b;
						// Apply mask to b (both must pass)
						if ((bObj->getQueryFlags() & mQueryMask) && bObj->isInScene()) {
							const AxisAlignedBox& box1 = aObj->getWorldBoundingBox();
							const AxisAlignedBox& box2 = bObj->getWorldBoundingBox();
	
							if (box1.intersects(box2)) {
								if (!listener->queryResult(const_cast<MovableObject*>(aObj), 
								     const_cast<MovableObject*>(bObj)))
										return; 
							}
						}
					}
				}
			}
			
			++node;
		}
		*/
	}
    
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	BspRaySceneQuery::BspRaySceneQuery(SceneManager* creator) : DefaultRaySceneQuery(creator) {
		// Add supported fragment types
		mSupportedWorldFragments.insert(SceneQuery::WFT_SINGLE_INTERSECTION);
		mSupportedWorldFragments.insert(SceneQuery::WFT_PLANE_BOUNDED_REGION);
	}
	
	//-----------------------------------------------------------------------
	void BspRaySceneQuery::execute(RaySceneQueryListener* listener) {
		/* TODO: !!!!
		clearTemporaries();
		BspLevelPtr lvl = static_cast<DarkSceneManager*>(mParentSceneMgr)->getLevel();
		
		if (!lvl.isNull()) {
			processNode(
				lvl->getRootNode(), 
				mRay, listener);
		}
		*/
	}
	
	//-----------------------------------------------------------------------
	BspRaySceneQuery::~BspRaySceneQuery() {
		clearTemporaries();
	}
	
	//-----------------------------------------------------------------------
	void BspRaySceneQuery::clearTemporaries(void) {
		mObjsThisQuery.clear();
		std::vector<WorldFragment*>::iterator i;
		for (i = mSingleIntersections.begin(); i != mSingleIntersections.end(); ++i) {
			delete *i;
		}
		
		mSingleIntersections.clear();
	}
	
	//-----------------------------------------------------------------------
	bool BspRaySceneQuery::processNode(const BspNode* node, const Ray& tracingRay, 
			RaySceneQueryListener* listener, Real maxDistance, Real traceDistance) {
		
		if (node->isLeaf()) {
			return processLeaf(node, tracingRay, listener, maxDistance, traceDistance);
		}

		bool res = true;
		
		std::pair<bool, Real> result = tracingRay.intersects(node->getSplitPlane());
		
		if (result.first && result.second < maxDistance) {
			// Crosses the split plane, need to perform 2 queries
			// Calculate split point ray
			Vector3 splitPoint = tracingRay.getOrigin() 
				+ tracingRay.getDirection() * result.second;
			
			Ray splitRay(splitPoint, tracingRay.getDirection());
		
			if (node->getSide(tracingRay.getOrigin()) == Plane::NEGATIVE_SIDE) {
				// Intersects from -ve side, so do back then front
				res = processNode(
					node->getBack(), tracingRay, listener, result.second, traceDistance);
				
				if (!res) return res;
                
				res = processNode(
					node->getFront(), splitRay, listener, 
					maxDistance - result.second, 
					traceDistance + result.second);
			} else {
				// Intersects from +ve side, so do front then back
				res = processNode(node->getFront(), tracingRay, listener, 
					result.second, traceDistance);
				
				if (!res) return res;
				
				res = processNode(node->getBack(), splitRay, listener,
					maxDistance - result.second, 
					traceDistance + result.second);
			}
		} else {
			// Does not cross the splitting plane, just cascade down one side
			res = processNode(node->getNextNode(tracingRay.getOrigin()),
				tracingRay, listener, maxDistance, traceDistance);
		}

		return res;
	}
	
	//-----------------------------------------------------------------------
	bool BspRaySceneQuery::processLeaf(const BspNode* leaf, const Ray& tracingRay, 
		RaySceneQueryListener* listener, Real maxDistance, Real traceDistance) {
		
		/* TODO: !!!!
			
		const BspNode::IntersectingObjectSet& objects = leaf->getObjects();

		BspNode::IntersectingObjectSet::const_iterator i, iend;
		
		iend = objects.end();
		
		//Check ray against objects
        
		for(i = objects.begin(); i != iend; ++i) {
			// cast away constness, constness of node is nothing to do with objects
			MovableObject* obj = const_cast<MovableObject*>(*i);
			// Skip this object if not enabled
			if((obj->getQueryFlags() & mQueryMask) == 0)
				continue;

			// check we haven't reported this one already
			// (objects can be intersecting more than one node)
			if (mObjsThisQuery.find(obj) != mObjsThisQuery.end())
				continue;

			//Test object as bounding box
			std::pair<bool, Real> result = 
					tracingRay.intersects(obj->getWorldBoundingBox());
			
			// if the result came back positive and intersection point is inside
			// the node, fire the event handler
			
			if(result.first && result.second <= maxDistance) {
				if (!listener->queryResult(obj, result.second + traceDistance))
					return false;
			}
		}


		// Check ray against brushes
		if (mQueryMask & SceneManager::WORLD_GEOMETRY_TYPE_MASK) {
			const BspNode::NodeBrushList& brushList = leaf->getSolidBrushes();
			BspNode::NodeBrushList::const_iterator bi, biend;
			biend = brushList.end();
			bool intersectedBrush = false;
			
			for (bi = brushList.begin(); bi != biend; ++bi) {
				BspNode::Brush* brush = *bi;
                
			std::pair<bool, Real> result = Math::intersects(tracingRay, brush->planes, true);
			// if the result came back positive and intersection point is inside
			// the node, check if this brush is closer
			if(result.first && result.second <= maxDistance) {
				intersectedBrush = true;
				if(mWorldFragmentType == SceneQuery::WFT_SINGLE_INTERSECTION) {
					// We're interested in a single intersection
					// Have to create these 
					SceneQuery::WorldFragment* wf = new SceneQuery::WorldFragment();
					wf->fragmentType = SceneQuery::WFT_SINGLE_INTERSECTION;
					wf->singleIntersection = tracingRay.getPoint(result.second);
					// save this so we can clean up later
					mSingleIntersections.push_back(wf);
					if (!listener->queryResult(wf, result.second + traceDistance))
							return false;
				} else 
					if (mWorldFragmentType ==  SceneQuery::WFT_PLANE_BOUNDED_REGION) {
					// We want the whole bounded volume
					assert((*bi)->fragment.fragmentType == SceneQuery::WFT_PLANE_BOUNDED_REGION);
					if (!listener->queryResult(const_cast<WorldFragment*>(&(brush->fragment)), 
						result.second + traceDistance))
							return false; 

					}
				}
			}
		
			if (intersectedBrush) {
				return false; // stop here
			}
		}
		*/
		return true;
	}
	
	//-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
	const String DarkSceneManagerFactory::FACTORY_TYPE_NAME = "DarkSceneManager";
	//-----------------------------------------------------------------------
	void DarkSceneManagerFactory::initMetaData(void) const {
		mMetaData.typeName = FACTORY_TYPE_NAME;
		mMetaData.description = "Scene manager for BSP/Portal based maps";
		mMetaData.sceneTypeMask = ST_INTERIOR;
		mMetaData.worldGeometrySupported = true;
	}
	//-----------------------------------------------------------------------
	SceneManager* DarkSceneManagerFactory::createInstance(
		const String& instanceName) {
		return new DarkSceneManager(instanceName);
	}
	//-----------------------------------------------------------------------
	void DarkSceneManagerFactory::destroyInstance(SceneManager* instance) {
		delete instance;
	}
}

