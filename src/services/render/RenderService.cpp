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
#include <OgreBone.h>
#include <OgreNode.h>

#include <OgreAnimation.h>

using namespace std;
using namespace Ogre;

namespace Opde {

	/*--------------------------------------------------------*/
	/*--------------------- RenderService --------------------*/
	/*--------------------------------------------------------*/
	RenderService::RenderService(ServiceManager *manager) : Service(manager) {
	    // TODO: This is just plain wrong. This service should be the maintainer of the used scene manager, if any other service needs the direct handle, etc.
	    // The fact is this service is probably game only, and should be the initialiser of graphics as the whole. This will be the
	    // modification that should be done soon in order to let the code look and be nice
	    // FIX!
	    mRoot = Root::getSingletonPtr();
	    mManualBinFileLoader = new ManualBinFileLoader();
	}

	// --------------------------------------------------------------------------
	RenderService::~RenderService() {
		if (!mPropPosition.isNull())
		    mPropPosition->unregisterListener(&mPropPositionListener);
		    
		if (!mPropModelName.isNull())
		    mPropModelName->unregisterListener(&mPropModelNameListener);
	}

	// --------------------------------------------------------------------------
	void RenderService::init() {
		// Property Service should have created us automatically through service masks.
		// So we can register as a link service listener
		LOG_INFO("RenderService::init()");

		mSceneMgr = mRoot->getSceneManager("DarkSceneManager"); // TODO: Really bad idea. Same goes to the WorldrepService

		mPropPositionListener.listener = this;
		mPropPositionListener.method = (PropertyChangeMethodPtr)(&RenderService::onPropPositionMsg);

        mPropModelNameListener.listener = this;
        mPropModelNameListener.method = (PropertyChangeMethodPtr)(&RenderService::onPropModelNameMsg);

		// Get the PropertyService, then the group Position

		// contact the config. service, and look for the inheritance link name
		// TODO: ConfigurationService::getKey("Core","InheritanceLinkName").toString();

		mPropertyService = ServiceManager::getSingleton().getService("PropertyService").as<PropertyService>();

		mPropPosition = mPropertyService->getPropertyGroup("Position");
		mPropPosition->registerListener(&mPropPositionListener);

		mPropModelName = mPropertyService->getPropertyGroup("ModelName"); // TODO: hardcoded, maybe not a problem after all
		mPropModelName->registerListener(&mPropModelNameListener);

		LOG_INFO("RenderService::init() - done");
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
                        LOG_ERROR("RenderService: Could not find the requested model %s", name.c_str());
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
		return new RenderService(manager);
	}
}
