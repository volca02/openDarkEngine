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

#include "RenderService.h"

#include "PropertyService.h"
#include "ObjectService.h"

#include "logger.h"
#include "ServiceCommon.h"

#include <OgreRoot.h>
#include <OgreStringConverter.h>
#include <OgreMeshManager.h>
#include <OgreMaterialManager.h>
#include <OgreRenderWindow.h>
#include <OgreBone.h>
#include <OgreNode.h>
#include <OgreStringConverter.h>
#include <OgreAnimation.h>
#include <OgreWindowEventUtilities.h>

using namespace std;
using namespace Ogre;

namespace Opde {

	/*--------------------------------------------------------*/
	/*--------------------- RenderService --------------------*/
	/*--------------------------------------------------------*/
	RenderService::RenderService(ServiceManager *manager, const std::string& name) : Service(manager, name), 
			mSceneMgr(NULL), 
			mRoot(NULL), 
			mDarkSMFactory(NULL), 
			mRenderWindow(NULL),
			mLoopService(NULL),
			mDefaultCamera(NULL) {
	    // TODO: This is just plain wrong. This service should be the maintainer of the used scene manager, if any other service needs the direct handle, etc.
	    // The fact is this service is probably game only, and should be the initialiser of graphics as the whole. This will be the
	    // modification that should be done soon in order to let the code look and be nice
	    // FIX!
	    mRoot = Root::getSingletonPtr();
	    mManualBinFileLoader = new ManualBinFileLoader();
	    
		mLoopClientDef.id = LOOPCLIENT_ID_RENDERER;
		mLoopClientDef.mask = LOOPMODE_RENDER;
		mLoopClientDef.name = mName;
		mLoopClientDef.priority = LOOPCLIENT_PRIORITY_RENDERER;
	}

	// --------------------------------------------------------------------------
	RenderService::~RenderService() {
        LOG_INFO("RenderService::~RenderService()");

		// Good thing about services. Render service will be released as the last 
		// one thanks to the shared_ptr. This means using the scene manager/whatever
		// is safe outside this class as long as it is done in a service which has 
		// a member pointer to render service

		if (!mPropModelName.isNull())
		    mPropModelName->unregisterListener(mPropModelNameListenerID);
		    
		if (!mPropLight.isNull())
		    mPropLight->unregisterListener(mPropLightListenerID);
		    
		if (!mPropSpotlight.isNull())
		    mPropSpotlight->unregisterListener(mPropSpotlightListenerID);

		if (mRenderWindow)
			mRenderWindow->removeAllViewports();
			
		if (mSceneMgr)
			mSceneMgr->destroyAllCameras();
			
		if (mDarkSMFactory) {
			Root::getSingleton().removeSceneManagerFactory(mDarkSMFactory);
			delete mDarkSMFactory;
			mDarkSMFactory = NULL;
		}
		
		if (!mLoopService.isNull())
			mLoopService->removeLoopClient(this);
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

		mSceneMgr = mRoot->createSceneManager(ST_INTERIOR, "DarkSceneManager");
		
		// mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
		mSceneMgr->setShadowTechnique(SHADOWTYPE_NONE);
        		
		// TODO: Set this in Database listener using Render Params chunk
		mSceneMgr->setAmbientLight(ColourValue(0.08, 0.08, 0.08));
		
		// Next step. We create a default camera in the mRenderWindow
		mDefaultCamera = mSceneMgr->createCamera( "MainCamera" );
		
		mDefaultCamera->setCastShadows(true);

		mRenderWindow->addViewport( mDefaultCamera );

		// Last step: Get the loop service and register as a listener
		mLoopService = ServiceManager::getSingleton().getService("LoopService").as<LoopService>();
		mLoopService->addLoopClient(this);

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
    Ogre::Viewport* RenderService::getDefaultViewport() {
    	assert(mDefaultCamera);
    	return mDefaultCamera->getViewport();
    }
           
    // --------------------------------------------------------------------------
    Ogre::Camera* RenderService::getDefaultCamera() {
		return mDefaultCamera;
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
	void RenderService::setWorldVisible(bool visible) {
		mSceneMgr->clearSpecialCaseRenderQueues();
		
		if (visible) {
			mSceneMgr->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_EXCLUDE);
		} else {
			mSceneMgr->addSpecialCaseRenderQueue(RENDER_QUEUE_OVERLAY);
			mSceneMgr->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_INCLUDE);
		}
	}
	
	// --------------------------------------------------------------------------
	void RenderService::bootstrapFinished() {
		// Property Service should have created us automatically through service masks.
		// So we can register as a link service listener
		LOG_INFO("RenderService::bootstrapFinished()");

		PropertyGroup::ListenerPtr cmodelc =
			new ClassCallback<PropertyChangeMsg, RenderService>(this, &RenderService::onPropModelNameMsg);
			
		PropertyGroup::ListenerPtr clightc =
			new ClassCallback<PropertyChangeMsg, RenderService>(this, &RenderService::onPropLightMsg);
		
		PropertyGroup::ListenerPtr cspotlightc =
			new ClassCallback<PropertyChangeMsg, RenderService>(this, &RenderService::onPropSpotlightMsg);


		// Get the PropertyService, then the group Position

		// contact the config. service, and look for the inheritance link name
		// TODO: ConfigurationService::getKey("Core","InheritanceLinkName").toString();

		mPropertyService = ServiceManager::getSingleton().getService("PropertyService").as<PropertyService>();

		// TODO: hardcoded property name, but that's hopefully not a problem after all
		
		// --- Model name listener
		mPropModelName = mPropertyService->getPropertyGroup("ModelName"); 

		if (mPropModelName.isNull())
            OPDE_EXCEPT("Could not get ModelName property group. Not defined. (Did you forget to load .pldef the scripts?)", "RenderService::bootstrapFinished");

		mPropModelNameListenerID = mPropModelName->registerListener(cmodelc);
		
		// --- Light property listener
		mPropLight = mPropertyService->getPropertyGroup("Light"); 

		if (mPropLight.isNull())
            OPDE_EXCEPT("Could not get Light property group. Not defined. (Did you forget to load .pldef the scripts?)", "RenderService::bootstrapFinished");

		mPropLightListenerID = mPropLight->registerListener(clightc);
		
		// --- SpotLight property listener
		mPropSpotlight = mPropertyService->getPropertyGroup("Spotlight"); 

		if (mPropSpotlight.isNull())
            OPDE_EXCEPT("Could not get Spotlight property group. Not defined. (Did you forget to load .pldef the scripts?)", "RenderService::bootstrapFinished");

		mPropSpotlightListenerID = mPropSpotlight->registerListener(cspotlightc);

		
		mObjectService = ServiceManager::getSingleton().getService("ObjectService").as<ObjectService>();

		LOG_INFO("RenderService::bootstrapFinished() - done");
	}

	// --------------------------------------------------------------------------
	void RenderService::loopStep(float deltaTime) {
		// Rendering step...
		mRoot->renderOneFrame();
		Ogre::WindowEventUtilities::messagePump();
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
			case PROP_CHANGED:
				removeObjectEntity(msg.objectID);
				

			case PROP_ADDED: {
                    try {
                        prepareMesh(name);
                        
                        // TODO: Create a MeshService. Then Entity* ent = meshService->createObjectEntity(name);
                        // That will react to DatabaseSevice notifications, and unload the meshes as needed when changing mission
                        Entity *ent = mSceneMgr->createEntity( "Object" + StringConverter::toString(msg.objectID), name + ".mesh" );

                        // bind the ent to the node of the obj.
                        SceneNode* node = NULL;
                        
                        try {
							node = mObjectService->getSceneNode(msg.objectID);
						} catch (BasicException& e) {
							LOG_ERROR("RenderService: Could not get the sceneNode for object %d! Not attaching object model!", msg.objectID);
							mSceneMgr->destroyEntity(ent);
							return;
						}
						
                        node->attachObject(ent);

                        ent->_initialise(true);
                        
                        ent->setCastShadows(true);

                        // Update the bones to be manual
                        if (ent->hasSkeleton()) {
							
							Skeleton::BoneIterator bi = ent->getSkeleton()->getBoneIterator();

							while (bi.hasMoreElements()) {
								Bone* n = bi.getNext();

								n->setManuallyControlled(true);
							}

							ent->getSkeleton()->setBindingPose();
                        }
                        
                        EntityInfo ei;
                        
                        ei.entity = ent;
                        ei.emi = new EntityMaterialInstance(ent);
                        
                        mEntityMap.insert(make_pair(msg.objectID, ei));

                    } catch (FileNotFoundException &e) {
                        LOG_ERROR("RenderService: Could not find the requested model %s (exception encountered)", name.c_str());
                    }

                    break;
                }
            case PROP_REMOVED:
                removeObjectEntity(msg.objectID);
                break;
		}
	}

	// --------------------------------------------------------------------------
	void RenderService::onPropLightMsg(const PropertyChangeMsg& msg) {
		// Management of the light
		if (msg.change == PROP_GROUP_CLEARED) {
            clear();
			return;
		}

		if (msg.objectID <= 0) // no action for archetypes
            return;

        LOG_INFO("RenderService: Adding Light for object %d", msg.objectID);

        // As a test, I'm loading cube.mesh
        switch (msg.change) {
			case PROP_CHANGED:
				// Will update the light's parameters
				// updateLight(msg.objectID, msg.data);
				updateLight(mLightInfoMap[msg.objectID], msg.data);
				break;
		
			case PROP_ADDED: {
				// let's add the light, then update it
				LightInfo& li = createLight(msg.objectID);
				updateLight(li, msg.data);
				
				// TODO: when adding the light, also check if the spotlight prop isn't present
				if (mPropSpotlight->has(msg.objectID))
					updateSpotLight(li, mPropSpotlight->getData(msg.objectID));
					
				break;
			}
				
				
			case PROP_REMOVED:
				removeLight(msg.objectID);
        }
	}

	
	// --------------------------------------------------------------------------
	void RenderService::onPropSpotlightMsg(const PropertyChangeMsg& msg) {
		if (msg.change == PROP_GROUP_CLEARED) {
            clear();
			return;
		}

		if (msg.objectID <= 0) // no action for archetypes
            return;
            
		LOG_INFO("RenderService: Adding Spotlight for object %d", msg.objectID);

		switch(msg.change) {
			case PROP_CHANGED: 
			case PROP_ADDED: {
				// have to be careful here. The propery creation order is not guaranteed
				LightInfoMap::iterator it = mLightInfoMap.find(msg.objectID);
				
				if (it != mLightInfoMap.end()) {
					updateSpotLight(it->second, msg.data);
				}
				break;
			}
				
			case PROP_REMOVED:
				removeLight(msg.objectID);
		}
	}
	
	// --------------------------------------------------------------------------
	RenderService::LightInfo& RenderService::createLight(int objID) {
		Light* l = mSceneMgr->createLight("Light" + StringConverter::toString(objID));
		
		// default to point light (spot will be indicated by a different property)
		l->setType(Light::LT_POINT);
		
		// attach the light to a newly created scenenode (child of the object's sn)
		SceneNode* on = mObjectService->getSceneNode(objID);
		
		Quaternion q;

		// TODO: Just a test. After finding the right orientation, replace with a constant quaternion value
		Matrix3 m;
		
		m.FromEulerAnglesZYX(Radian(0), Radian(Math::PI), Radian(0));
		q.FromRotationMatrix(m);

		SceneNode* n = on->createChildSceneNode(Vector3::ZERO, q);
		
		n->attachObject(l);
		
		LightInfo li;
		
		li.light = l;
		li.node = n;
		
		return (mLightInfoMap[objID] = li);
	}

	// --------------------------------------------------------------------------
	void RenderService::updateLight(LightInfo& li, const PropertyDataPtr& propLightData) {
		// The node will be a child of the Object's
		// Set the light's parameters
		// brightness, offset, radius
		Real brightness = propLightData->get("brightness").toFloat();
		Real radius = propLightData->get("radius").toFloat();

		// For lights with zero radius, use linear attenuation
		if (radius <= 0) {
			// TODO: This needs experiments to be validated
			// I wonder how the original dark handles these
			// It would be optimal to create a test scene in with D2 and R,G,B lights
			// To reveal how this one works (which is exactly what I'm gonna do)
			
			// TODO: Putting in 0 for now, because something rotten is in _traversePortalTree (recursion does not end)
			radius = 0;
			
			// make the radius sane using a limit
			/* if (radius > 20)
				radius = 20; */
				
			// linear attenuation for zero range lights
			li.light->setAttenuation(radius, 0.0, 1.0, 0.0);
		} else {
			// Hardcoded to have no attenuation over the radius
			li.light->setAttenuation(radius, 1.0, 0.0, 0.0);
		}

		// BW lights only now...
		brightness /= 100.0;
		
		li.light->setDiffuseColour(brightness, brightness, brightness);
		li.light->setSpecularColour(brightness, brightness, brightness);
		
		li.node->setPosition(propLightData->get("offset").toVector());

		static_cast<DarkSceneManager*>(mSceneMgr)->queueLightForUpdate(li.light);
	}
	
	// --------------------------------------------------------------------------
	void RenderService::removeLight(int id) {
		LightInfoMap::iterator it = mLightInfoMap.find(id);
		
		if (it != mLightInfoMap.end()) {
			// remove light, then scenenode
			it->second.node->detachAllObjects();
			
			mSceneMgr->destroyLight(it->second.light);
			mSceneMgr->destroySceneNode(it->second.node->getName());
		}
	}
		
	// --------------------------------------------------------------------------
	void RenderService::updateSpotLight(LightInfo& li, const PropertyDataPtr& propSpotData) {
		// look at the results
		// TODO: Conversion (angle in degrees or what?)
		Degree inner(propSpotData->get("inner").toFloat());
		Degree outer(propSpotData->get("outer").toFloat());
		Real dist(propSpotData->get("distance").toFloat());
		
		// The angle is in degrees!
		// The third parameter may mean distance from object... hmm.
		// I'll bet the third parameter is falloff. Let's use it as that for now
		
		li.light->setType(Light::LT_SPOTLIGHT);
		
		li.light->setSpotlightInnerAngle(inner);
		li.light->setSpotlightOuterAngle(outer);
		li.light->setSpotlightFalloff(dist);
		
		static_cast<DarkSceneManager*>(mSceneMgr)->queueLightForUpdate(li.light);
	}
	
	// --------------------------------------------------------------------------
	void RenderService::removeSpotLight(int id) {
		LightInfoMap::iterator it = mLightInfoMap.find(id);
				
		if (it != mLightInfoMap.end()) {
			it->second.light->setType(Light::LT_POINT);
		
			static_cast<DarkSceneManager*>(mSceneMgr)->queueLightForUpdate(it->second.light);
		}
	}

	
    // --------------------------------------------------------------------------
    void RenderService::prepareMesh(const Ogre::String& name) {
        String fname = name + ".mesh";

        try {
            // First, try to load the mesh directly as a mesh file
            Ogre::MeshPtr mesh1 = Ogre::MeshManager::getSingleton().load(fname, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            mesh1->prepareForShadowVolume(); 
        } catch (FileNotFoundException &e) {
            // Undefine in advance, so there will be no clash
            Ogre::MeshManager::getSingleton().remove(fname);
            // If it is not found
            LOG_DEBUG("RenderService::prepareMesh: Mesh definition for %s not found, loading manually from .bin file...", name.c_str());
            // do a create

            try {
                Ogre::MeshPtr mesh1 = Ogre::MeshManager::getSingleton().create(fname, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                   true, mManualBinFileLoader);
                   
				mesh1->prepareForShadowVolume(); 
            } catch (FileNotFoundException &e) {
                LOG_ERROR("RenderService::prepareMesh: Could not find the requested model %s", name.c_str());
            } catch (Opde::FileException &e) {
                LOG_ERROR("RenderService::prepareMesh: Could not load the requested model %s", name.c_str());
            }
        }
    }
    
    // --------------------------------------------------------------------------
    void RenderService::removeObjectEntity(int id) {
		// Not validated in prop service to be any different
		// just remove the entity, will add a new one
		ObjectEntityMap::iterator it = mEntityMap.find(id);
		
		if (it != mEntityMap.end()) {
			try {
				SceneNode* node = mObjectService->getSceneNode(id);
				
				node->detachObject(it->second.entity);
				
			} catch (BasicException& e) {
				LOG_ERROR("RenderService: Could not get the sceneNode for object %d! Not detaching entity from scene node!", id);
			}

			// destroy the emi
			delete it->second.emi;
				
			mSceneMgr->destroyEntity(it->second.entity);
			
			mEntityMap.erase(it);
		}
    }

    // --------------------------------------------------------------------------
    void RenderService::clear() {
        mEntityMap.clear();
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
