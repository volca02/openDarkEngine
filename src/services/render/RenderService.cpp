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
#include <OgreViewport.h>

// Jorge texture png
#include "jorge.h"

#include "HasRefsProperty.h"
#include "RenderTypeProperty.h"
#include "RenderAlphaProperty.h"
#include "ZBiasProperty.h"
#include "ModelScaleProperty.h"
#include "ModelNameProperty.h"

using namespace std;
using namespace Ogre;

namespace Opde {
	const char* DEFAULT_RAMP_OBJECT_NAME = "DefaultRamp";
	const char* FX_PARTICLE_OBJECT_NAME =	"FX_PARTICLE";

	/*--------------------------------------------------------*/
	/*--------------------- EntityInfo -----------------------*/
	/*--------------------------------------------------------*/
	EntityInfo::EntityInfo(Ogre::SceneManager* man, Ogre::Entity* entity, Ogre::SceneNode* node) :
		mSceneMgr(man),
		mRenderType(RENDER_TYPE_NORMAL),
		mHasRefs(true),
		mSkip(false),
		mAlpha(1.0f),
		mZBias(0.0f),
		mEntity(entity),
		mNode(node),
		mEmi(NULL) {

		mEmi = new EntityMaterialInstance(mEntity);
		mEmi->setSceneBlending(SBT_TRANSPARENT_ALPHA);
		// mEmi->setSceneBlending(SBT_MODULATE);
	};

	// --------------------------------------------------------------------------
	EntityInfo::~EntityInfo() {
		//
		mNode->detachObject(mEntity);

		delete mEmi;

		mSceneMgr->destroyEntity(mEntity);

		mSceneMgr->destroySceneNode(mNode->getName());
	}

	// --------------------------------------------------------------------------
	void EntityInfo::setHasRefs(bool _hasRefs) {
		mHasRefs = _hasRefs;
		refreshVisibility();
	};

	// --------------------------------------------------------------------------
	void EntityInfo::setRenderType(unsigned int _renderType) {
		mRenderType = _renderType;
		refreshVisibility();
	};

	// --------------------------------------------------------------------------
	void EntityInfo::setSkip(bool _skip) {
		mSkip = _skip;
		refreshVisibility();
	};

	// --------------------------------------------------------------------------
	void EntityInfo::setAlpha(float alpha) {
		mAlpha = alpha;
		mEmi->setTransparency(1.0f - mAlpha);
	};

	// --------------------------------------------------------------------------
	void EntityInfo::setZBias(float bias) {
		mZBias = bias;
		mEmi->setZBias(bias);
	};

	// --------------------------------------------------------------------------
	void EntityInfo::setScale(const Vector3& scale) {
		mNode->setScale(scale);
	};

	// --------------------------------------------------------------------------
	void EntityInfo::setEntity(Ogre::Entity* entity) {
		if (mEntity == entity)
			return;

		// detach the old entity
		mNode->detachObject(mEntity);

		// attach the new entity
		mNode->attachObject(entity);

		mEmi->setEntity(entity);

		// destroy the previous entity
		mSceneMgr->destroyEntity(mEntity);

		mEntity = entity;

		refreshVisibility();
	};

	// --------------------------------------------------------------------------
	void EntityInfo::refreshVisibility() {
		// calculate the visibilities:
		bool brType = true;

		if (mRenderType == RENDER_TYPE_NOT_RENDERED)
			brType = false;

		// todo: editor mode handling

		mNode->setVisible(!mSkip && mHasRefs && brType, true);
	};

	/*--------------------------------------------------------*/
	/*--------------------- RenderService --------------------*/
	/*--------------------------------------------------------*/
	RenderService::RenderService(ServiceManager *manager, const std::string& name) : Service(manager, name),
			mPropModelName(NULL),
			mPropPosition(NULL),
			mPropScale(NULL),
			mRoot(NULL),
			mSceneMgr(NULL),
			mRenderWindow(NULL),
			mDarkSMFactory(NULL),
			mDefaultCamera(NULL),
			mLoopService(NULL),
			mEditorMode(false),
			mHasRefsProperty(NULL),
			mRenderTypeProperty(NULL),
			mRenderAlphaProperty(NULL),
			mZBiasProperty(NULL) {
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
	}

	// --------------------------------------------------------------------------
	void RenderService::shutdown() {
		LOG_INFO("RenderService::shutdown()");

        clear();

		if (mHasRefsProperty) {
			mPropertyService->unregisterPropertyGroup(mHasRefsProperty);
			delete mHasRefsProperty;
			mHasRefsProperty = NULL;
		}

		if (mRenderTypeProperty) {
			mPropertyService->unregisterPropertyGroup(mRenderTypeProperty);
			delete mRenderTypeProperty;
			mRenderTypeProperty = NULL;
		}

		if (mRenderAlphaProperty) {
			mPropertyService->unregisterPropertyGroup(mRenderAlphaProperty);
			delete mRenderAlphaProperty;
			mRenderAlphaProperty = NULL;
		}

		if (mZBiasProperty) {
			mPropertyService->unregisterPropertyGroup(mZBiasProperty);
			delete mZBiasProperty;
			mZBiasProperty = NULL;
		}

		if (mPropPosition != NULL)
		    mPropPosition->unregisterListener(mPropPositionListenerID);
		mPropPosition = NULL;

		if (mPropModelName != NULL) {
		    mPropertyService->unregisterPropertyGroup(mPropModelName);
			delete mPropModelName;
			mPropModelName = NULL;
		}

		if (mPropScale != NULL) {
		    mPropertyService->unregisterPropertyGroup(mPropScale);
			delete mPropScale;
			mPropScale = NULL;
		}


		if (!mLoopService.isNull()) {
			mLoopService->removeLoopClient(this);
			mLoopService = NULL;
		}

		mConfigService = NULL;
		mPropertyService = NULL;
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

		// aspect derived from screen aspect (to avoid stretching)
		mDefaultCamera->setAspectRatio(Real(mRenderWindow->getWidth()) / Real(mRenderWindow->getHeight()));

		mRenderWindow->addViewport( mDefaultCamera );

		// Last step: Get the loop service and register as a listener
		mLoopService = GET_SERVICE(LoopService);
		mLoopService->addLoopClient(this);

		// prepare the default models and textures
		prepareHardcodedMedia();

		mPropertyService = GET_SERVICE(PropertyService);

		mConfigService = GET_SERVICE(ConfigService);

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

		// create the properties the render service uses (built-in)
		createProperties();

		// Get the PropertyService, then the group Position
		// --- Position property listener
		mPropPosition = mPropertyService->getPropertyGroup("Position");

		if (mPropPosition == NULL)
            OPDE_EXCEPT("Could not get Position property group. Not defined. Fatal", "RenderService::bootstrapFinished");

		// listener to the position property to control the scenenode
		PropertyGroup::ListenerPtr cposc =
			new ClassCallback<PropertyChangeMsg, RenderService>(this, &RenderService::onPropPositionMsg);

		mPropPositionListenerID = mPropPosition->registerListener(cposc);

		// ===== OBJECT SERVICE LISTENER =====
		mObjectService = GET_SERVICE(ObjectService);

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
    void RenderService::prepareMesh(const Ogre::String& name) {
        String fname = name + ".mesh";

		if (!Ogre::MeshManager::getSingleton().resourceExists(fname)) {
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

		EntityInfoPtr ei = new EntityInfo(mSceneMgr, ent, enode);
		ei->refreshVisibility();

		mEntityMap.insert(make_pair(id, ei));
    }

    // --------------------------------------------------------------------------
    void RenderService::removeObjectModel(int id) {
		// Not validated in prop service to be any different
		// just remove the entity, will add a new one
		ObjectEntityMap::iterator it = mEntityMap.find(id);

		if (it != mEntityMap.end()) {
			LOG_VERBOSE("RenderService: Destroying the entity for %d", id);

			// erase the record itself - will destroy the EntityInfo
			mEntityMap.erase(it);
		}
    }

	// --------------------------------------------------------------------------
	void RenderService::setObjectModel(int id, const std::string& name) {
		EntityInfo* ei = _getEntityInfo(id);

		LOG_VERBOSE("RenderService::setObjectModel: Object model for %d - setting to %s", id, name.c_str());

		prepareMesh(name);

		// if there was an error finding the entity info, the object does not exist
		if (!ei) {
			LOG_VERBOSE("RenderService: Object model could not be set for %d, object not found", id);
			return;
		}

		// upper, to compare without case
		std::string iname = name;
		StringUtil::toUpperCase(iname);

		// if the new name is particle, just set skip and it's done
		if (iname == FX_PARTICLE_OBJECT_NAME) {
			LOG_VERBOSE("RenderService: Mesh rendering for %d disabled", id);
			ei->setSkip(true);
			return;
		}

		// name not empty. Prepare new entity, swap, destroy old

		Ogre::String idstr = StringConverter::toString(id);

		Entity *ent = NULL;

		// load the new entity, set it as the current in the entity info
		try {
			ent = mSceneMgr->createEntity( "Object" + idstr + name, name + ".mesh");
		} catch (Ogre::Exception& e) {
			// TODO: This could also be handled by not setting the new entity at all
			LOG_ERROR("RenderService: Could not load model %s for obj %d : %s", name.c_str(), id, e.getFullDescription().c_str());
			ent = mSceneMgr->createEntity( "Object" + idstr + "Ramp", DEFAULT_RAMP_OBJECT_NAME);
		}

		prepareEntity(ent);

		// will destroy the previous
		ei->setEntity(ent);

		// Last step - refresh the skip
		ei->setSkip(false);
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
		// will destroy all EntityInfos
        mEntityMap.clear();

		mObjectToNode.clear();
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
	EntityInfo* RenderService::_getEntityInfo(int oid) {
		ObjectEntityMap::iterator it = mEntityMap.find(oid);

		if (it != mEntityMap.end()) {
			return (it->second.ptr());
		}

		return NULL;
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
			
			default: break;
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
			
			default: break; // nothing, ignore
		}
	}

	// --------------------------------------------------------------------------
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
			sqrt13,sqrt13,sqrt13,
			-2.0, 1.0,-1.0, //1 position
			sqrt13,-sqrt13,sqrt13,
			-2.0, 1.0, 1.0, //2 position
			sqrt13,-sqrt13,-sqrt13,
			-2.0,-1.0, 1.0, //3 position
			sqrt13,sqrt13,-sqrt13,
			2.0, 1.0, -0.3,  //4 position
			-sqrt13,-sqrt13,sqrt13,
			2.0,-1.0, -0.3,  //5 position
			-sqrt13,sqrt13,sqrt13
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

	// --------------------------------------------------------------------------
	void RenderService::createProperties() {
		// Ok, what do we have here?
		// Model Name. Simple fixed-length string prop
		// Fixed on version 2.16
		mPropModelName = new ModelNameProperty(this, mPropertyService.ptr());
		mPropertyService->registerPropertyGroup(mPropModelName);

		// RenderAlpha property - single float prop
		mRenderAlphaProperty = new RenderAlphaProperty(this, mPropertyService.ptr());
		mPropertyService->registerPropertyGroup(mRenderAlphaProperty);

		// HasRefs - single bool prop
		mHasRefsProperty = new HasRefsProperty(this, mPropertyService.ptr());
		mPropertyService->registerPropertyGroup(mHasRefsProperty);

		// RenderType property - single unsigned int property with enum
		mRenderTypeProperty = new RenderTypeProperty(this, mPropertyService.ptr());
		mPropertyService->registerPropertyGroup(mRenderTypeProperty);

		// ZBias - z bias for depth fighting avoidance
		if (mConfigService->getGameType() > ConfigService::GAME_TYPE_T1) { // only t1 does not have ZBIAS
			mZBiasProperty = new ZBiasProperty(this, mPropertyService.ptr());
			mPropertyService->registerPropertyGroup(mZBiasProperty);
		}

		// Scale property
		mPropScale = new ModelScaleProperty(this, mPropertyService.ptr());
		mPropertyService->registerPropertyGroup(mPropScale);
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

