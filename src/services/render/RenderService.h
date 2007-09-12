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
 *****************************************************************************/


#ifndef __RENDERSERVICE_H
#define __RENDERSERVICE_H

#include "ServiceCommon.h"
#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "PropertyService.h"
#include "SharedPtr.h"
#include "ManualBinFileLoader.h"
#include <OgreEntity.h>
#include <OgreSceneNode.h>

namespace Opde {

	/** @brief Render service - service managing in-game object rendering
	* Some preliminary notes: VHot will probably be converted to TagPoints somehow
	* The 0,0,0 bug will reveal itself here too, maybe. This is caused by default object coordinates being 0,0,0 and we have to wait for Position prop message to come in order to move the node
	*/
	class RenderService : public Service, public PropertyChangeListener {
		public:
			RenderService(ServiceManager *manager);
			virtual ~RenderService();

			virtual void init();

		protected:
            void onPropPositionMsg(const PropertyChangeMsg& msg);
            void onPropModelNameMsg(const PropertyChangeMsg& msg);

            void createSceneNode(const PropertyChangeMsg& msg);
            void setSceneNodePosition(const PropertyChangeMsg& msg);
            void removeSceneNode(const PropertyChangeMsg& msg);

            void prepareMesh(const Ogre::String& name);

            /** Returns a scene node for a given object ID
            * @param objID The object ID for which to find the scene node
            * @param create If true, the method will create a new scene node on 0,0,0 if no was found
            * @return SceneNode pointer if successfull, NULL otherwise */
            Ogre::SceneNode* getSceneNode(int objID, bool create = false);

            void clear();

            static Ogre::Quaternion toOrientation(PropertyDataPtr posdata);

			/// Map of objectID -> Entity
			typedef std::map<int, Ogre::Entity*> ObjectEntityMap;
			/// Map of objectID -> SceneNode (To which the Entity connects)
			typedef std::map<int, Ogre::SceneNode*> ObjectSceneNodeMap;


			ObjectSceneNodeMap mSceneNodeMap;
			ObjectEntityMap mEntityMap;

			// Listener structs for property messages. We have 2 now: Position and ModelName
			PropertyChangeListenerPtr mPropPositionListener;
			PropertyGroupPtr mPropPosition;

			PropertyChangeListenerPtr mPropModelNameListener;
			PropertyGroupPtr mPropModelName;


			/// Shared pointer to the property service
			PropertyServicePtr mPropertyService;

            Ogre::Root* mRoot;
			Ogre::SceneManager* mSceneMgr;

			/// Manual loader for bin meshes
			Ogre::ManualBinFileLoader* mManualBinFileLoader;
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

			virtual const unsigned int getMask() { return SERVICE_PROPERTY_LISTENER; };

		private:
			static std::string mName;
	};
}


#endif
