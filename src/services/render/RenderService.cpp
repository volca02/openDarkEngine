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
 *	  $Id$
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
#include <OgreException.h>

// Jorge texture png
#include "jorge.h"

// Internal rendertype constants
// Using #define to be able to Switch on them
#define RENDER_TYPE_NORMAL 0
#define RENDER_TYPE_NOT_RENDERED 1
#define RENDER_TYPE_NO_LIGHTMAP 2
#define RENDER_TYPE_EDITOR_ONLY 3

using namespace std;
using namespace Ogre;

namespace Opde {
	const char* DEFAULT_RAMP_OBJECT_NAME = "DefaultRamp";
	const char* FX_PARTICLE_OBJECT_NAME =	"FX_PARTICLE";

	

	/*--------------------------------------------------------*/
	/*--------------------- RenderService --------------------*/
	/*--------------------------------------------------------*/
	RenderService::RenderService(ServiceManager *manager, const std::string& name) : Service(manager, name), 
			mSceneMgr(NULL), 
			mRoot(NULL), 
			mRenderWindow(NULL),
			mDarkSMFactory(NULL), 
			mLoopService(NULL),
			mDefaultCamera(NULL),
			mEditorMode(false) {
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
		if (mPropPosition != NULL)
		    mPropPosition->unregisterListener(mPropPositionListenerID);

		if (mPropModelName != NULL)
		    mPropModelName->unregisterListener(mPropModelNameListenerID);
		    
		if (mPropLight != NULL)
		    mPropLight->unregisterListener(mPropLightListenerID);
		    
		if (mPropSpotlight != NULL)
		    mPropSpotlight->unregisterListener(mPropSpotlightListenerID);
		    
		if (mPropRenderType != NULL)
		    mPropRenderType->unregisterListener(mPropRenderTypeListenerID);
		    
		if (mPropScale != NULL)
		    mPropScale->unregisterListener(mPropScaleListenerID);

		if (mRenderWindow)
			mRenderWindow->removeAllViewports();
			
		if (mSceneMgr)
			mSceneMgr->destroyAllCameras();
			
		if (mDarkSMFactory) {
			Root::getSingleton().removeSceneManagerFactory(mDarkSMFactory);
			delete mDarkSMFactory;
			mDarkSMFactory = NULL;
		}
		
		delete mManualBinFileLoader;
		mManualBinFileLoader = NULL;
		
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
        LOG_DEBUG("RenderService::init(): new DarkSceneManagerFactory()");
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
		
		// prepare the default models and textures
		prepareHardcodedMedia();

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

		if (mPropModelName == NULL)
            OPDE_EXCEPT("Could not get ModelName property group. Not defined. (Did you forget to load .pldef the scripts?)", "RenderService::bootstrapFinished");

		mPropModelNameListenerID = mPropModelName->registerListener(cmodelc);
		
		// --- Light property listener
		mPropLight = mPropertyService->getPropertyGroup("Light"); 

		if (mPropLight == NULL)
            OPDE_EXCEPT("Could not get Light property group. Not defined. (Did you forget to load .pldef the scripts?)", "RenderService::bootstrapFinished");

		mPropLightListenerID = mPropLight->registerListener(clightc);
		
		// --- SpotLight property listener
		mPropSpotlight = mPropertyService->getPropertyGroup("Spotlight"); 

		if (mPropSpotlight == NULL)
            OPDE_EXCEPT("Could not get Spotlight property group. Not defined. (Did you forget to load .pldef the scripts?)", "RenderService::bootstrapFinished");

		mPropSpotlightListenerID = mPropSpotlight->registerListener(cspotlightc);

		// --- Position property listener
		mPropPosition = mPropertyService->getPropertyGroup("Position");

		if (mPropPosition == NULL)
            OPDE_EXCEPT("Could not get Position property group. Not defined. Fatal", "RenderService::bootstrapFinished");

		// listener to the position property to control the scenenode
		PropertyGroup::ListenerPtr cposc =
			new ClassCallback<PropertyChangeMsg, RenderService>(this, &RenderService::onPropPositionMsg);

		mPropPositionListenerID = mPropPosition->registerListener(cposc);
		
		// --- Scale property listener
		mPropScale = mPropertyService->getPropertyGroup("ModelScale");

		if (mPropScale == NULL)
            OPDE_EXCEPT("Could not get Scale property group. Not defined. Fatal", "RenderService::bootstrapFinished");

		PropertyGroup::ListenerPtr cscalec =
			new ClassCallback<PropertyChangeMsg, RenderService>(this, &RenderService::onPropScaleMsg);

		mPropScaleListenerID = mPropScale->registerListener(cscalec);

		// --- Render type property listener
		mPropRenderType = mPropertyService->getPropertyGroup("RenderType");

		if (mPropRenderType == NULL)
            OPDE_EXCEPT("Could not get RenderType property group. Not defined. Fatal", "RenderService::bootstrapFinished");

		PropertyGroup::ListenerPtr crtc =
			new ClassCallback<PropertyChangeMsg, RenderService>(this, &RenderService::onPropRenderTypeMsg);

		mPropRenderTypeListenerID = mPropRenderType->registerListener(crtc);
		
		// --- Render alpha property listener
		mPropRenderAlpha = mPropertyService->getPropertyGroup("RenderAlpha");

		if (mPropRenderAlpha == NULL)
            OPDE_EXCEPT("Could not get RenderAlpha property group. Not defined. Fatal", "RenderService::bootstrapFinished");

		PropertyGroup::ListenerPtr crac =
			new ClassCallback<PropertyChangeMsg, RenderService>(this, &RenderService::onPropRenderAlphaMsg);

		mPropRenderAlphaListenerID = mPropRenderAlpha->registerListener(crac);

		// TODO: The hardcoded z-bias is doing problems, unsurprisingly
		// to fix this, we should create a handler for that property

		// ===== OBJECT SERVICE LISTENER =====
		mObjectService = ServiceManager::getSingleton().getService("ObjectService").as<ObjectService>();
		
		// Listener to object messages
		ObjectService::ListenerPtr objlist =
			new ClassCallback<ObjectServiceMsg, RenderService>(this, &RenderService::onObjectMsg);
		
		mObjSystemListenerID = mObjectService->registerListener(objlist);

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

		DVariant res = "";
        mPropModelName->get(msg.objectID, "label", res); // the model name
        
        std::string name = res.toString();
        
   		LOG_VERBOSE("RenderService: A ModelName change happened : %s is new for %d", name.c_str(), msg.objectID);

        // As a test, I'm loading cube.mesh
        switch (msg.change) {
			case PROP_CHANGED:
			case PROP_ADDED: {
				std::string upper = name;
				Ogre::StringUtil::toUpperCase(upper);

				// FX_Particle is a keyword! If found, it means that the model will be invisible, and a particle effect will be done instead
				if (upper == FX_PARTICLE_OBJECT_NAME) {
					// empty param means no entity
					LOG_VERBOSE("RenderService: FX_Particle for %d", msg.objectID);
					setObjectModel(msg.objectID, "");
				} else {
					// TODO: This can consume time even when the new model name == old model name
					prepareMesh(name);
					setObjectModel(msg.objectID, name + ".mesh");
				}
				
				break;
			}
                    
            case PROP_REMOVED:
				setObjectModel(msg.objectID, DEFAULT_RAMP_OBJECT_NAME);
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
				updateLight(mLightInfoMap[msg.objectID], msg.objectID);
				break;
		
			case PROP_ADDED: {
				// let's add the light, then update it
				LightInfo& li = createLight(msg.objectID);
				updateLight(li, msg.objectID);
				
				// TODO: when adding the light, also check if the spotlight prop isn't present
				if (mPropSpotlight->has(msg.objectID))
					updateSpotLight(li, msg.objectID);
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
					updateSpotLight(it->second, msg.objectID);
				}
				break;
			}
				
			case PROP_REMOVED:
				removeLight(msg.objectID);
		}
	}
	
	// --------------------------------------------------------------------------
	void RenderService::onPropScaleMsg(const PropertyChangeMsg& msg) {
		if (msg.objectID <= 0) // no action for archetypes
            return;
            
		LOG_VERBOSE("RenderService: Scale prop for obj. %d changed", msg.objectID);

		switch(msg.change) {
			case PROP_CHANGED: 
			case PROP_ADDED: {
				Ogre::SceneNode* node = getSceneNode(msg.objectID);
					
				if (node == NULL)
					return;
					
				DVariant scale; 

				mPropScale->get(msg.objectID, "scale", scale);
				
				Vector3 vscale = scale.toVector(); 
				
				LOG_VERBOSE("RenderService: New object %d scale : %f, %f, %f", msg.objectID, vscale.x, vscale.y, vscale.z);
				
				node->setScale(vscale);
				break;
			}
				
			case PROP_REMOVED:
				// TODO: reset to scale 1, 1, 1?
				break;
		}
	}
	
	// --------------------------------------------------------------------------
	void RenderService::onPropRenderTypeMsg(const PropertyChangeMsg& msg) {
		if (msg.objectID <= 0) // no action for archetypes
            return;
        
        
		LOG_VERBOSE("RenderService: non-archetype RenderType prop for obj. %d changed [%d]", msg.objectID, (int)(msg.change));
		
        // Rendertype property only affects the model (entity) not the lightning properties.    
		
		switch(msg.change) {
			case PROP_CHANGED: 
			case PROP_ADDED: {
				// It seems this property !only! affects models
				ObjectEntityMap::iterator it = mEntityMap.find(msg.objectID);
			
				if (it != mEntityMap.end()) {
					SceneNode* node = it->second.node;
				
					assert(node != NULL);
				
					DVariant mode; 
					mPropRenderType->get(msg.objectID, "mode", mode);
					
					LOG_VERBOSE("RenderService: RenderType prop for obj. %d changed to %d", msg.objectID, mode.toUInt());
					
					it->second.renderType = mode.toUInt();
					
					setNodeRenderType(node, it->second);
				} else {
					LOG_ERROR("RenderService: Could not find an entity info for object %d, could not propagate RenderType", msg.objectID);
				}
				
				
				return;
			}
				
			case PROP_REMOVED:
				// Default is visible (?)
				ObjectEntityMap::iterator it = mEntityMap.find(msg.objectID);
			
				if (it != mEntityMap.end()) {
					it->second.node->setVisible(true);
				}
				break;
		}
	}
	
	// --------------------------------------------------------------------------
	void RenderService::onPropRenderAlphaMsg(const PropertyChangeMsg& msg) {
		if (msg.objectID <= 0) // no action for archetypes
            return;
            
		ObjectEntityMap::iterator it = mEntityMap.find(msg.objectID);
				
		if (it == mEntityMap.end())
			return;
            
		switch(msg.change) {
			case PROP_CHANGED: 
			case PROP_ADDED: {
				DVariant alpha; 
				mPropRenderAlpha->get(msg.objectID, "alpha", alpha);
				
				LOG_DEBUG("RenderService: RenderAlpha prop for obj. %d changed to %f", msg.objectID, alpha.toFloat());
				
				it->second.emi->setTransparency(1.0f - alpha.toFloat());
				return;
			} 
			case PROP_REMOVED:
				it->second.emi->setTransparency(0);
				return;
		}
	}
	
	// --------------------------------------------------------------------------
	RenderService::LightInfo& RenderService::createLight(int objID) {
		Light* l = mSceneMgr->createLight("Light" + StringConverter::toString(objID));
		
		// default to point light (spot will be indicated by a different property)
		l->setType(Light::LT_POINT);
		
		// attach the light to a newly created scenenode (child of the object's sn)
		SceneNode* on = getSceneNode(objID);
		
		Quaternion q;

		// TODO: Just a test. After finding the right orientation, replace with a constant quaternion value
		Matrix3 m;
		
		m.FromEulerAnglesZYX(Radian(0), Radian(Math::PI), Radian(0));
		q.FromRotationMatrix(m);

		SceneNode* n = on->createChildSceneNode(Vector3::ZERO, q);
		
		on->attachObject(l);
		
		LightInfo li;
		
		li.light = l;
		li.node = n;
		
		return (mLightInfoMap[objID] = li);
	}

	// --------------------------------------------------------------------------
	void RenderService::updateLight(LightInfo& li, int objectID) {
		// The node will be a child of the Object's
		// Set the light's parameters
		// brightness, offset, radius
		DVariant dbrightness, dradius;
		
		if (!mPropLight->get(objectID, "brightness", dbrightness)) {
			LOG_FATAL("RenderService: Could not get brightness field of light property!");
			return;
		}
		
		if (!mPropLight->get(objectID, "radius", dradius)) {
			LOG_FATAL("RenderService: Could not get radius field of light property!");
			return;
		}

		Real brightness, radius;
		
		brightness = dbrightness.toFloat();
		radius = dradius.toFloat();
	
		// For lights with zero radius, use linear attenuation
		if (radius <= 0) {
			// TODO: This needs experiments to be validated
			// I wonder how the original dark handles these
			// It would be optimal to create a test scene in with D2 and R,G,B lights
			// To reveal how this one works (which is exactly what I'm gonna do)
			radius = brightness; 
			
			// make the radius sane using a limit
			if (radius > 100)
				radius = 100;
				
			// linear attenuation for zero range lights
			li.light->setAttenuation(radius, 0.0, 1.0, 0.0);
		} else {
			// Hardcoded to have no attenuation over the radius
			li.light->setAttenuation(radius, 1.0, 0.0, 0.0);
		}

		// TODO: BW lights only now...
		// brightness /= 100.0;
		
		li.light->setDiffuseColour(brightness, brightness, brightness);
		li.light->setSpecularColour(brightness, brightness, brightness);
		
		DVariant offs;
		
		mPropLight->get(objectID, "offset", offs);
		
		li.node->setPosition(offs.toVector());

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
	void RenderService::updateSpotLight(LightInfo& li, int objectID) {
		// look at the results
		// TODO: Conversion (angle in degrees or what?)
		DVariant res;
		
		mPropSpotlight->get(objectID, "inner", res);
		Degree inner(res.toFloat());
		
		res = DVariant(); // so that toFloat will be errorneous if field name mismatches
		mPropSpotlight->get(objectID, "outer", res);
		Degree outer(res.toFloat());
		
		res = DVariant();
		mPropSpotlight->get(objectID, "distance", res);
		Real dist(res.toFloat());
		
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
	void RenderService::setNodeRenderType(Ogre::SceneNode* node, const EntityInfo& ei) {
		// skip flag or hasRefs false will mean no rendering regardless of the renderType value
		if (ei.skip || !ei.hasRefs) {
			node->setVisible(false, true);
		} else {
			switch(ei.renderType) {
				case RENDER_TYPE_NORMAL: node->setVisible(true, true); break;
				case RENDER_TYPE_NOT_RENDERED: node->setVisible(false, true); break;
				case RENDER_TYPE_NO_LIGHTMAP: node->setVisible(true, true); break;// Not sure. Seems to indicate shadow should not be cast on lmaps?
				case RENDER_TYPE_EDITOR_ONLY: node->setVisible(mEditorMode, true); break;
			}
		}
	}

    // --------------------------------------------------------------------------
    void RenderService::prepareMesh(const Ogre::String& name) {
        String fname = name + ".mesh";

        try {
            // First, try to load the mesh directly as a mesh file
            Ogre::MeshPtr mesh1 = Ogre::MeshManager::getSingleton().load(fname, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

        } catch (FileNotFoundException) {
            // Undefine in advance, so there will be no clash
            Ogre::MeshManager::getSingleton().remove(fname);
            // If it is not found
            LOG_DEBUG("RenderService::prepareMesh: Mesh definition for %s not found, loading manually from .bin file...", name.c_str());
            // do a create

            try {
                Ogre::MeshPtr mesh1 = Ogre::MeshManager::getSingleton().create(fname, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                   true, mManualBinFileLoader);
            } catch (FileNotFoundException) {
                LOG_ERROR("RenderService::prepareMesh: Could not find the requested model %s", name.c_str());
            } catch (Opde::FileException) {
                LOG_ERROR("RenderService::prepareMesh: Could not load the requested model %s", name.c_str());
            }
        }
    }
    
    // --------------------------------------------------------------------------
    void RenderService::createObjectModel(int id) {
    	Ogre::String idstr = StringConverter::toString(id);
			
		Entity *ent = mSceneMgr->createEntity( "Object" + idstr, DEFAULT_RAMP_OBJECT_NAME);

		// bind the ent to the node of the obj.
		SceneNode* node = NULL;
		
		try {
			node = getSceneNode(id);
		} catch (BasicException) {
			LOG_ERROR("RenderService: Could not get the sceneNode for object %d! Not attaching object model!", id);
			mSceneMgr->destroyEntity(ent);
			return;
		}
		
		assert(node != NULL);
		
		SceneNode* enode = mSceneMgr->createSceneNode("ObjMesh" + idstr);
		enode->setPosition(Vector3::ZERO);
		enode->setOrientation(Quaternion::IDENTITY);
		node->addChild(enode);
		
		enode->attachObject(ent);

		prepareEntity(ent);
		
		EntityInfo ei;
		
		ei.entity = ent;
		ei.node = enode;
		ei.emi = new EntityMaterialInstance(ent);
		
		ei.renderType = RENDER_TYPE_NORMAL; // seems to be the default. Affects Models, Particle effects (DynamicLights?)
		ei.hasRefs = true;
		ei.skip = false;
		setNodeRenderType(node, ei);
		
		ei.emi->setSceneBlending(SBT_TRANSPARENT_ALPHA);
		// ei.emi->setSceneBlending(SBT_MODULATE);

		
		mEntityMap.insert(make_pair(id, ei));
    }
    
    // --------------------------------------------------------------------------
    void RenderService::removeObjectModel(int id) {
		// Not validated in prop service to be any different
		// just remove the entity, will add a new one
		ObjectEntityMap::iterator it = mEntityMap.find(id);
		
		if (it != mEntityMap.end()) {
			LOG_VERBOSE("RenderService: Destroying the entity for %d", id);
			
			SceneNode* node = it->second.node;
			// SceneNode* node = getSceneNode(id);
				
			assert(node != NULL);
			
			node->detachObject(it->second.entity);

			// destroy the emi
			delete it->second.emi;
				

			// destroy the detached entity
			mSceneMgr->destroyEntity(it->second.entity);
			
			// destroy the scenenode of the entity
			mSceneMgr->destroySceneNode(node->getName());
			
			// erase the record itself
			mEntityMap.erase(it);
		}
    }

	// --------------------------------------------------------------------------
	void RenderService::setObjectModel(int id, const std::string& name) {
		ObjectEntityMap::iterator it = mEntityMap.find(id);
			
		if (it != mEntityMap.end()) {
			Ogre::String idstr = StringConverter::toString(id);
			
			// destroy the prev. entity
			SceneNode* node = it->second.node;
				
			assert(node != NULL);
			
			node->detachObject(it->second.entity);
			
			if (name == "") { // emty model name means no render
				// leave in the previous entity, set hidden flag
				it->second.skip = true;
				setNodeRenderType(node, it->second);
				LOG_VERBOSE("RenderService: Mesh rendering for %d disabled", id);
			} else {
				Entity *ent;
				// load the new entity, set it to emi
				try {
					ent = mSceneMgr->createEntity( "Object" + idstr + name, name);
				} catch (Ogre::Exception& e) {
					// TODO: This could also be handled by not setting the new entity at all
					LOG_ERROR("RenderService: Could not load model %s for obj %d : %s", name.c_str(), id, e.getFullDescription().c_str());
					ent = mSceneMgr->createEntity( "Object" + idstr + "Ramp", DEFAULT_RAMP_OBJECT_NAME);
				}
				
				it->second.skip = false;
				Entity *prevent = it->second.entity;
				prepareEntity(ent);
				it->second.entity = ent;
				it->second.emi->setEntity(ent);
				
				node->attachObject(ent);
				
				// set the visibility again, node does not propagate to new members!
				setNodeRenderType(node, it->second);
				
				// last, destroy the detached entity
				mSceneMgr->destroyEntity(prevent);
			}
		} else {
			LOG_FATAL("RenderService: Entity info not found for object %d. Cannot set model!", id);
		}
	}

    // --------------------------------------------------------------------------
	void RenderService::prepareEntity(Ogre::Entity* e) {
		e->_initialise(true);
		
		e->setCastShadows(true);

		// Update the bones to be manual
		// TODO: This should only apply for non-ai meshes!
		if (e->hasSkeleton()) {
			
			Skeleton::BoneIterator bi = e->getSkeleton()->getBoneIterator();

			while (bi.hasMoreElements()) {
				Bone* n = bi.getNext();

				n->setManuallyControlled(true);
			}

			e->getSkeleton()->setBindingPose();
		}
	}

    // --------------------------------------------------------------------------
    void RenderService::clear() {
        mEntityMap.clear();
    }


	// --------------------------------------------------------------------------
	void RenderService::attachCameraToObject(int objID) {
		SceneNode* sn = getSceneNode(objID);
		
		if (sn) {
			sn->attachObject(mDefaultCamera);
		}
	}
	
	// --------------------------------------------------------------------------
	void RenderService::detachCamera() {
		if (mDefaultCamera->isAttached()) {
			SceneNode* sn = mDefaultCamera->getParentSceneNode();
			sn->detachObject(mDefaultCamera);
		}
	}

	// --------------------------------------------------------------------------
	// ---- Scene Node handling routines ----------------------------------------
	// --------------------------------------------------------------------------
	Ogre::SceneNode* RenderService::getSceneNode(int objID) {
		if (objID > 0) {
			ObjectToNode::iterator snit = mObjectToNode.find(objID);
			
			if (snit != mObjectToNode.end()) {
				return snit->second;
			}
		}
		
		// Throwing exceptions is not a good idea
		return NULL;
		// OPDE_EXCEPT("Could not find scenenode for object. Does object exist?", "ObjectService::getSceneNodeForObject");
	}
	
	// --------------------------------------------------------------------------
	void RenderService::onPropPositionMsg(const PropertyChangeMsg& msg) {
		// Update the scene node's position and orientation
		if (msg.objectID <= 0) // no action for archetypes
            return;

		switch (msg.change) {
			case PROP_ADDED   :
			case PROP_CHANGED : {
				try {
					// Find the scene node by it's object id, and update the position and orientation
					Ogre::SceneNode* node = getSceneNode(msg.objectID);
					
					if (node == NULL)
						return;

					DVariant pos; 
					mPropPosition->get(msg.objectID, "position", pos);
					DVariant ori;
					mPropPosition->get(msg.objectID, "facing", ori);
					
					node->setPosition(pos.toVector());
					node->setOrientation(ori.toQuaternion());
				
				} catch (BasicException& e) {
					LOG_ERROR("RenderService: Exception while setting position of object: %s", e.getDetails().c_str());
				}
				
				break;
			}
		}
	}
	
	// --------------------------------------------------------------------------
	void RenderService::onObjectMsg(const ObjectServiceMsg& msg) {
		if (msg.objectID <= 0) // no action for archetypes
            return;

		switch (msg.type) {
			case OBJ_CREATE_STARTED   : {
				// create scenenode, return
				std::string nodeName("Object");
			
				nodeName += Ogre::StringConverter::toString(msg.objectID);
				
				// Use render service to create a scene node for the object
				Ogre::SceneNode* snode = mSceneMgr->createSceneNode(nodeName);
				
				assert(snode != NULL);
				
				// Attach the node to the root SN of the scene
				mSceneMgr->getRootSceneNode()->addChild(snode);
				
				mObjectToNode.insert(std::make_pair(msg.objectID, snode));
				
				// set the default model - default ramp
				createObjectModel(msg.objectID);
				
				return;
			}
			
			case OBJ_DESTROYED : {
				// destroy all the entities/lights attached, then the scenenode
				// Light and spotlight are removed because of property removal, 
				// TODO: no modelName -> default model (mesh) for the object
				// in original dark
				// if not present, so after we construct the same behavior, 
				// destroy the entity here
				
				removeObjectModel(msg.objectID);
				
				// find the SN, destroy if found
				ObjectToNode::iterator oit = mObjectToNode.find(msg.objectID);
				
				if (oit!=mObjectToNode.end()) {
					std::string nodeName("Object");
					nodeName += Ogre::StringConverter::toString(msg.objectID);
					mSceneMgr->destroySceneNode(oit->second->getName());
				}
			
				return;
			}
		}
	}

	void RenderService::prepareHardcodedMedia() {
		// 1. The default texture - Jorge (recreated to be nearly the same visually)
		TexturePtr jorgeTex = TextureManager::getSingleton().getByName("jorge.png");
			
		if (jorgeTex.isNull()) {
			DataStreamPtr stream(new MemoryDataStream(JORGE_TEXTURE_PNG, JORGE_TEXTURE_PNG_SIZE, false));
			
			Image img;
			img.load(stream, "png");
			jorgeTex = TextureManager::getSingleton().loadImage(
							"jorge.png", ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
							img, TEX_TYPE_2D);
		}
		
		// 2. The default geometric shape - White Ramp
		/* I don't have the exact sizes of this thing. */
		createRampMesh();
	}
	
	void RenderService::createRampMesh() {
		// Code copied from Ogre3D wiki, and modified (Thanks to the original author for saving my time!)
		/// Create the mesh via the MeshManager
		Ogre::MeshPtr msh = MeshManager::getSingleton().createManual(DEFAULT_RAMP_OBJECT_NAME, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		/// Create one submesh
		SubMesh* sub = msh->createSubMesh("BaseWhite");

		// Not exactly accurate for 1:1:2 side size ratio, but well...
		const float sqrt13 = 0.577350269f; /* sqrt(1/3) */

		const size_t nVertices = 6;
		const size_t vbufCount = 3*2*nVertices;
		
		float vertices[vbufCount] = {
			-2.0,-1.0,-1.0,	//0 position
			-sqrt13,-sqrt13,-sqrt13,
			-2.0, 1.0,-1.0, //1 position
			-sqrt13,sqrt13,-sqrt13,
			-2.0, 1.0, 1.0, //2 position
			-sqrt13,sqrt13,sqrt13,
			-2.0,-1.0, 1.0, //3 position
			-sqrt13,-sqrt13,sqrt13,
			2.0, 1.0, -0.3,  //4 position
			sqrt13,sqrt13,-sqrt13,
			2.0,-1.0, -0.3,  //5 position
			sqrt13,-sqrt13,-sqrt13
		};
		
		const size_t ibufCount = 24;
		unsigned short faces[ibufCount] = {
				0,1,2,
				0,2,3,
				5,1,0,
				5,4,1,
				5,3,2,
				5,2,4,
				1,4,2,
				5,0,3
		};
			

		/// Create vertex data structure for 8 vertices shared between submeshes
		msh->sharedVertexData = new VertexData();
		msh->sharedVertexData->vertexCount = nVertices;

		/// Create declaration (memory format) of vertex data
		VertexDeclaration* decl = msh->sharedVertexData->vertexDeclaration;
		size_t offset = 0;
		// 1st buffer
		decl->addElement(0, offset, VET_FLOAT3, VES_POSITION);
		offset += VertexElement::getTypeSize(VET_FLOAT3);
		decl->addElement(0, offset, VET_FLOAT3, VES_NORMAL);
		offset += VertexElement::getTypeSize(VET_FLOAT3);
		
		/// Allocate vertex buffer of the requested number of vertices (vertexCount) 
		/// and bytes per vertex (offset)
		HardwareVertexBufferSharedPtr vbuf = 
			HardwareBufferManager::getSingleton().createVertexBuffer(
			offset, msh->sharedVertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
		
		/// Upload the vertex data to the card
		vbuf->writeData(0, vbuf->getSizeInBytes(), vertices, true);

		/// Set vertex buffer binding so buffer 0 is bound to our vertex buffer
		VertexBufferBinding* bind = msh->sharedVertexData->vertexBufferBinding; 
		bind->setBinding(0, vbuf);

		/// Allocate index buffer of the requested number of vertices (ibufCount) 
		HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().
			createIndexBuffer(
			HardwareIndexBuffer::IT_16BIT, 
			ibufCount, 
			HardwareBuffer::HBU_STATIC_WRITE_ONLY);

		/// Upload the index data to the card
		ibuf->writeData(0, ibuf->getSizeInBytes(), faces, true);

		/// Set parameters of the submesh
		sub->useSharedVertices = true;
		sub->indexData->indexBuffer = ibuf;
		sub->indexData->indexCount = ibufCount;
		sub->indexData->indexStart = 0;

		/// Set bounding information (for culling)
		msh->_setBounds(AxisAlignedBox(-2,-1,-1,2,1,1));
		msh->_setBoundingSphereRadius(Math::Sqrt(6));

		/// Notify Mesh object that it has been loaded
		msh->load();
		
		prepareMesh(DEFAULT_RAMP_OBJECT_NAME);
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
