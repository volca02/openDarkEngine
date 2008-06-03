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
 
 
#ifndef __DARKSCENEMANAGER_H
#define __DARKSCENEMANAGER_H

#include <OgreSceneManager.h>

#include "DarkBspPrerequisites.h"
#include "DarkLight.h"

namespace Ogre {

	/** Portal + BSP based SceneManager targetted at DE based levels. 
	* This SceneManager is targetted at scenes composed of great amount of convex cells connected with portals.
	* Each cell has to be a leaf node of the supplied Bsp node tree. Portals are created */
	class DarkSceneManager : public SceneManager {
			// So we don't need to expose getBspTree() method to public
			friend class DarkCamera;
			friend class DarkLight;
			
		public:
			/// Constructor
			DarkSceneManager(const String& instanceName);
			
			/// Destructor
			~DarkSceneManager();
			
			/** Clears the scene. 
			* @note Delete's the current BSP tree and creates a new one
			*/
			virtual void clearScene(void);
			
			/** Creates a new portal from src to dst on a plane 'plane' 
			* @param src the source cell (leaf bsp node) to attach to
			* @param dst the destination cell (leaf bsp node) to attach to
			* @param plane the plane on which the portal should be placed
			* @return new portal instance (to be destroyed with destroyPortal call)
			* @note the returned portal has to be filled with points to be usable */
			Portal* createPortal(BspNode* src, BspNode* dst, const Plane& plane);
			
			/** Creates a new portal from src to dst on a plane 'plane'.
			* @param src the source cell (leaf) ID to attach to
			* @param dst the destination cell (leaf) ID to attach to
			* @param plane the plane on which the portal should be placed
			* @return new portal instance (to be destroyed with destroyPortal call)
			* @note the returned portal has to be filled with points to be usable */
			Portal* createPortal(int srcLeafID, int dstLeafID, const Plane& plane);
			
			/** Destroys the given portal */
			void destroyPortal(Portal* portal);
			
			/** Creates our specialized camera (DarkCamera) */
			virtual Camera* createCamera(const String& name);
			
			/** Specialised scene graph update method. Update's the Camera's visible cell list if needed */
			virtual void _updateSceneGraph(Camera* cam);
			
			/** creates a new BSP node 
			* @param id the bsp node id
			* @param leafID the id of the cell (leaf) - if >=0 then this BSP node is marked as leaf, and is registered as a cell with ID leafID 
			*/
			BspNode* createBspNode(int id, int leafID = -1);
			
			/** gets a BSP node by id */
			BspNode* getBspNode(int id);
			
			/** gets a BSP Leaf node by leaf id */
			BspNode* getBspLeaf(int leafID);
			
			/** sets new root bsp node */
			void setRootBspNode(int id);
			
			/// Specialized version of SceneNode creation. Creates DarkSceneNode instances
			virtual SceneNode* createSceneNode(void);
			
			/// Specialized version of SceneNode creation. Creates DarkSceneNode instances
			virtual SceneNode* createSceneNode(const String& name);
			
			/// Internal method that updates scenenode membership in BSP tree
			void _notifyObjectMoved(const MovableObject* mov, const Vector3& pos);
			
			/** Internal method, makes sure an object is removed from the leaves when detached from a node. */ 
			void _notifyObjectDetached(const MovableObject* mov);
		
			/** Overriden visible object finder which only enlists the object found in Camera's visible cells */
			virtual void _findVisibleObjects(Camera* cam, VisibleObjectsBoundsInfo* visibleBounds, bool onlyShadowCasters);
			
			/// Scene manager type name getter
			virtual const String& getTypeName(void) const;
			
			/// Specialized method for creating DarkLight objects
			virtual Light* createLight(const String& name);
			
			/// Specialized method for getting DarkLight objects
			virtual Light* getLight(const String& name);
			
			/// Specialized method for querying the existence of DarkLight objects
			virtual bool hasLight(const String& name);
			
			/// Specialized method for destroying DarkLight objects
			virtual void destroyLight(const String& name);
			
			/// Specialized method for destroying all DarkLight objects
			virtual void destroyAllLights(void);
			
			/// Queues a light to be included in the pre-render update
			/// @note Do call this method after manipulating the light's parameters
			void queueLightForUpdate(Light* l);
			
			/// Finds lights that are affecting the cells the camera sees
			virtual void findLightsAffectingFrustum(const Camera* camera);
			
			/** Populates a light list by looking into the BSP tree and getting all the lights from leaf nodes the MovableObject is in 
			 (customized to get use of caching of BSPTree) */
			virtual void _populateLightList(const Vector3 &position, Real radius, LightList &destList);
			
			/** Overrided entity creation. Sets BspTree as the entity listener - this accelerates the population of the light lists */
			virtual Entity *createEntity(const String &entityName, const String &meshName);
			
			// TODO: The DarkGeometry should probably have a factory
			/** Creates an instance of static geometry */
			DarkGeometry *createGeometry(const String& geomName);
			
			/** destroys an instance of static geometry */
			void destroyGeometry(const String& name);
			
			/** retrieves a pointer to existing static geometry */
			DarkGeometry *getGeometry(const String& name);
			
			/** sets a geometry that will be active and rendered */
			void setActiveGeometry(const String& name);
			
			/** sets the active geometry (NULL means no geom) */
			void setActiveGeometry(DarkGeometry* g);
			
			/** gets an option from this scenemanager 
			* @param strKey the option name (valid options: StaticBuildTime - unsigned long) */
			virtual bool getOption(const String &strKey, void *pDestValue);
		
		protected:
			/// BSP Tree getter
			BspTree* getBspTree(void) { return mBspTree; };
			
			/// routine that updates dirty lights before rendering takes place
			void updateDirtyLights();
			
			/// Internal method, which really queues the light
			void _queueLightForUpdate(Light* l);
			
			/// destroys all static geometries
			void destroyAllGeometries(void);
			
			/// The BSP tree currently used
			BspTree* mBspTree;
			
			typedef std::set<Portal*> PortalSet;
			
			PortalSet mPortals;
			
			typedef std::set<DarkLight*> LightSet;
			
			LightSet mLightsForUpdate;
			
			/// Current frame number
			int mFrameNum; 
			
			/// Time it took to build the static geometry
			unsigned long mStaticBuildTime;

			/** movables found to be visible */
			typedef std::set<const MovableObject*> MovablesForRendering;

            /** MovableObjects listed that will get inserted into the renderQueue */
            // MovablesForRendering mMovablesForRendering;
            
            /** Factory for DarkLight objects */
            DarkLightFactory *mDarkLightFactory;
            
            /** Map of dark geometry objects */
            typedef std::map<String, DarkGeometry*> DarkGeometryMap;
            
            DarkGeometryMap mDarkGeometryMap;
            
            DarkGeometry* mActiveGeometry;
	};

	/// Factory for DarkSceneManager
	class DarkSceneManagerFactory : public SceneManagerFactory {
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

	
};

#endif
