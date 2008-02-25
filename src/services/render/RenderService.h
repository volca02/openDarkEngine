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
			
		protected:
            virtual bool init();
            virtual void bootstrapFinished();
            
			virtual void loopStep(float deltaTime);

            void onPropModelNameMsg(const PropertyChangeMsg& msg);
            void onPropLightMsg(const PropertyChangeMsg& msg);
            void onPropSpotlightMsg(const PropertyChangeMsg& msg);

            void prepareMesh(const Ogre::String& name);

			/// Removes entity from the given object (meaning it will not have a visible representation)
			void removeObjectEntity(int id);
			
            void clear();
            
   			// A package of a light and it's scene node
			struct LightInfo {
				Ogre::Light* light;
				Ogre::SceneNode* node;
			};
			
			// A package of an entity and a EntityMaterialInstance
			struct EntityInfo {
				Ogre::Entity* entity;
				EntityMaterialInstance* emi;
			};

            /// Creates a new light with next to no initialization
            LightInfo& createLight(int objID);
            
            /// Updates a light based on the data of property LIGHT
			void updateLight(LightInfo& li, const PropertyDataPtr& propLightData);
			
			/// removes a light
   			void removeLight(int id);
   			
   			/// Updates a light to be a spotlight, updates it's params
   			void updateSpotLight(LightInfo& li, const PropertyDataPtr& propSpotData);
   			
   			/// removes spotlight quality from a light (leaves the light as a point light)
   			void removeSpotLight(int id);


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
			
			// SpotLight Propert related
			PropertyGroup::ListenerID mPropSpotlightListenerID;
			PropertyGroupPtr mPropSpotlight;

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
			
			
			typedef std::map<int, LightInfo> LightInfoMap;
			
			LightInfoMap mLightInfoMap;
			
			// Database listener handle related (For render params chunk)
			// DatabaseServicePtr mDatabaseService;
			// BinaryServicePtr mBinaryService;
			
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
