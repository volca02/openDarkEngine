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
#include <OgreHardwareBufferManager.h>
#include <vector>
#include <fstream>

// #define debug
// #define cellist

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
	void DarkSceneManager::_renderVisibleObjects(void) {
		// Render static level geometry first
		renderStaticGeometry();

		// Call superclass to render the rest
		SceneManager::_renderVisibleObjects();		
	}
	
	//-----------------------------------------------------------------------
	void DarkSceneManager::prepareCell(BspNode *cell, Camera *camera, Matrix4& toScreen, PortalFrustum *frust) { 
		PortalListConstIterator outPortal_it = cell->mDstPortals.begin();
		
		for (;outPortal_it != cell->mDstPortals.end(); outPortal_it++) {
			Portal *out_portal = (*outPortal_it);

			out_portal->refreshScreenRect(camera, toScreen, frust);
			
			out_portal->mMentions = 0; 
		}
		
		cell->mInitialized = true;
		cell->mFrameNum = mActualFrame;
	}
	
	//-----------------------------------------------------------------------
	void DarkSceneManager::traversePortals(BspNode *cell, Camera* camera, RenderQueue *queue, PortalRect &viewrect, bool onlyShadowCasters) {
		#ifdef debug
		std::cerr << "traversal Starting" << std::endl;		
		#endif
		
		// Prepare the to screen transform matrix. 
		const Matrix4& viewM = camera->getViewMatrix();
		const Matrix4& projM = camera->getProjectionMatrix();
		
		Matrix4 toScreen = projM * viewM;
		
		PortalFrustum *cameraFrustum = new PortalFrustum(camera);
		
		mActiveCells.clear();
		
		// prepare the mother cell (in which the camera is)
		prepareCell(cell, camera, toScreen, cameraFrustum);
		mActiveCells.push_back(cell);
		mActualPosition = 0;
		cell->mListPosition = 0;
		cell->mFrameNum = mActualFrame; 
		
		PortalListConstIterator outPortal_it = cell->mDstPortals.begin();
		#ifdef debug
		std::cerr << " * MOTHER CELL ID : " << cell->mCellNum << std::endl;
		#endif
		
		for (;outPortal_it != cell->mDstPortals.end(); outPortal_it++) {
			Portal *outportal = (*outPortal_it);
			#ifdef debug	
			std::cerr << "  * MOTHER CELL: Testing portal " << outportal->mPortalID << " to cell " << outportal->mTarget->mCellNum << std::endl;
			#endif
			outportal->mMentions = 0; 
			
			if  (outportal->mPortalCull) {
				#ifdef debug
				std::cerr << "  * MOTHER CELL: backface cull for outcell " << outportal->mTarget->mCellNum << std::endl;
				#endif				
				
				continue;
			}

			PortalRect tgt;
				
			if (outportal->intersectByRect(viewrect, tgt)) {
				outportal->unionActualWithRect(tgt); // so we have non-zero actual rect for that portal
				
				// mark the visible portal, so it will be evaluated as a view source
				outportal->mMentions = 1; 
			} 
			#ifdef debug
			else {
				std::cerr << "  * MOTHER CELL: offscreen for " << outportal->mTarget << std::endl;
			}
			#endif
		}
		
		
		// While a cell is needed to be evaluated
		while (mActualPosition < mActiveCells.size()) {
			// get one cell
			#ifdef debug
			std::cerr << " * Actual position " << mActualPosition << std::endl;
			#endif
			
			BspNode *actual = mActiveCells.at(mActualPosition);

			#ifdef debug
			std::cerr << "    - with cell number " << actual->mCellNum << std::endl;
			#endif
			
			if (!actual->mInitialized)
				prepareCell(actual, camera, toScreen, cameraFrustum);
			
			int chcount = 0; // change count
			
			// now that we have the cell prepared, process the outgoing portals with input views
			PortalListConstIterator tgtPortal_it = actual->mDstPortals.begin();
		
			for (;tgtPortal_it != actual->mDstPortals.end(); tgtPortal_it++) {
				Portal *target_portal = *tgtPortal_it;	

				#ifdef debug
				std::cerr << "  * test output portal " << target_portal->mPortalID << " to cell " << target_portal->mTarget->mCellNum << std::endl;
				#endif				
				
				if  (target_portal->mPortalCull) { // backface cull
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
					BspNode *source_cell = source->mSource;
					
					if ((source_cell->mFrameNum != mActualFrame) || (!source_cell->mInitialized)) 
						continue;

					// Ok. The source cell was initialized
					
					// test for backface cull
					if  (source->mPortalCull) { // backface cull
						#ifdef debug
						std::cerr << "    * backface cull for source portal " << source->mPortalID << std::endl;
						#endif				

						continue;
					}
					
					
					if (source->mMentions > 0) { // reevaluation is needed
						#ifdef debug
						std::cerr << "     * Mention on portal, reconsidering" << std::endl; 
						#endif
						
						// clip the src_portals view to the target portal
						
						PortalRect tgt; // the rectangle which represents the actual computed src to target portal intersection
						
						if (target_portal->intersectByRect(source->mActualRect, tgt)) {
							#ifdef debug
							std::cerr << "   - non-zero portal " << source->mPortalID << " intersection" << std::endl;
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
					
					BspNode *target_cell = target_portal->mTarget;
					
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
						
						// If the cell was already processed, move it to the to
						// We do not want to requeue the mother cell (Hey, this would be a cycle!)
						if (target_cell->mListPosition < mActualPosition && target_cell->mListPosition != 0) { 
							#ifdef debug
							std::cerr << "   - requeueing cell " << target_cell->mCellNum << std::endl;
							#endif
							
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
		
		mCellVisited = 0;
		mCellDrawn = 0;
		
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
			
			#ifdef cellist
			std::cerr << "CELLIST: ";
			#endif

			for (;it != mActiveCells.rend(); it++) {
				#ifdef cellist
				std::cerr << (*it)->mCellNum << " ";
				#endif
				
				// TODO: HMM. Queue the movables.
				// Try to insert the movableObjects checking if it already is in the movablesForRendering or not
				queueBspNode((*it), camera, onlyShadowCasters);
				
			}
			#ifdef cellist
			std::cerr << std::endl;
			#endif
		}
		
		firstTime = false;
		
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
				
				// Check normal (manual culling). Something rotten here, not doing for now...
				/*ManualCullingMode cullMode = pMat->getTechnique(0)->getPass(0)->getManualCullingMode();
				
				if (cullMode != MANUAL_CULL_NONE) {
				    Real dist = faceGroup->plane.getDistance(camera->getDerivedPosition());
				    if ( (dist < 0 && cullMode == MANUAL_CULL_BACK) ||
					(dist > 0 && cullMode == MANUAL_CULL_FRONT) )
					continue; // skip
				} */
				

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
		for (size_t elem = 0; elem < numIdx; ++elem) {
			*pIndexes++ = *pSrc++;
		}
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
		// TODO: freeMemory();
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

		BspNode* node = tree->getLeafNodeStart();
		int numNodes = tree->getNumLeafNodes();
		
		while (numNodes--) {
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
		const BspNode* rootNode = static_cast<DarkSceneManager*>(mParentSceneMgr)->getRootBspNode();
		
		if (rootNode != NULL) {
			processNode(
				rootNode, 
				mRay, listener);
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
				BspNode *backNode = node->getBack();
				BspNode *frontNode = node->getFront();
				
				// A good place for assertion? If both front and back are null, houston has a problem (and us too)
				assert((backNode==NULL)&&(frontNode==NULL));
				
				if (backNode) { // If the back side is inside the world. if not, shall we ignore?
					// trace the abscissa from the start point to the split plane.
					res = processNode(
						backNode, tracingRay, listener, result.second, traceDistance);
					
					if (!res) return res;
				}
				
				// trace the rest of the ray.
				if (frontNode) {
					res = processNode(
						frontNode, splitRay, listener, 
						maxDistance - result.second, 
						traceDistance + result.second);
				}
				
			} else {
				BspNode *backNode = node->getBack();
				BspNode *frontNode = node->getFront();
				
				// A good place for assertion? If both front and back are null, houston has a problem (and us too)
				assert((backNode==NULL)&&(frontNode==NULL));
				
				if (frontNode) { // If the back side is inside the world. if not, shall we ignore?
					// trace the abscissa from the start point to the split plane.
					res = processNode(frontNode, tracingRay, listener, 
						result.second, traceDistance);
				
					
					if (!res) return res;
				}
				
				// trace the rest of the ray.
				if (backNode) {
					res = processNode(
						backNode, splitRay, listener, 
						maxDistance - result.second, 
						traceDistance + result.second);
				}
			}
		} else {
			// Does not cross the splitting plane, just cascade down one side
			BspNode *nextNode = node->getNextNode(tracingRay.getOrigin());
			
			// in the void space out of the tree, we have a problem too!
			assert(!nextNode);
			
			res = processNode(nextNode,
				tracingRay, listener, maxDistance, traceDistance);
		}

		return res;
	}
	
	//-----------------------------------------------------------------------
	bool BspRaySceneQuery::processLeaf(const BspNode* leaf, const Ray& tracingRay, 
		RaySceneQueryListener* listener, Real maxDistance, Real traceDistance) {
		
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


		// Check ray against the cell planes
		if (mQueryMask & SceneManager::WORLD_GEOMETRY_TYPE_MASK) {
			// Get the plane list out of the leaf cell
			const BspNode::CellPlaneList& planes = leaf->getPlaneList();
			
			bool intersectedBrush = false;
			
			std::pair<bool, Real> result = Math::intersects(tracingRay, planes, true);
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
						assert(leaf->mPlaneFragment.fragmentType == SceneQuery::WFT_PLANE_BOUNDED_REGION);
						if (!listener->queryResult(const_cast<WorldFragment*>(&(leaf->mPlaneFragment)), 
						result.second + traceDistance))
							return false; 

					}
				}
			
		
			if (intersectedBrush) {
				return false; // stop here
			}
		}
		
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

