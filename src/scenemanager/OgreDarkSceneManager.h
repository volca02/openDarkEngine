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

Rewritten to be used by the openDarkEngine project by Filip Volejnik <f.volejnik@centrum.cz>
*/
#ifndef __DarkSceneManager_H__
#define __DarkSceneManager_H__


#include "OgreBspPrerequisites.h"
#include "OgreSceneManager.h"
#include "OgreStaticFaceGroup.h"
#include "OgreRenderOperation.h"
#include <set>
#include <vector>
#include "OgrePortalFrustum.h"
#include "OgreDarkSceneNode.h"
#include "OgreBspTree.h"

namespace Ogre {

	/**
	*/
	class DarkSceneManager : public SceneManager {
		protected:
			// some statistics
			unsigned int	mCellDrawn;
			unsigned int	mCellVisited;
			unsigned int 	mActualFrame;
		    
			// State variables for rendering WIP
			RenderOperation mRenderOp;
		
			// Debugging features
			bool mShowNodeAABs;
			RenderOperation mAABGeometry;
		
			BspTree *mBspTree;
		
			/** Walks the BSP tree looking for the node which the camera
			    is in, and tags any geometry which is in a visible leaf for
			    later processing using Portal Visibility.
			    @param camera Pointer to the viewpoint.
			    @returns The BSP node the camera was found in, for info.
			*/
			BspNode* walkTree(Camera* camera, RenderQueue *queue, bool onlyShadowCasters);
			
			
			/**
			* Prepares the cell (visited first time the frame) to be processed by the traversal routine
			*/
			void prepareCell(DarkSceneNode *cell, Camera *camera, PortalFrustum *frust);
			
			/**
			* Traverses the scene using portals. Fills the mCellList vector with visible SceneNodes.
			*/
			void traversePortals(DarkSceneNode *cell, Camera* camera, RenderQueue *queue, Rectangle &viewrect, bool onlyShadowCasters);
			
			/** Tags geometry in the leaf specified for later rendering. */
			void processVisibleLeaf(BspNode* leaf, Camera* cam, bool onlyShadowCasters);
		
			/** Caches a face group for imminent rendering. */
			unsigned int cacheGeometry(unsigned int* pIndexes, const StaticFaceGroup* faceGroup);
		
			/** Frees up allocated memory for geometry caches. */
			void freeMemory(void);
		
			/** Adds a bounding box to draw if turned on. */
			void addBoundingBox(const AxisAlignedBox& aab, bool visible);
		
			// TODO: ???
			typedef std::set<const MovableObject*> MovablesForRendering;
			MovablesForRendering mMovablesForRendering;
		
			// debugging var. first time frame rendered
			bool	firstTime;
			
			// the list of cells to be processed / allready processed
			std::vector<DarkSceneNode *> mActiveCells;
			size_t mActualPosition; // actual position in the Active Cell list TODO: Make a local variable?
			
		public:
			DarkSceneManager(const String& instanceName);
			~DarkSceneManager();
		
			/** Not implemented. Please use the supplied methods for now */
			void setWorldGeometry(const String& filename);
		
			/**  Not implemented. Returns 0. */
			size_t estimateWorldGeometry(const String& filename);
			
			/** Tells the manager whether to draw the axis-aligned boxes that surround
			    nodes in the Bsp tree. For debugging purposes.
			*/
			void showNodeBoxes(bool show);
		
			/** Specialised to suggest viewpoints. */
			ViewPoint getSuggestedViewpoint(bool random = false);
		
			/** Overriden from SceneManager. */
			void _findVisibleObjects(Camera* cam, bool onlyShadowCasters);
		
			/** Creates a specialized BspSceneNode */
			SceneNode * createSceneNode ( void );
			/** Creates a specialized BspSceneNode */
			SceneNode * createSceneNode ( const String &name );
		
			/** Internal method for tagging BspNodes with objects which intersect them. */
			void _notifyObjectMoved(const MovableObject* mov, const Vector3& pos);
			
			/** Internal method for notifying the level that an object has been detached from a node */
			void _notifyObjectDetached(const MovableObject* mov);
		
			/** return the SceneManager type name */
			const String& getTypeName(void) const;
			
			/** Sets the BSP tree to the new value, specified by the root BspNode. 
			* @note The rootNode is unallocated using delete[] when freeing the level geometry. */
			void setBspTree(BspNode *rootNode);
			
			/** @copydoc SceneManager::clearScene */
			void clearScene(void);
			
			/** Creates an AxisAlignedBoxSceneQuery for this scene manager. 
			@remarks
			    This method creates a new instance of a query object for this scene manager, 
			    for an axis aligned box region. See SceneQuery and AxisAlignedBoxSceneQuery 
			    for full details.
			@par
			    The instance returned from this method must be destroyed by calling
			    SceneManager::destroyQuery when it is no longer required.
			@param box Details of the box which describes the region for this query.
			@param mask The query mask to apply to this query; can be used to filter out
			    certain objects; see SceneQuery for details.
			*/
			/*
			virtual AxisAlignedBoxSceneQuery* 
			    createAABBQuery(const AxisAlignedBox& box, unsigned long mask = 0xFFFFFFFF);
			*/
			/** Creates a SphereSceneQuery for this scene manager. 
			@remarks
			    This method creates a new instance of a query object for this scene manager, 
			    for a spherical region. See SceneQuery and SphereSceneQuery 
			    for full details.
			@par
			    The instance returned from this method must be destroyed by calling
			    SceneManager::destroyQuery when it is no longer required.
			@param sphere Details of the sphere which describes the region for this query.
			@param mask The query mask to apply to this query; can be used to filter out
			    certain objects; see SceneQuery for details.
			*/
			/*
			virtual SphereSceneQuery* 
			    createSphereQuery(const Sphere& sphere, unsigned long mask = 0xFFFFFFFF);
			*/
			/** Creates a RaySceneQuery for this scene manager. 
			@remarks
			    This method creates a new instance of a query object for this scene manager, 
			    looking for objects which fall along a ray. See SceneQuery and RaySceneQuery 
			    for full details.
			@par
			    The instance returned from this method must be destroyed by calling
			    SceneManager::destroyQuery when it is no longer required.
			@param ray Details of the ray which describes the region for this query.
			@param mask The query mask to apply to this query; can be used to filter out
			    certain objects; see SceneQuery for details.
			*/
			virtual RaySceneQuery* 
			    createRayQuery(const Ray& ray, unsigned long mask = 0xFFFFFFFF);
			/** Creates an IntersectionSceneQuery for this scene manager. 
			@remarks
			    This method creates a new instance of a query object for locating
			    intersecting objects. See SceneQuery and IntersectionSceneQuery
			    for full details.
			@par
			    The instance returned from this method must be destroyed by calling
			    SceneManager::destroyQuery when it is no longer required.
			@param mask The query mask to apply to this query; can be used to filter out
			    certain objects; see SceneQuery for details.
			*/
			virtual IntersectionSceneQuery* 
			    createIntersectionQuery(unsigned long mask = 0xFFFFFFFF);

    };

    /** BSP specialisation of IntersectionSceneQuery */
    class BspIntersectionSceneQuery : public DefaultIntersectionSceneQuery
    {
    public:
        BspIntersectionSceneQuery(SceneManager* creator);

        /** See IntersectionSceneQuery. */
        void execute(IntersectionSceneQueryListener* listener);

    };

    /** BSP specialisation of RaySceneQuery */
    class BspRaySceneQuery : public DefaultRaySceneQuery
    {
    public:
        BspRaySceneQuery(SceneManager* creator);
        ~BspRaySceneQuery();

        /** See RaySceneQuery. */
        void execute(RaySceneQueryListener* listener);
    protected:
        /// Set for eliminating duplicates since objects can be in > 1 node
        std::set<MovableObject*> mObjsThisQuery;
        /// list of the last single intersection world fragments (derived)
        std::vector<SceneQuery::WorldFragment*> mSingleIntersections;

        void clearTemporaries(void);
        /** Internal processing of a single node.
        @returns true if we should continue tracing, false otherwise
        */
        bool processNode(const BspNode* node, const Ray& tracingRay, RaySceneQueryListener* listener,
            Real maxDistance = Math::POS_INFINITY, Real traceDistance = 0.0f);
        /** Internal processing of a single leaf.
        @returns true if we should continue tracing, false otherwise
        */
        bool processLeaf(const BspNode* node, const Ray& tracingRay, RaySceneQueryListener* listener,
            Real maxDistance = Math::POS_INFINITY, Real traceDistance = 0.0f);

    };
    
	/// Factory for DarkSceneManager
	class DarkSceneManagerFactory : public SceneManagerFactory
	{
	protected:
		void initMetaData(void) const;
	public:
		DarkSceneManagerFactory() {}
		~DarkSceneManagerFactory() {}
		/// Factory type name
		static const String FACTORY_TYPE_NAME;
		SceneManager* createInstance(const String& instanceName);
		void destroyInstance(SceneManager* instance);
	};
}

#endif
