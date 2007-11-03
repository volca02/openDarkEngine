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
#include "OgreBspPrerequisites.h"
#include "OgreDarkSceneNode.h"
#include "OgreBspTree.h"

namespace Ogre {

	#define PLANE_DISTANCE_CORRECTION 0.00001
	
	/** BSP/Portal based scene manager.
	*
	* This sceneManager uses BSP tree and Portals. Each cell (leaf node of the BSP tree) is a convex cell, that is constructed with a 
	* set of textured polygons and portal polygons, which enclose the whole cell. The cells are convex pieces of geometry. For the portal
	* polygons, the visibility is evaluated, and if the portal is visible, the cell that is connected by the portal is rendered as well.
	*
	* @note This scene manager does not support portal movement, as the Intersection querries and visibility evaluators would have to be modified (and yet the modifying code would have to leave the cell convex and without holes) 
	*/
	class DarkSceneManager : public SceneManager {
		protected:
			bool mTraversalLog;
			
			// some statistics
			unsigned int 	mActualFrame;
			unsigned int	mBackfaced;
			unsigned int	mCellsRendered;
			unsigned int	mEvalPortals;
		    unsigned int 	mTraversalTime;
			unsigned int	mStaticRenderTime;
			
			// State variables for rendering WIP
			RenderOperation mRenderOp;
		
			// Debugging features
			bool mShowNodeAABs;
			RenderOperation mAABGeometry;
		
			BspTree *mBspTree;
		
			PortalList mVisiblePortals;
			
			// Set of already included face groups
			typedef std::set<int> FaceGroupSet;
			FaceGroupSet mFaceGroupSet;
			
			// Material -> face group hashmap
			typedef std::map<Material*, std::vector<StaticFaceGroup*>, materialLess > MaterialFaceGroupMap;
			MaterialFaceGroupMap mMatFaceGroupMap;
		
			/** Bsp Leaf nodes */
			Ogre::BspNode* mLeafNodes;
			unsigned int mNumLeafNodes;
			
			/** Bsp non-leaf nodes */
			Ogre::BspNode* mNonLeafNodes;
			unsigned int mNumNonLeafNodes;
		
			/// Global static level geometry vertex data 
			Ogre::VertexData* mVertexData;
		
			/// Global static level geometry index data 
			Ogre::HardwareIndexBufferSharedPtr mIndexes;
			/// Size of the index buffer (entries)
 			unsigned int mNumIndexes;

			/// Face groups
			Ogre::StaticFaceGroup* mFaceGroups;
			/// Face group count
			unsigned int mNumFaceGroups;
			
			
			/** Walks the BSP tree looking for the node which the camera
			    is in, and tags any geometry which is in a visible leaf for
			    later processing using Portal Visibility.
			    @param camera Pointer to the viewpoint.
			    @param queue The render queue to use
			    @param onlyShadowCasters if true, only shadow casters are queued
			    @returns The BSP node the camera was found in, for info.
			*/
			BspNode* walkTree(Camera* camera, RenderQueue *queue, bool onlyShadowCasters);
			
			/** 
			* Queues all movables attached to the given BspNode for rendering. Skips those already queued.
			* Also prepares static geometry faces for later rendering if onlyShadowCasters is false
			*/
			void queueBspNode(BspNode* node, Camera* camera, bool onlyShadowCasters);
			
			/**
			* Prepares the cell (visited first time the frame) to be processed by the traversal routine
			*/
			void prepareCell(BspNode *cell, Camera *camera, Matrix4& toScreen, PortalFrustum *frust);
			
			/**
			* Traverses the scene using portals. Fills the mCellList vector with visible SceneNodes.
			*/
			void traversePortals(BspNode *cell, Camera* camera, RenderQueue *queue, PortalRect &viewrect, bool onlyShadowCasters);
			
			/** Tags geometry in the leaf specified for later rendering. */
			void processVisibleLeaf(BspNode* leaf, Camera* cam, bool onlyShadowCasters);
		
			/** Caches a face group for imminent rendering. */
			unsigned int cacheGeometry(unsigned int* pIndexes, const StaticFaceGroup* faceGroup);
		
			/** Renders the static geometry - e.g. visible parts of the level data */ 
			void renderStaticGeometry(void);
		
			/** Renders the visible portals as wireframe - debug method 
			* @note Must be enabled using the setOption - ShowPortals */
			void renderPortals(void);
			
			/** Frees up allocated memory for geometry caches. */
			void freeMemory(void);
		
			/** Adds a bounding box to draw if turned on. */
			void addBoundingBox(const AxisAlignedBox& aab, bool visible);
		
			// 
			typedef std::set<const MovableObject*> MovablesForRendering;
			
			/** MovableObjects listed that will get inserted into the renderQueue */
			MovablesForRendering mMovablesForRendering;
		
			/// Display the visible portals as wireframe (set through setOption - ShowPortals)
			bool mShowPortals;
			
			/// the list of cells to be processed / allready processed
			std::vector<BspNode *> mActiveCells;
			/// actual position in the Active Cell list TODO: Make a local variable?
			size_t mActualPosition; 
			
		public:
			/** Constructor */
			DarkSceneManager(const String& instanceName);
			
			/** Destructor */
			~DarkSceneManager();
		
			/** Not implemented. Please use the supplied methods for now */
			void setWorldGeometry(const String& filename);
		
			/**  Not implemented. Returns 0. */
			size_t estimateWorldGeometry(const String& filename);
			
			/** The static world geometry setter method. Caller supplies a vertex buffer and index buffer, a set of faces. 
			* The acompaniing method setBspTree sets the BSP tree that contains face lists for each cell.
			* @see setBspTree
			*/ 
			void setStaticGeometry(Ogre::VertexData* vertexData, Ogre::HardwareIndexBufferSharedPtr indexes, 
					Ogre::StaticFaceGroup* faceGroups, unsigned int numIndexes, unsigned int numFaceGroups);
		
			/** Tells the manager whether to draw the axis-aligned boxes that surround
			    nodes in the Bsp tree. For debugging purposes.
			*/
			void showNodeBoxes(bool show);
		
			/** Specialised to suggest viewpoints. */
			ViewPoint getSuggestedViewpoint(bool random = false);
		
			/** Overriden from SceneManager. */
			void _findVisibleObjects(Camera *cam, VisibleObjectsBoundsInfo *visibleBounds, bool onlyShadowCasters);
		
			/** Overriden from SceneManager. */
			void _renderVisibleObjects(void);
		
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
			* @note The rootNode is unallocated using delete[] when freeing the level geometry. 
			* @note The previous BSP tree is not freed
			* @see BspTree::setBspTree */
			void setBspTree(BspNode *rootNode, BspNode *leafNodes, BspNode* nonLeafNodes, size_t leafNodeCount, size_t nonLeafNodeCount);
			
			/** return the root node of the bsp tree */
			const BspNode* getRootBspNode();
			
			/** returns the pointer to the BspTree class which holds the Bsp tree */
			BspTree* getBspTree() const { return mBspTree; };
			
			/** Clears the scene */
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
			
			/**
				Set a scene manager option.
				@remarks
					Options:
						"ShowPortals" : bool * - display visible portals as wireframe
			*/
			bool setOption( const String & key, const void * val );

			/**
				Get a scene manager option
				@remarks
					Options:
						ShowPortals - bool - True if portals are rendered as wireframe
						BackfaceCulls - unsigned int - number of portals that were backfaced
						CellsRendered - count of rendered cells
						EvaluatedPortals - Count of portals evaluated for visibility
			*/
			bool getOption( const String & key, void *val );
    };

    /** BSP specialisation of IntersectionSceneQuery */
    class BspIntersectionSceneQuery : public DefaultIntersectionSceneQuery
    {
    public:
        BspIntersectionSceneQuery(SceneManager* creator);

        /** See IntersectionSceneQuery. */
        void execute(IntersectionSceneQueryListener* listener);

    };

    /** BSP/Portal specialisation of RaySceneQuery */
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
	
        /** Internal processing of a leaf node traversal
        */
        void traverseLeafNodes(const BspNode* leaf, Ray tracingRay, RaySceneQueryListener* listener);

	/** Helper method getting the nearest intersection for convex volume of planes, when inside of it.
	* if intersection occured, the planeindex is filled up with the index of the plane which resulted in the returned minimal distance */
	std::pair<bool, Real> rayIntersects(const Ray& ray, 
					    const std::list<Plane>& planes, bool normalIsOutside, int& planeindex, Plane& cPlane);
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
