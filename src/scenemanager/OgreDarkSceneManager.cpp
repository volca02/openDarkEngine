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

#include "config.h"
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
#include <OgreHardwareBufferManager.h>
#include <vector>
#include <fstream>

// The ray query traversal log
// #define raydetails
// only the leaf node's details
// #define raydetailsl
// The details of intersection test
// #define raydetailsli

// A conditional logging of cell-portal traversal
#ifdef OPDE_DEBUG
	#define LOG_TRAVERSAL(...) if (mTraversalLog) fprintf(stderr, __VA_ARGS__);
#else
	#define LOG_TRAVERSAL(...) ;
#endif


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
	    
		mBspTree = NULL;
		
		mShowPortals = false;
		
		mTraversalLog = false;
		
		mBackfaced = 0;
		mCellsRendered = 0;
		mEvalPortals = 0;
	}

	//-----------------------------------------------------------------------
	DarkSceneManager::~DarkSceneManager() {
		freeMemory();
	}
    
	//-----------------------------------------------------------------------
	void DarkSceneManager::freeMemory(void) {
		// no need to delete index buffer, will be handled by shared pointer
		delete mRenderOp.indexData;
		mRenderOp.indexData = 0;
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
			"World geometry is not supported by the DarkSceneManager. Use the supplied methods to inject it.",
			"SceneManager::setWorldGeometry");
	}
    
	//-----------------------------------------------------------------------
	void DarkSceneManager::setStaticGeometry(Ogre::VertexData* vertexData, Ogre::HardwareIndexBufferSharedPtr indexes, 
		Ogre::StaticFaceGroup* faceGroups, unsigned int numIndexes, unsigned int numFaceGroups) {
		
		mVertexData = vertexData;
		mIndexes = indexes;
		mNumIndexes = numIndexes;
		mFaceGroups = faceGroups;
		mNumFaceGroups = numFaceGroups;

		// TODO: If already allocated, deallocate
		mRenderOp.vertexData = mVertexData;
		// index data is per-frame
		mRenderOp.indexData = new IndexData();
		mRenderOp.indexData->indexStart = 0;
		mRenderOp.indexData->indexCount = 0;
		// Create enough index space to render whole level
		mRenderOp.indexData->indexBuffer = HardwareBufferManager::getSingleton()
		.createIndexBuffer(
			HardwareIndexBuffer::IT_32BIT, // always 32-bit
			mNumIndexes, 
			HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE, false);
	
		mRenderOp.operationType = RenderOperation::OT_TRIANGLE_LIST;
		mRenderOp.useIndexes = true;
	}
	
	//-----------------------------------------------------------------------
	void DarkSceneManager::_findVisibleObjects(Camera *cam, VisibleObjectsBoundsInfo *visibleBounds, bool onlyShadowCasters) {
		// Erase the render queue
		RenderQueue* renderQueue = getRenderQueue();
		
		renderQueue->clear();
		
		// Clear unique list of movables for this frame
		mMovablesForRendering.clear();
	    
		// Walk the tree, tag static geometry, return camera's node (for info only)
		walkTree(cam, renderQueue, onlyShadowCasters);
	}
	
	//-----------------------------------------------------------------------
	void DarkSceneManager::renderStaticGeometry(void) {
		// Check we should be rendering
		if (!isRenderQueueToBeProcessed(mWorldGeometryRenderQueue))
			return;

		// Cache vertex/face data first
		std::vector<StaticFaceGroup*>::const_iterator faceGrpi;
		static RenderOperation patchOp;
        
		// no world transform required
		mDestRenderSystem->_setWorldMatrix(Matrix4::IDENTITY);
		// Set view / proj
		mDestRenderSystem->_setViewMatrix(mCameraInProgress->getViewMatrix(true));
		mDestRenderSystem->_setProjectionMatrix(mCameraInProgress->getProjectionMatrixRS());

		// For each material in turn, cache rendering data & render
		MaterialFaceGroupMap::const_iterator mati;

		for (mati = mMatFaceGroupMap.begin(); mati != mMatFaceGroupMap.end(); ++mati) {
			// Get Material
			Material* thisMaterial = mati->first;

			// Empty existing cache
			mRenderOp.indexData->indexCount = 0;
			// lock index buffer ready to receive data
			unsigned int* pIdx = static_cast<unsigned int*>(
				mRenderOp.indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));

			for (faceGrpi = mati->second.begin(); faceGrpi != mati->second.end(); ++faceGrpi) {
				// Cache each
				unsigned int numelems = cacheGeometry(pIdx, *faceGrpi);
				mRenderOp.indexData->indexCount += numelems;
				pIdx += numelems;
			}
			
			// Unlock the buffer
			mRenderOp.indexData->indexBuffer->unlock();

			// Skip if no faces to process (we're not doing flare types yet)
			if (mRenderOp.indexData->indexCount == 0)
				continue;

			Technique::PassIterator pit = thisMaterial->getTechnique(0)->getPassIterator();
		
			while (pit.hasMoreElements()) {
				_setPass(pit.getNext());

				// Hacky. To let the polygon mode be wireframe
				mDestRenderSystem->_setPolygonMode(mCameraInProgress->getPolygonMode());
				mDestRenderSystem->_render(mRenderOp);
			}
		} // for each material
	}
    
	//-----------------------------------------------------------------------
	void DarkSceneManager::renderPortals(void) {
		if (!isRenderQueueToBeProcessed(mWorldGeometryRenderQueue))
			return;
		
		// no world transform required
		mDestRenderSystem->_setWorldMatrix(Matrix4::IDENTITY);
		// Set view / proj
		mDestRenderSystem->_setViewMatrix(mCameraInProgress->getViewMatrix(true));
		mDestRenderSystem->_setProjectionMatrix(mCameraInProgress->getProjectionMatrixRS());
		
		// Iterate through the portal renderables and render
		PortalListConstIterator pit = mVisiblePortals.begin();
		
		mDestRenderSystem->_setPolygonMode(mCameraInProgress->getPolygonMode());
		
		for (;pit!= mVisiblePortals.end(); pit++) {
			// add the portal to the renderqueue as a renderable
			getRenderQueue()->addRenderable((*pit));
		}
		
		
	}
	
	//-----------------------------------------------------------------------
	void DarkSceneManager::_renderVisibleObjects(void) {
		// render the sky if appropriate. Do this here so the skyhack will work
		/* To do this, i have to skip the queueing because the static geometry is rendered directly, skipping the queue. I could in theory fire the rendering
		for this single group
		*/
		// TODO: Once the sky system is solved, do a direct rendering (using renderOps) of the sky mesh
				
		// Render static level geometry first
		renderStaticGeometry();

		if (mShowPortals)
			renderPortals();
		
		// Call superclass to render the rest
		SceneManager::_renderVisibleObjects();		
	}
	
	//-----------------------------------------------------------------------
	void DarkSceneManager::prepareCell(BspNode *cell, Camera *camera, Matrix4& toScreen, PortalFrustum *frust) { 
		PortalListConstIterator outPortal_it = cell->mDstPortals.begin();
		PortalListConstIterator end = cell->mDstPortals.end();
		
		for (;outPortal_it != end; outPortal_it++) {
			Portal *out_portal = (*outPortal_it);

			mEvalPortals++;
			
			out_portal->refreshScreenRect(camera, toScreen, frust);
			
			if  (out_portal->mPortalCull) {
				mBackfaced++;
			}
		}
		
		cell->mInitialized = true;
		cell->mFrameNum = mActualFrame;
		mCellsRendered++;
	}
	
	//-----------------------------------------------------------------------
	void DarkSceneManager::traversePortals(BspNode *cell, Camera* camera, RenderQueue *queue, PortalRect &viewrect, bool onlyShadowCasters) {
		LOG_TRAVERSAL("CELL_TRAVERSAL: traversal Starting");
		// Prepare the to screen transform matrix. 
		const Matrix4& viewM = camera->getViewMatrix();
		const Matrix4& projM = camera->getProjectionMatrix();
		
		// clear statistics
		mBackfaced = 0;
		mCellsRendered = 0;
		mEvalPortals = 0;
		
		
		Matrix4 toScreen = projM * viewM;
		
		PortalFrustum *cameraFrustum = new PortalFrustum(camera);
		
		mActiveCells.clear();
		
		// prepare the mother cell (in which the camera is)
		prepareCell(cell, camera, toScreen, cameraFrustum);
		mActiveCells.push_back(cell);
		mActualPosition = 0;
		cell->mListPosition = 0;
		cell->mFrameNum = mActualFrame; 
		
		// Whole screen rect for the input view
		// TODO: Init these with the current view size
		cell->mViewRect.left = 0;
		cell->mViewRect.bottom = 0;
		cell->mViewRect.right = 1024;
		cell->mViewRect.top = 768;
		
		PortalListConstIterator outPortal_it = cell->mDstPortals.begin();
		
		LOG_TRAVERSAL("CELL_TRAVERSAL: * MOTHER CELL ID : %d", cell->mCellNum);

		
		// While a cell is needed to be evaluated
		while (mActualPosition < mActiveCells.size()) {
			// get one cell
			LOG_TRAVERSAL("CELL_TRAVERSAL: * Actual position ", mActualPosition);
			
			BspNode *actual = mActiveCells.at(mActualPosition);

			LOG_TRAVERSAL("CELL_TRAVERSAL:  - with cell number ", actual->mCellNum);
			
			if (!actual->mInitialized)
				prepareCell(actual, camera, toScreen, cameraFrustum);
			
			int chcount = 0; // change count
			
			// now that we have the cell prepared, process the outgoing portals with input views
			PortalListConstIterator tgtPortal_it = actual->mDstPortals.begin();
		
			// for all target portals.
			for (;tgtPortal_it != actual->mDstPortals.end(); tgtPortal_it++) {
				Portal *target_portal = *tgtPortal_it;	

				LOG_TRAVERSAL("CELL_TRAVERSAL: * test output portal %d to cell %d", 
					      target_portal->mPortalID, target_portal->mTarget->mCellNum);
				
				if  (target_portal->mPortalCull) { // backface cull
					LOG_TRAVERSAL("CELL_TRAVERSAL:    * backface cull for %d to cell %d", 
							target_portal->mPortalID, target_portal->mTarget->mCellNum);

					continue;
				}

				bool changed = false;
				
				PortalRect tgt;
				
				BspNode *target_cell = target_portal->mTarget;
				
				
				if (target_portal->intersectByRect(actual->mViewRect, tgt)) {
					LOG_TRAVERSAL("CELL_TRAVERSAL:    - non-zero portal view intersection");
							
					if (target_portal->unionActualWithRect(tgt)) { // if the view change modifies the portals vision-through
						LOG_TRAVERSAL("CELL_TRAVERSAL: - outgoing portal view changed.");
							
						// cell's cull box should be empty...
						if (target_cell->mFrameNum != mActualFrame) {
							target_cell->mViewRect = tgt;
							changed = true;
						} else {
							changed = target_portal->mTarget->updateViewRect(tgt);
						}
						chcount++;
					}
				}
				
				// end of portal reevaluation
				
				if (changed) { // the view to the target cell has changed
					LOG_TRAVERSAL("CELL_TRAVERSAL:     * a change in portal view occured - %d", target_portal->mPortalID);
					
					// will only add the ones that are not inside already
					mVisiblePortals.insert(target_portal);
					
					// if the cell was not yet added
					if (target_cell->mFrameNum != mActualFrame) {
						LOG_TRAVERSAL("CELL_TRAVERSAL:    - adding cell %d", target_cell->mCellNum);
						
						target_cell->mInitialized = false; // invalidate the prepareness state
						
						mActiveCells.push_back(target_cell);
						target_cell->mListPosition = mActiveCells.size()-1;
						
						target_cell->mFrameNum = mActualFrame;
					} else {
						// we'll have to move the cell to the end if it has a position less than the actual cell
						LOG_TRAVERSAL("CELL_TRAVERSAL:    - requeue? %d < %d",
								target_cell->mListPosition, mActualPosition);
						
						// If the cell was already processed, move it to the to
						// We do not want to requeue the mother cell (Hey, this would be a cycle!)
						if (target_cell->mListPosition < mActualPosition && target_cell->mListPosition != 0) { 
							LOG_TRAVERSAL("CELL_TRAVERSAL:    - requeueing cell %d", target_cell->mCellNum);
							
							// change the ListPosition for all succesive cells
							std::vector< BspNode *>::iterator cell_it = mActiveCells.begin() + target_cell->mListPosition + 1;
							
							for (;cell_it != mActiveCells.end(); cell_it++)
								(*cell_it)->mListPosition--;
							
							// requeue the cell
							mActiveCells.erase(mActiveCells.begin() + target_cell->mListPosition);
							
							mActiveCells.push_back(target_cell);
							target_cell->mListPosition = mActiveCells.size() - 1; // we put it to the end	
							
							mActualPosition--;
						}
					} // if cell not actual
				} // if changed
			} // for portals
			
			if (chcount == 0)
				LOG_TRAVERSAL("CELL_TRAVERSAL:   ! No change occured!");
			
			// now unmention all the incoming portals
			// I have to do it here because directly unmentioning would destroy the portal vis testing...
			PortalListConstIterator src_portal_it = actual->mSrcPortals.begin();
			
			mActualPosition++;
		} // while mActualPosition

		LOG_TRAVERSAL("CELL_TRAVERSAL: Finished the traversal...");
		
		// Get rid of our frustum
		delete cameraFrustum;
	}
	
	
	//-----------------------------------------------------------------------
	BspNode* DarkSceneManager::walkTree(Camera* camera, RenderQueue *queue, bool onlyShadowCasters) {
		BspNode* cameraNode = NULL;
		
		if (mBspTree != NULL) // Locate the leaf node where the camera is located
			cameraNode = mBspTree->findLeaf(camera->getDerivedPosition());
		
		mActualFrame++; 
		
		// Clear the rendering helping buffers
		mMatFaceGroupMap.clear();
		mFaceGroupSet.clear();
		
		// visible portal list clear
		mVisiblePortals.clear();
		
		if (cameraNode != NULL) {
			// I'm walking on BspNodes now. It seems to be cleaner approach
			//DarkSceneNode *rootSceneNode = static_cast<DarkSceneNode *>(cameraNode->getSceneNode());
			
			// TODO: Check if DirectX renderer has the same screen rect size
			PortalRect screen; 
			screen.right = 1024;
			screen.top = 768;
			screen.bottom = 0;
			screen.left = 0;
			
			traversePortals(cameraNode, camera, queue, screen, onlyShadowCasters);
			
			// TODO: No need to order back-to-front now, the rendering in underlying sceneManager will handle this
			// Maybe this is a key for the skyhack to work. Leave it as it is
			std::vector<BspNode *>::reverse_iterator it = mActiveCells.rbegin();
			
			for (;it != mActiveCells.rend(); it++)
				queueBspNode((*it), camera, onlyShadowCasters);
		}
		
		mTraversalLog = false;
		
		return cameraNode;
	}
    
    
	//-----------------------------------------------------------------------
	void DarkSceneManager::queueBspNode(BspNode* node, Camera* camera, bool onlyShadowCasters) {
		MaterialPtr pMat;
		
		// Skip world geometry if we're only supposed to process shadow casters
		// World is pre-lit
		if (!onlyShadowCasters) {
			// Parse the leaf node's faces, add face groups to material map
			int numGroups = node->mNumFaceGroups;
			int idx = node->mFaceGroupStart;

			for (;numGroups--;idx++) {
				// Check not already included
				if (mFaceGroupSet.find(idx) != mFaceGroupSet.end())
				    	continue;

				StaticFaceGroup* faceGroup = mFaceGroups + idx;
				
				// Get Material pointer by handle
				pMat = MaterialManager::getSingleton().getByHandle(faceGroup->materialHandle);
				assert (!pMat.isNull());
				
				// Check normal (manual culling).
				ManualCullingMode cullMode = pMat->getTechnique(0)->getPass(0)->getManualCullingMode();
				
				if (cullMode != MANUAL_CULL_NONE) {
				    Real dist = faceGroup->plane.getDistance(camera->getDerivedPosition());
				    if ( (dist < 0 && cullMode == MANUAL_CULL_BACK) ||
					(dist > 0 && cullMode == MANUAL_CULL_FRONT) )
					continue; // skip
				}
				

				mFaceGroupSet.insert(idx);
				// Try to insert, will find existing if already there
				std::pair<MaterialFaceGroupMap::iterator, bool> matgrpi;
				matgrpi = mMatFaceGroupMap.insert(
				    MaterialFaceGroupMap::value_type(pMat.getPointer(), std::vector<StaticFaceGroup*>())
				    );
				
				// Whatever happened, matgrpi.first is map iterator
				// Need to get second part of that to get vector
				matgrpi.first->second.push_back(faceGroup);
			}
		}
		
		const BspNode::IntersectingObjectSet& objects = node->getObjects();
		BspNode::IntersectingObjectSet::const_iterator oi, oiend;
		
		oiend = objects.end();
		for (oi = objects.begin(); oi != oiend; ++oi)
		{
		    if (mMovablesForRendering.find(*oi) == mMovablesForRendering.end())
		    {
			// It hasn't been rendered yet
			MovableObject *mov = const_cast<MovableObject*>(*oi); // hacky
			if (mov->isVisible() && 
			    (!onlyShadowCasters || mov->getCastShadows()) && 
						camera->isVisible(mov->getWorldBoundingBox()))
			{
				mov->_notifyCurrentCamera(camera);
				mov->_updateRenderQueue( getRenderQueue() );
			    
				mMovablesForRendering.insert(*oi);
			}

		    }
		}
	}
	
	//-----------------------------------------------------------------------
	unsigned int DarkSceneManager::cacheGeometry(unsigned int* pIndexes, 
		const StaticFaceGroup* faceGroup) {

		size_t idxStart, numIdx, vertexStart;

		if (faceGroup->fType == FGT_FACE_LIST) {
			idxStart = faceGroup->elementStart;
			numIdx = faceGroup->numElements;
			vertexStart = faceGroup->vertexStart;
		} else	{
			// Unsupported face type
			return 0;
		}


		// Copy index data
		unsigned int* pSrc = static_cast<unsigned int*>(
		    mIndexes->lock(
			idxStart * sizeof(unsigned int),
			numIdx * sizeof(unsigned int), 
			HardwareBuffer::HBL_READ_ONLY));
		
		// I do not offset indexes here as I can do it before the rendering gets place
		/*for (size_t elem = 0; elem < numIdx; ++elem) {
			*pIndexes++ = *pSrc++;
		}*/
		memcpy(pIndexes, pSrc, numIdx * sizeof(unsigned int));
		
		mIndexes->unlock();

		// return number of elements
		return static_cast<unsigned int>(numIdx);
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
		
		freeMemory();
	}
	
	//-----------------------------------------------------------------------
	void DarkSceneManager::setBspTree(BspNode *rootNode, BspNode *leafNodes, BspNode* nonLeafNodes, 
		size_t leafNodeCount, size_t nonLeafNodeCount) {
		if (mBspTree == NULL) 
				mBspTree = new BspTree();
		
		
		mBspTree->setBspTree(rootNode, leafNodes, nonLeafNodes, leafNodeCount, nonLeafNodeCount);
		
		mLeafNodes = leafNodes;
		mNumLeafNodes = leafNodeCount;
		
		mNonLeafNodes = nonLeafNodes;
		mNumNonLeafNodes = nonLeafNodeCount;
	}
	
	//-----------------------------------------------------------------------
	const BspNode* DarkSceneManager::getRootBspNode() {
		return mBspTree->getRootNode();
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
	bool DarkSceneManager::setOption( const String & key, const void * val ) {
    		if ( key == "ShowPortals" ) {
        		mShowPortals = * static_cast < const bool * > ( val );
        		return true;
	    	}

		return SceneManager::setOption( key, val );
	}
	
	//-----------------------------------------------------------------------
	bool DarkSceneManager::getOption( const String & key, void *val ) {
		if ( key == "ShowPortals" ) {
			bool * bptr = ((bool *)(val));
        		*bptr = mShowPortals;
        		return true;
	    	} else if ( key == "BackfaceCulls" ) {
			unsigned int * uptr = ((unsigned int *)(val));
        		*uptr = mBackfaced;
        		return true;
	    	} else if ( key == "CellsRendered" ) {
			unsigned int * uptr = ((unsigned int *)(val));
        		*uptr = mCellsRendered;
        		return true;
	    	} else if ( key == "EvaluatedPortals" ) {
			unsigned int * uptr = ((unsigned int *)(val));
        		*uptr = mEvalPortals;
        		return true;
	    	}

		return SceneManager::setOption( key, val );
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
		
		BspTree* tree = ((DarkSceneManager*)mParentSceneMgr)->getBspTree();
		
		if (tree == NULL) return;

		BspNode* leaf = tree->getLeafNodeStart();
		int numNodes = tree->getNumLeafNodes();
		
		while (numNodes--) {
			const BspNode::IntersectingObjectSet& objects = leaf->getObjects();
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
			
			
				// Check object against cell's geometry
				if (mQueryTypeMask & SceneManager::WORLD_GEOMETRY_TYPE_MASK) {
					// Get the plane list out of the leaf cell
					const BspNode::CellPlaneList& planes = leaf->getPlaneList();
					
					BspNode::CellPlaneList::const_iterator pi, piend;
					piend = planes.end();
					Real radius = aObj->getBoundingRadius();
					const Vector3& pos = aObj->getParentNode()->_getDerivedPosition();

					bool planeIntersect = false; // Assume no intersection for now
					int pidx = 0;
					
					for (pi = planes.begin(); pi != piend; ++pi, ++pidx) {
						Real dist = pi->getDistance(pos);
						
						if (dist < radius) { // a real intersection occured. this means we can stop testing now...
							// But first, test if the sphere is not totally covered by a portal
							BspNode::PlanePortalMap::const_iterator ports = leaf->mPortalMap.find(pidx);
			
							if (ports != leaf->mPortalMap.end()) {
								PortalListConstIterator ptend = ports->second.end();
								PortalListConstIterator it = ports->second.begin();
								
								bool portalCovered = false;
								for (; it != ptend; it++) {
									if ((*it)->enclosesSphere(pos, radius, dist)) {
										portalCovered = true;
										break;
									}
								}
								
								if (portalCovered)
									continue;
							}
							
							planeIntersect = true;
							break;
						}
					}
					
					if (planeIntersect) {
						// report the cell as it's WorldFragment
						assert(leaf->mCellFragment.fragmentType == SceneQuery::WFT_CUSTOM_GEOMETRY);
						if (!listener->queryResult(const_cast<MovableObject*>(aObj), 
							const_cast<WorldFragment*>(&(leaf->mCellFragment))))
								return; 
					}
					
				}
			} // for each object in the leaf
			
			++leaf;
		}
		
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
		clearTemporaries();
		
		const BspTree* bspTree = static_cast<DarkSceneManager*>(mParentSceneMgr)->getBspTree();
		
		// Find the leaf node the ray origin is in
		const BspNode* baseNode = bspTree->findLeaf(mRay.getOrigin());
		
		if (baseNode != NULL) {
			#ifdef raydetails
			std::cerr << "! STARTING RAY SCENE QUERY !" << std::endl;
			#endif
			
			traverseLeafNodes(baseNode, mRay, listener);
		}
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
	void BspRaySceneQuery::traverseLeafNodes(const BspNode* leaf, Ray tracingRay, 
			RaySceneQueryListener* listener) {
		
		Real traceDistance = 0;
		bool nextCell = true;
		Ray actualRay = tracingRay;	
		
		while (nextCell) {
			nextCell = false;
			
			if (!leaf->isLeaf()) 
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Node given is not a leaf node. Cannot progress", "BspRaySceneQuery::traverseLeafNode");
	
			#ifdef raydetailsl
			std::cerr << " * RSQ PL: To process the leaf " << leaf->getCellNum() << " with : " << actualRay.getOrigin() << " dir " << actualRay.getDirection()
				<< std::endl;
			#endif
				
			const BspNode::IntersectingObjectSet& objects = leaf->getObjects();
	
			BspNode::IntersectingObjectSet::const_iterator i, iend;
			
			iend = objects.end();
			
			// Get the plane list out of the leaf cell
			const BspNode::CellPlaneList& planes = leaf->getPlaneList();
			
			bool intersectedBrush = false;
			
			int pidx = -1;
			
			Plane cl_plane;
			
			std::pair<bool, Real> result = rayIntersects(actualRay, planes, false, pidx, cl_plane);
			
			#ifdef raydetailsl
			if (result.first) {
				std::cerr << "\tRSQ PL: Intersection! D = "  << result.second << " plane index " << pidx << std::endl;
			} else {
				std::cerr << "\tRSQ PL: No intersection. with bulshit dist : "<< result.second << std::endl;
			}
			#endif
			
			// Because I'm inside, I definetaly should hit a plane... If I'm not inside, the BSP should have informed me
			if (!result.first) {
				std::cerr << "No plane hit while traversing through cell no. " << leaf->getCellNum() <<
					" ray " << actualRay.getOrigin() << " -> " << actualRay.getDirection() << std::endl;
				return;
			}
				//OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Ray didn't hit a plane!", "BspRaySceneQuery::traverseLeafNode");
			
			// I'll only report a success on the object hit when it is nearer the wall (is this right thing to do?)
			Real collDistance = result.second;
			
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
						actualRay.intersects(obj->getWorldBoundingBox());
				
				// if the result came back positive and intersection point is inside
				// the node, fire the event handler
				
				if(result.first && result.second <= collDistance) {
					if (!listener->queryResult(obj, collDistance + traceDistance))
						return; // We hit an object, we can stop
				}
			}
			
			// Now, we'll test for portal intersection
			BspNode::PlanePortalMap::const_iterator ports = leaf->mPortalMap.find(pidx);
			
					
			if (ports != leaf->mPortalMap.end()) {
				#ifdef raydetailsl
				std::cerr << "\tRSQ PL: Will test PORTALS " << std::endl;
				#endif
				
				PortalListConstIterator ptend = ports->second.end();
				PortalListConstIterator it = ports->second.begin();
				
				bool portalHit = false;
				for (; it != ptend; it++) {
					if ((*it)->isHitBy(actualRay)) {
						#ifdef raydetailsl
						std::cerr << "\tRSQ PL: Portal hit..." << std::endl;
						#endif
						
						// Move our origin into the target cell
						// We'll move the origin of the Ray into the collision point, plus some extra to fix some irregularities of the FPU math
						
						Vector3 portalPoint = actualRay.getOrigin() 
							+ actualRay.getDirection() * (collDistance + PLANE_DISTANCE_CORRECTION);
				
						Ray newRay(portalPoint, actualRay.getDirection());
						
						// now traverse to the next cell 
						leaf = (*it)->getTarget();
						actualRay = newRay;
						traceDistance += collDistance + PLANE_DISTANCE_CORRECTION;
						nextCell = true;
						
						portalHit = true;
						
						break;
					}	
				}
				
				if (portalHit)
					continue;
			}
			#ifdef raydetailsl
			else {
				std::cerr << "\tRSQ PL: No portals for the colliding plane..." << std::endl;
			}
			#endif
			
			// In this point, I know I hit a wall (portals were already tested)
			if(result.first) {
				#ifdef raydetailsl
				std::cerr << "\tRSQ PL: Found intersection " << actualRay.getPoint(result.second) << " dist " << collDistance + traceDistance << "(is " << collDistance << " " << traceDistance << ")" << std::endl;
				#endif
				
				Vector3 onPlane = actualRay.getPoint(result.second);
				
				if(mWorldFragmentType == SceneQuery::WFT_SINGLE_INTERSECTION) {
					// We're interested in a single intersection
					// Have to create these 
					SceneQuery::WorldFragment* wf = new SceneQuery::WorldFragment();
					wf->fragmentType = SceneQuery::WFT_SINGLE_INTERSECTION;
					wf->singleIntersection = onPlane;
					
					// save this so we can clean up later
					mSingleIntersections.push_back(wf);
					if (!listener->queryResult(wf, collDistance + traceDistance))
							return;
					
				} else if (mWorldFragmentType ==  SceneQuery::WFT_CUSTOM_GEOMETRY) {
					// We want the whole bounded volume
					assert(leaf->mCellFragment.fragmentType == SceneQuery::WFT_CUSTOM_GEOMETRY);
					if (!listener->queryResult(const_cast<WorldFragment*>(&(leaf->mCellFragment)), 
						collDistance + traceDistance))
						return; 
				}
			}
		} // While 
	}
	
	//-----------------------------------------------------------------------
	std::pair<bool, Real> BspRaySceneQuery::rayIntersects(const Ray& ray, 
		const std::list<Plane>& planes, bool normalIsOutside, int& planeindex, Plane& cPlane) {
		
		// I have to implement my own intersects routine, because I actually want to be inside the volume
		// If any of the planes return outside, I'm not inside the volume, and therefore no intersection could occur
		/* The second reason for custom implementation is the fact that the character of the data I use force me to
		evaluate whether the intersection happened inside the portal or not. It would not be a great thing to iterate through all
		the portals of the cell if I only need to evaluate those which lie on the plane that I got the minimal distance from 
		*/	
			
		std::list<Plane>::const_iterator planeit, planeitend;
		planeitend = planes.end();
		bool anyOutside = false;
		std::pair<bool, Real> ret;
		ret.first = false;
		ret.second = Math::POS_INFINITY;
		
		// TODO: DELETE THIS
		Real f = Math::POS_INFINITY;
		
		#ifdef raydetailsli
		std::cerr << "   RSQ RI: Will search for an intersection " << std::endl;
		#endif
		
		Plane::Side inside  = normalIsOutside ?  Plane::NEGATIVE_SIDE : Plane::POSITIVE_SIDE;
		Plane::Side outside = normalIsOutside ?  Plane::POSITIVE_SIDE : Plane::NEGATIVE_SIDE;
		
		int index = 0;
		
		for (planeit = planes.begin(); planeit != planeitend; ++planeit, ++index) {
			const Plane& plane = *planeit;
			
			// is origin inside?
			if (plane.getSide(ray.getOrigin()) != outside) { 
				// Test single plane
				std::pair<bool, Real> planeRes = ray.intersects(plane);
				
				// only consider this one if no backface cull happens
				Real toRay = plane.normal.dotProduct(ray.getDirection() );
				
				if ((toRay < 0) && planeRes.first) {
					// Ok, we intersected
					ret.first = true;
					
					// Use the least distant result since we have a convex volume
					if (planeRes.second < ret.second) {
						ret.second = planeRes.second;
						planeindex = index;
						cPlane = plane;
					}
				}
			} else {
				anyOutside = true;
				break;
			}
		}

		if (anyOutside) {
			#ifdef raydetailsli
			std::cerr << "   RSQ RI: Outside the volume from plane index " << index << ". Will not report min. d. " <<  ret.second << " plane dist " << f << std::endl;
			#endif
			// Intersecting at 0 distance since inside the volume!
			ret.first = false;
			ret.second = Math::POS_INFINITY;
		}

		return ret;
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

