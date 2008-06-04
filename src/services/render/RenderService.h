/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *		$Id$
 *
 *****************************************************************************/


#ifndef __RENDERSERVICE_H
#define __RENDERSERVICE_H

#include "ServiceCommon.h"
#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "PropertyService.h"
#include "SharedPtr.h"
#include "ManualBinFileLoader.h"
#include "DarkSceneManager.h"
#include "MessageSource.h"
#include "LoopService.h"
#include "ObjectService.h"
#include "EntityMaterialInstance.h"

#include <OgreEntity.h>
#include <OgreLight.h>
#include <OgreSceneNode.h>

namespace Opde {
	/// Render System message type
	typedef enum {
	    /// Render window changed the size
	    RENDER_WINDOW_SIZE_CHANGE = 1
	} RenderMessageType;	


	/// window size message details
	typedef struct RenderWindowSize {
		/// The message type
		RenderMessageType msg_type;
		/// New width of the window (pixels)
		unsigned int width;
		/// New height of the window (pixels)
		unsigned int height;
		/// The new window is fullscreen (or windowed)
		bool fullscreen;
		// TODO: Pixelformat
	};

	/// Render service message (Used to signalize a change in the renderer setup)
	typedef union {
		RenderMessageType msg_type;
		
		RenderWindowSize size;
	} RenderServiceMsg;

	/** @brief Render service - service managing in-game object rendering
	* Some preliminary notes: VHot will probably be converted to TagPoints somehow
	* The 0,0,0 bug will reveal itself here too, maybe. This is caused by default object coordinates being 0,0,0 and we have to wait for Position prop message to come in order to move the node
	*/
	class RenderService : public Service, public MessageSource< RenderServiceMsg >, public LoopClient {
		public:
			RenderService(ServiceManager *manager, const std::string& name);
			virtual ~RenderService();

            Ogre::Root* getOgreRoot();
            Ogre::SceneManager* getSceneManager();
            Ogre::RenderWindow* getRenderWindow();
            
            /**
            Getter for the default viewport. Most of the time sufficient (for game)
            @returns The default viewport (Viewport of the default camera )
			*/
            Ogre::Viewport* getDefaultViewport();
            
            /**
            Getter for the default camera. Default camera is a camera named "DefaultCamera" that is autocreated and added to the autocreated window
            @returns The default camera
			*/
            Ogre::Camera* getDefaultCamera();

			// TODO: Here, we need some code to gather possible resolutions to switch to (to fill the GUI with options).
			/// Screen size setter. Please use this instead of the Ogre::RenderWindow methods, as it broadcasts a message about the change
			void setScreenSize(bool fullScreen, unsigned int width, unsigned int height);


			/** Simplified version of the SpecialCaseRenderQueue switching. Enables/Disables world display based on SCRQ_INCLUDE/SCRQM_EXCLUDE
				@param visible If set to true, normal rendering takes place, false causes only overlay render queue to be processed
			*/
			void setWorldVisible(bool visible);
			
			/** Getter for SceneNodes of objects. Those only exist for concrete objects.
			@returns The scene node of the object id (only for concrete objects)
			@throws BasicException if the method is used on archetypes or on invalid object id (unused id)
			*/
			Ogre::SceneNode* getSceneNode(int objID);
			
			/** Attaches the default camera to a defined object
			* @todo Framework to work with cameras, rendertargets, etc. Should a service be made for this, move this inside such 
			* @todo This one needs special handling if placed on Player object (leaning scene node(s)) - could be solved by placing a child SN "anywhen" attached */
			void attachCameraToObject(int objID);
			
			/** Camera is detached from the specified object */
			void detachCamera();
		protected:
            virtual bool init();
            virtual void bootstrapFinished();
            
			virtual void loopStep(float deltaTime);

			/** Model name property callback
			* @todo Default to the same model as dark (see if T1/T2/SS2 use the same model for concretes not having this property specified)
			*/
            void onPropModelNameMsg(const PropertyChangeMsg& msg);
            
            /// Property Light change callback
            void onPropLightMsg(const PropertyChangeMsg& msg);
            
            /// Property Spotlight change callback
            void onPropSpotlightMsg(const PropertyChangeMsg& msg);
            
   			/// Position property change callback
			void onPropPositionMsg(const PropertyChangeMsg& msg);
			
			/// Scale property change callback
			void onPropScaleMsg(const PropertyChangeMsg& msg);
			
			/// Render type property change callback
			void onPropRenderTypeMsg(const PropertyChangeMsg& msg);
			
			/// Render alpha property change callback
			void onPropRenderAlphaMsg(const PropertyChangeMsg& msg);
			
			/// Object creation/destruction callback
			void onObjectMsg(const ObjectServiceMsg& msg);
			
			/// Prepares mesh named "name" for usage on entity (if it was not prepared already)
            void prepareMesh(const Ogre::String& name);

			/// Initializes the object's model
			void createObjectModel(int id);
			
			/** Removes geometry from the given object (meaning it will not have a visible representation).
			* This method is used to do a cleanup on object's destroy event.
			*/
			void removeObjectModel(int id);
			
			/** Call this to set a new model for the object. Preserves all the object's rendering settings
			* @note setting empty model name means the model won't be rendered. This is used for FX_Particle for example
			*/
			void setObjectModel(int id, const std::string& name);
			
			/// Prepares an entity to be used by renderer (manual bones, etc.)
			void prepareEntity(Ogre::Entity* e);

			/// Clears all the rendering data
            void clear();
            
            /// prepares some hardcoded media (included in the executable)
            void prepareHardcodedMedia();
            
            /// Cretes a ramp mesh, that is used as a default mesh when not specified otherwise
            void createRampMesh();
            
   			/// A package of a light and it's scene node
			struct LightInfo {
				Ogre::Light* light;
				Ogre::SceneNode* node;
			};
			
			/// A package of an entity and a EntityMaterialInstance
			struct EntityInfo {
				unsigned int renderType;
				bool hasRefs;

				/// Used by FX_Particle and such. Hides without interfering with the previous two
				bool skip;

				Ogre::Entity* entity;
				Ogre::SceneNode* node;
				EntityMaterialInstance* emi;
			};

            /// Creates a new light with next to no initialization
            LightInfo& createLight(int objID);
            
            /// Updates a light based on the data of property LIGHT
			void updateLight(LightInfo& li, int objectID);
			
			/// removes a light
   			void removeLight(int id);
   			
   			/// Updates a light to be a spotlight, updates it's params
   			void updateSpotLight(LightInfo& li, int objectID);
   			
   			/// removes spotlight quality from a light (leaves the light as a point light)
   			void removeSpotLight(int id);

			/// set's the node's visibility based on the rendertype property value
			void setNodeRenderType(Ogre::SceneNode* node, const EntityInfo& ei);

			/// Map of objectID -> Entity
			typedef std::map<int, EntityInfo> ObjectEntityMap;

			ObjectEntityMap mEntityMap;

			// Listener structs for property messages
			
			// ModelName listener related
			PropertyGroup::ListenerID mPropModelNameListenerID;
			PropertyGroupPtr mPropModelName;
			
			// Light Property related
			PropertyGroup::ListenerID mPropLightListenerID;
			PropertyGroupPtr mPropLight;
			
			// SpotLight Property related
			PropertyGroup::ListenerID mPropSpotlightListenerID;
			PropertyGroupPtr mPropSpotlight;
			
			// "Position" Property related
			PropertyGroup::ListenerID mPropPositionListenerID;
			PropertyGroupPtr mPropPosition;
			
			// "ModelScale" Property related
			PropertyGroup::ListenerID mPropScaleListenerID;
			PropertyGroupPtr mPropScale;
			
			// "RenderType" Property related
			PropertyGroup::ListenerID mPropRenderTypeListenerID;
			PropertyGroupPtr mPropRenderType;
			
			// "RenderAlpha" Property related
			PropertyGroup::ListenerID mPropRenderAlphaListenerID;
			PropertyGroupPtr mPropRenderAlpha;
			


			/// Shared pointer to the property service
			PropertyServicePtr mPropertyService;

            // --- RENDERING INSTANCES ---
            /// Ogre root handle
            Ogre::Root* mRoot;
            
            /// Scene manager handle
			Ogre::SceneManager* mSceneMgr;
			
			/// Render window handle
			Ogre::RenderWindow* mRenderWindow;
			
			/// Factory instance for the DarkSceneManager
			Ogre::DarkSceneManagerFactory* mDarkSMFactory;
			
			/// Default camera. Used solely for game mode
			Ogre::Camera* mDefaultCamera;

			/// Manual loader for bin meshes
			Ogre::ManualBinFileLoader* mManualBinFileLoader;
			
			/// Pointer for loop service
			LoopServicePtr mLoopService;
			
			/// Shared ptr to object service (for scene nodes)
			ObjectServicePtr mObjectService;
			ObjectService::ListenerID mObjSystemListenerID;
			
			
			typedef std::map<int, LightInfo> LightInfoMap;
			
			LightInfoMap mLightInfoMap;
			
			// Database listener handle related (For render params chunk)
			// DatabaseServicePtr mDatabaseService;
			// BinaryServicePtr mBinaryService;
			
			
			/** Object id to scene node (only concrete objects)
			* This map stores the base scenenode for the object. Object have this sceneNode structure:
			*
			* Base SceneNode:
			*	@li Child 1 - the Rendered Objects scene node
			*	@li Child 2 - the Light object scene node
			*/
			typedef std::map<int, Ogre::SceneNode*> ObjectToNode;
			
			/// Mapping of object id to scenenode
			ObjectToNode mObjectToNode;
			
			/// editor mode display
			bool mEditorMode;
	};

	/// Shared pointer to Link service
	typedef shared_ptr<RenderService> RenderServicePtr;

	/// Factory for the LinkService objects
	class RenderServiceFactory : public ServiceFactory {
		public:
			RenderServiceFactory();
			~RenderServiceFactory() {};

			/** Creates a LinkService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

			virtual const unsigned int getMask() { return SERVICE_PROPERTY_LISTENER | SERVICE_RENDERER; };

		private:
			static std::string mName;
	};
}


#endif
