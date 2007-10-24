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

#include "RenderService.h"

#include "PropertyService.h"

#include "logger.h"
#include "ServiceCommon.h"

#include <OgreRoot.h>
#include <OgreStringConverter.h>
#include <OgreMeshManager.h>
#include <OgreRenderWindow.h>
#include <OgreBone.h>
#include <OgreNode.h>

#include <OgreAnimation.h>

using namespace std;
using namespace Ogre;

namespace Opde {

	/*--------------------------------------------------------*/
	/*--------------------- RenderService --------------------*/
	/*--------------------------------------------------------*/
	RenderService::RenderService(ServiceManager *manager, const std::string& name) : Service(manager, name), mSceneMgr(NULL), mRoot(NULL), mDarkSMFactory(NULL) {
	    // TODO: This is just plain wrong. This service should be the maintainer of the used scene manager, if any other service needs the direct handle, etc.
	    // The fact is this service is probably game only, and should be the initialiser of graphics as the whole. This will be the
	    // modification that should be done soon in order to let the code look and be nice
	    // FIX!
	    mRoot = Root::getSingletonPtr();
	    mManualBinFileLoader = new ManualBinFileLoader();
	}

	// --------------------------------------------------------------------------
	RenderService::~RenderService() {
        LOG_INFO("RenderService::~RenderService()");

		if (!mPropPosition.isNull())
		    mPropPosition->unregisterListener(mPropPositionListenerID);

		if (!mPropModelName.isNull())
		    mPropModelName->unregisterListener(mPropModelNameListenerID);

        if (mDarkSMFactory) {
			Root::getSingleton().removeSceneManagerFactory(mDarkSMFactory);
			delete mDarkSMFactory;
			mDarkSMFactory = NULL;
		}
	}

    // --------------------------------------------------------------------------
    bool RenderService::init() {
        // TODO: These can be gathered from the config file. Usage of the config service would be a nice thing to do
        if( !mRoot->restoreConfig() ) {
        		// If there is no config file, show the configuration dialog
        		if( !mRoot->showConfigDialog() ) {
            			LOG_ERROR("RenderService::init: The renderer setup was canceled. Application termination in progress.");
            			return false;
        		}
        }

        // Initialise and create a default rendering window
		mRenderWindow = mRoot->initialise( true, "openDarkEngine" );

        // Dark scene manager factory
		mDarkSMFactory = new DarkSceneManagerFactory();

		// Register
		Root::getSingleton().addSceneManagerFactory(mDarkSMFactory);

		mRoot->createSceneManager(ST_INTERIOR, "DarkSceneManager");

        // Get the resolution parameters, and instantinate ogre display. We expect the Ogre::Root to be already defined
		mSceneMgr = mRoot->getSceneManager("DarkSceneManager"); // TODO: Really bad idea. Same goes to the WorldrepService

		return true;
    }


    // --------------------------------------------------------------------------
    Ogre::Root* RenderService::getOgreRoot() {
        assert(mRoot);
        return mRoot;
    }

    // --------------------------------------------------------------------------
    Ogre::SceneManager* RenderService::getSceneManager() {
        assert(mSceneMgr);
        return mSceneMgr;
    }

    // --------------------------------------------------------------------------
    Ogre::RenderWindow* RenderService::getRenderWindow() {
        assert(mRenderWindow);
        return mRenderWindow;
    }

	// --------------------------------------------------------------------------
	void RenderService::setScreenSize(bool fullScreen, unsigned int width, unsigned int height) {
		assert(mRenderWindow);
		
		mRenderWindow->setFullscreen(fullScreen, width, height);
		
		RenderServiceMsg msg;
		
		msg.msg_type = RENDER_WINDOW_SIZE_CHANGE;
		
		msg.size.fullscreen = fullScreen;
		msg.size.width = width;
		msg.size.height = height;
		
		broadcastMessage(msg);
	}
	
	// --------------------------------------------------------------------------
	void RenderService::bootstrapFinished() {
		// Property Service should have created us automatically through service masks.
		// So we can register as a link service listener
		LOG_INFO("RenderService::bootstrapFinished()");

		PropertyGroup::ListenerPtr cposc =
			new ClassCallback<PropertyChangeMsg, RenderService>(this, &RenderService::onPropPositionMsg);

		PropertyGroup::ListenerPtr cmodelc =
			new ClassCallback<PropertyChangeMsg, RenderService>(this, &RenderService::onPropModelNameMsg);


		// Get the PropertyService, then the group Position

		// contact the config. service, and look for the inheritance link name
		// TODO: ConfigurationService::getKey("Core","InheritanceLinkName").toString();

		mPropertyService = ServiceManager::getSingleton().getService("PropertyService");

		mPropPosition = mPropertyService->getPropertyGroup("Position");

		if (mPropPosition.isNull())
            OPDE_EXCEPT("Could not get Position property group. Not defined. Fatal", "RenderService::bootstrapFinished");

		mPropPositionListenerID = mPropPosition->registerListener(cposc);

		mPropModelName = mPropertyService->getPropertyGroup("ModelName"); // TODO: hardcoded, maybe not a problem after all

		if (mPropModelName.isNull())
            OPDE_EXCEPT("Could not get ModelName property group. Not defined. Fatal", "RenderService::bootstrapFinished");

		mPropModelNameListenerID = mPropModelName->registerListener(cmodelc);

		LOG_INFO("RenderService::bootstrapFinished() - done");
	}

	// --------------------------------------------------------------------------
	void RenderService::loopStep(float deltaTime) {
		// Rendering step...
		mRoot->renderOneFrame();
	}

	// --------------------------------------------------------------------------
	void RenderService::onPropPositionMsg(const PropertyChangeMsg& msg) {
		// Update the scene node's position and orientation
		// (hrm. The BspSceneNode should report cell it is in for us, but well, there is no need besides SaveGame compatibility)
		// Two keys are interesting: position and facing
		// TODO: Could we hijack the system so that the facing would be implemented by quaternions? Would this be worth the trouble?
		// If there is no sceneNode, we'll create one for messages about change and create
		if (msg.change == PROP_GROUP_CLEARED) {
                clear();
                return;
		}

		if (msg.objectID <= 0) // no action for archetypes
            return;

        // LOG_INFO("RenderService: Setting position for node of object %d", msg.objectID);

		switch (msg.change) {
			case PROP_ADDED   :
			case PROP_CHANGED : {
                // Find the scene node by it's object id, and update the position and orientation
                SceneNode* node = getSceneNode(msg.objectID, true);
                Vector3 position = msg.data->get("position").toVector();
                node->setPosition(position);

                LOG_INFO("RenderService: Setting position for node of object %d: %10.2f %10.2f %10.2f", msg.objectID, position.x, position.y, position.z);
                // Convert the orientation to quaternion
                node->setOrientation(toOrientation(msg.data));

				break;
			}
            case PROP_REMOVED:
                removeSceneNode(msg);
                break;
		}
	}

	// --------------------------------------------------------------------------
	void RenderService::onPropModelNameMsg(const PropertyChangeMsg& msg) {
		// Update the Model (mesh) for the object
		if (msg.change == PROP_GROUP_CLEARED) {
                clear();
                return;
		}

		if (msg.objectID <= 0) // no action for archetypes
            return;

        LOG_INFO("RenderService: Adding model for object %d", msg.objectID);

        Ogre::String name = msg.data->get("label"); // the model name

        // As a test, I'm loading cube.mesh
        switch (msg.change) {
			case PROP_ADDED: {
                    try {
                        prepareMesh(name);
                        // createMesh, so it will be prepared before the entity creation
                        //setObjectModel(msg.objectID, name);

                        // TODO: Create a MeshService. Then Entity* ent = meshService->createObjectEntity(name);
                        // That will react to DatabaseSevice notifications, and unload the meshes as needed when changing mission

                        Entity *ent = mSceneMgr->createEntity( "Object" + StringConverter::toString(msg.objectID), name + ".mesh" );
                        // Entity *ent = mSceneMgr->createEntity( "Object" + StringConverter::toString(msg.objectID), "jaiqua.mesh" );

                        // bind the ent to the node of the obj.
                        SceneNode* node = getSceneNode(msg.objectID, true);

                        node->attachObject(ent);

                        ent->_initialise(true);

                        // Update the bones to be manual
                        Skeleton::BoneIterator bi = ent->getSkeleton()->getBoneIterator();

                        while (bi.hasMoreElements()) {
                            Bone* n = bi.getNext();

                            n->setManuallyControlled(true);
                        }

                        ent->getSkeleton()->setBindingPose();


                    } catch (FileNotFoundException &e) {
                        LOG_ERROR("RenderService: Could not find the requested model %s (exception encountered)", name.c_str());
                    }

                    break;
                }
			case PROP_CHANGED:
                //setObjectModel(msg.objectID, name);
                break;
            case PROP_REMOVED:
                // removeSceneNode(msg);
                break;
		}
	}

    // --------------------------------------------------------------------------
    void RenderService::createSceneNode(const PropertyChangeMsg& msg) {
        // Search for scene node, if not created, create. then set position
        ObjectSceneNodeMap::iterator it = mSceneNodeMap.find(msg.objectID);

        if (it == mSceneNodeMap.end()) {
            SceneNode * node = mSceneMgr->createSceneNode("Object" + StringConverter::toString(msg.objectID));
            mSceneNodeMap.insert(make_pair(msg.objectID, node));
        }

        setSceneNodePosition(msg);
    }

    // --------------------------------------------------------------------------
    void RenderService::setSceneNodePosition(const PropertyChangeMsg& msg) {
        ObjectSceneNodeMap::iterator it = mSceneNodeMap.find(msg.objectID);

        if (it != mSceneNodeMap.end()) {
            Vector3 position  = msg.data->get("position").toVector();
            it->second->setPosition(position);

            // Convert the orientation to quaternion
            it->second->setOrientation(toOrientation(msg.data));
        }
    }

    // --------------------------------------------------------------------------
    SceneNode* RenderService::getSceneNode(int objID, bool create) {
        ObjectSceneNodeMap::iterator it = mSceneNodeMap.find(objID);

        if (it != mSceneNodeMap.end()) {
            return it->second;
        } else {
            if (create) {
                SceneNode * node = mSceneMgr->createSceneNode("Object" + StringConverter::toString(objID));
                mSceneNodeMap.insert(make_pair(objID, node));
                mSceneMgr->getRootSceneNode()->addChild(node);
                return node;
            } else {
                return NULL;
            }
        }
    }

    // --------------------------------------------------------------------------
    void RenderService::removeSceneNode(const PropertyChangeMsg& msg) {
        ObjectSceneNodeMap::iterator it = mSceneNodeMap.find(msg.objectID);

        if (it != mSceneNodeMap.end()) {
            mSceneNodeMap.erase(it);
        }
    }

    // --------------------------------------------------------------------------
    void RenderService::prepareMesh(const Ogre::String& name) {
        String fname = name + ".mesh";

        try {
            // First, try to load the mesh directly as a mesh file
            Ogre::MeshPtr mesh1 = Ogre::MeshManager::getSingleton().load(fname, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        } catch (FileNotFoundException &e) {
            // Undefine in advance, so there will be no clash
            Ogre::MeshManager::getSingleton().remove(fname);
            // If it is not found
            LOG_DEBUG("RenderService::prepareMesh: Mesh definition for %s not found, loading manually from .bin file...", name.c_str());
            // do a create

            try {
                Ogre::MeshPtr mesh1 = Ogre::MeshManager::getSingleton().create(fname, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                   true, mManualBinFileLoader);
            } catch (FileNotFoundException &e) {
                LOG_ERROR("RenderService::prepareMesh: Could not find the requested model %s", name.c_str());
            } catch (Opde::FileException &e) {
                LOG_ERROR("RenderService::prepareMesh: Could not load the requested model %s", name.c_str());
            }
        }

    }

    // --------------------------------------------------------------------------
    void RenderService::clear() {
        mSceneNodeMap.clear();
        mEntityMap.clear();
    }


    // --------------------------------------------------------------------------
    Ogre::Quaternion RenderService::toOrientation(PropertyDataPtr posdata) {
        // Hmm. ok. we have 3 values - x,y,z
        Ogre::Real x,y,z;

        x = (posdata->get("facing.x").toFloat() / 32768) * Math::PI; // heading - y
        y = (posdata->get("facing.y").toFloat() / 32768) * Math::PI; // pitch - x
        z = (posdata->get("facing.z").toFloat() / 32768) * Math::PI; // bank - z

        Matrix3 m;

        // Still not there, but close
        m.FromEulerAnglesYXZ(Radian(y),Radian(x),Radian(z));
        Quaternion q;
        q.FromRotationMatrix(m);

        return  q;
    }



	//-------------------------- Factory implementation
	std::string RenderServiceFactory::mName = "RenderService";

	RenderServiceFactory::RenderServiceFactory() : ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
	};

	const std::string& RenderServiceFactory::getName() {
		return mName;
	}

	Service* RenderServiceFactory::createInstance(ServiceManager* manager) {
		return new RenderService(manager, mName);
	}
}
