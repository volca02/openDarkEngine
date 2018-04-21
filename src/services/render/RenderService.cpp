/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
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


#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "RenderService.h"

#include "DarkSceneManager.h"

#include "OpdeServiceManager.h"

#include "config/ConfigService.h"
#include "object/ObjectService.h"
#include "property/PropertyService.h"

#include "ServiceCommon.h"
#include "logger.h"

#include <OgreAnimation.h>
#include <OgreBone.h>
#include <OgreEntity.h>
#include <OgreException.h>
#include <OgreLight.h>
#include <OgreMaterialManager.h>
#include <OgreMeshManager.h>
#include <OgreNode.h>
#include <OgreRenderWindow.h>
#include <OgreRoot.h>
#include <OgreSceneNode.h>
#include <OgreStringConverter.h>
#include <OgreViewport.h>
#include <OgreWindowEventUtilities.h>

// Jorge texture png
#include "jorge.h"

#include "loop/LoopService.h"
#include "object/ObjectService.h"
#include "property/PropertyService.h"
#include "OpdeServiceManager.h"
#include "ManualBinFileLoader.h"
#include "EntityInfo.h"

#include "HasRefsProperty.h"
#include "ModelNameProperty.h"
#include "ModelScaleProperty.h"
#include "RenderAlphaProperty.h"
#include "RenderTypeProperty.h"
#include "ZBiasProperty.h"

using namespace std;
using namespace Ogre;

namespace Opde {
const char *DEFAULT_RAMP_OBJECT_NAME = "DefaultRamp";
const char *FX_PARTICLE_OBJECT_NAME = "FX_PARTICLE";

/*--------------------------------------------------------*/
/*--------------------- RenderService --------------------*/
/*--------------------------------------------------------*/
template <> const size_t ServiceImpl<RenderService>::SID = __SERVICE_ID_RENDER;

RenderService::RenderService(ServiceManager *manager, const std::string &name)
    : ServiceImpl<Opde::RenderService>(manager, name),
      mPropModelName(),
      mPropPosition(NULL),
      mPropScale(),
      mRoot(NULL),
      mSceneMgr(NULL),
      mRenderWindow(NULL),
      mDarkSMFactory(NULL),
      mDefaultCamera(NULL),
      mLoopService(NULL),
      mEditorMode(false),
      mHasRefsProperty(),
      mRenderTypeProperty(),
      mRenderAlphaProperty(),
      mZBiasProperty(),
      mCurrentSize(0, 0),
      mSDLWindow(NULL)
{
    // TODO: This is just plain wrong. This service should be the maintainer of
    // the used scene manager, if any other service needs the direct handle,
    // etc. The fact is this service is probably game only, and should be the
    // initialiser of graphics as the whole. This will be the modification that
    // should be done soon in order to let the code look and be nice FIX!
    mRoot = Root::getSingletonPtr();
    mManualBinFileLoader.reset(new ManualBinFileLoader());

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
}

// --------------------------------------------------------------------------
void RenderService::shutdown() {
    LOG_INFO("RenderService::shutdown()");

    clear();

    if (mHasRefsProperty) {
        mPropertyService->unregisterProperty(mHasRefsProperty.get());
        mHasRefsProperty.reset();
    }

    if (mRenderTypeProperty) {
        mPropertyService->unregisterProperty(mRenderTypeProperty.get());
        mRenderTypeProperty.reset();
    }

    if (mRenderAlphaProperty) {
        mPropertyService->unregisterProperty(mRenderAlphaProperty.get());
        mRenderAlphaProperty.reset();
    }

    if (mZBiasProperty) {
        mPropertyService->unregisterProperty(mZBiasProperty.get());
        mZBiasProperty.reset();
    }

    if (mPropModelName) {
        mPropertyService->unregisterProperty(mPropModelName.get());
        mPropModelName.reset();
    }

    if (mPropScale) {
        mPropertyService->unregisterProperty(mPropScale.get());
        mPropScale.reset();
    }

    if (mPropPosition != NULL)
        mPropPosition->unregisterListener(mPropPositionListenerID);
    mPropPosition = NULL;

    if (mLoopService) {
        mLoopService->removeLoopClient(this);
        mLoopService.reset();
    }

    mConfigService.reset();
    mPropertyService.reset();
}

// --------------------------------------------------------------------------
bool RenderService::init() {
    mConfigService = GET_SERVICE(ConfigService);

    mConfigService->setParamDescription("fullscren",
                                        "Toggles display to fullscreen/window");
    mConfigService->setParamDescription(
        "window_width", "Desired game's window/fullscreen width");
    mConfigService->setParamDescription(
        "window_height", "Desired game's window/fullscreen height");
    mConfigService->setParamDescription(
        "display", "Display id (for multi-screen systems)");

    // get screen resolution from opde.cfg
    mCurrentSize.fullscreen =
        mConfigService->getParam("fullscreen", false).toBool();
    mCurrentSize.width = mConfigService->getParam("window_width", 0).toUInt();
    mCurrentSize.height = mConfigService->getParam("window_height", 0).toUInt();
    mCurrentSize.display = mConfigService->getParam("display", -1).toInt();

    LOG_INFO("RenderService: initializing SDL");

    initSDLWindow();

#warning DESTROY mSDLWindow

    LOG_INFO("RenderService: initializing OGRE Window");

    // initialize ogre window and render system...
    initOgreWindow();

    broadcastScreenSize();

    // Dark scene manager factory
    LOG_DEBUG("RenderService::init(): new DarkSceneManagerFactory()");
    mDarkSMFactory = new DarkSceneManagerFactory();

    // Register
    Root::getSingleton().addSceneManagerFactory(mDarkSMFactory);

    mSceneMgr =
        mRoot->createSceneManager("DarkSceneManager", "DarkSceneManager");

    // mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
    mSceneMgr->setShadowTechnique(SHADOWTYPE_NONE);

    // TODO: Set this in Database listener using Render Params chunk
    mSceneMgr->setAmbientLight(ColourValue(0.08, 0.08, 0.08));

    // Next step. We create a default camera in the mRenderWindow
    mDefaultCamera = mSceneMgr->createCamera("MainCamera");

    mDefaultCamera->setCastShadows(true);

    // aspect derived from screen aspect (to avoid stretching)
    mDefaultCamera->setAspectRatio(Real(mRenderWindow->getWidth()) /
                                   Real(mRenderWindow->getHeight()));

    mRenderWindow->addViewport(mDefaultCamera);

    // Last step: Get the loop service and register as a listener
    mLoopService = GET_SERVICE(LoopService);
    mLoopService->addLoopClient(this);

    // prepare the default models and textures
    prepareHardcodedMedia();

    mPropertyService = GET_SERVICE(PropertyService);

    Ogre::uint8 mDefaultRenderQueue =
        mSceneMgr->getRenderQueue()->getDefaultQueueGroup();

    // preset the default render queue according to the RenderQueue
    EntityMaterialInstance::setBaseRenderQueueGroup(mDefaultRenderQueue);

    return true;
}

// --------------------------------------------------------------------------
void RenderService::initSDLWindow() {
    int res = SDL_Init(SDL_INIT_EVERYTHING);
    if (res != 0) {
        OPDE_EXCEPT(String("SDL Initialization error: ") + SDL_GetError(),
                    "RenderService::initSDLWindow");
    }

    // try to reach propper display and resolution
    if (mCurrentSize.fullscreen ||
        (!mCurrentSize.width || !mCurrentSize.height)) {
        // first validate the selected display
        int dispCount = SDL_GetNumVideoDisplays();

        if (dispCount < 0)
            OPDE_EXCEPT(String("SDL can't get display count. Error: ") +
                            SDL_GetError(),
                        "RenderService::initSDLWindow");

        if (mCurrentSize.display < 0)
            mCurrentSize.display = 0;

        if (mCurrentSize.display >= dispCount) {
            LOG_ERROR("Obsolete/invalid display number specified in config, "
                      "ignoring");
            mCurrentSize.display = 0;
        }

        // now try to gather the desktop resolution
        SDL_DisplayMode mode;
        res = SDL_GetDesktopDisplayMode(mCurrentSize.display, &mode);

        if (res)
            OPDE_EXCEPT(String("SDL can't get current video mode. Error: ") +
                            SDL_GetError(),
                        "RenderService::initSDLWindow");

        mCurrentSize.width = mode.w;
        mCurrentSize.height = mode.h;
        mCurrentSize.fullscreen = true;
    }

    // based on the settings, try to set the resolution.
    LOG_INFO("RenderService: Requested%s window %dx%d",
             mCurrentSize.fullscreen ? " fullscreen" : " ", mCurrentSize.width,
             mCurrentSize.height);

    int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

    if (mCurrentSize.fullscreen)
        flags |= SDL_WINDOW_FULLSCREEN;

    mSDLWindow = SDL_CreateWindow("openDarkEngine", SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED, mCurrentSize.width,
                                  mCurrentSize.height, flags);

    if (mSDLWindow == NULL)
        OPDE_EXCEPT("Can't create SDL window", "RenderService::initSDLWindow");
}

// --------------------------------------------------------------------------
void RenderService::initOgreWindow() {
    // where do I find this?
    if (mRoot->getAvailableRenderers().size() < 1)
        OPDE_EXCEPT("Failed to initialize RenderSystem",
                    "RenderService::initOgre");
    mRoot->setRenderSystem(mRoot->getAvailableRenderers()[0]);
    mRoot->initialise(false);

    Ogre::NameValuePairList params;
    // PARTS COPIED FROM https://github.com/TTimo/es_core/
    // this has to be done in platform specific way...

    SDL_SysWMinfo sysInfo;
    SDL_VERSION(&sysInfo.version);
    if (SDL_GetWindowWMInfo(mSDLWindow, &sysInfo) <= 0) {
        OPDE_EXCEPT("Can't create SDL GL context",
                    "RenderService::initSDLWindow");
    }

#ifdef __WINDOWS__
    params["parentWindowHandle"] =
        Ogre::StringConverter::toString(sysInfo.info.win.window);
#elif __LINUX__
    params["parentWindowHandle"] =
        Ogre::StringConverter::toString(sysInfo.info.x11.window);
#elif __APPLE__
#error Nobody tried this yet.
    params["externalGLControl"] = "1";
    params["externalWindowHandle"] = OSX_cocoa_view(mSDLWindow);
    params["macAPI"] = "cocoa";
    params["macAPICocoaUseNSView"] = "true";
#endif

    mRenderWindow = mRoot->createRenderWindow(
        "openDarkEngine", mCurrentSize.width, mCurrentSize.height,
        mCurrentSize.fullscreen, &params);

    mRenderWindow->setVisible(true);
}

// --------------------------------------------------------------------------
Ogre::Root *RenderService::getOgreRoot() {
    assert(mRoot);
    return mRoot;
}

// --------------------------------------------------------------------------
Ogre::SceneManager *RenderService::getSceneManager() {
    assert(mSceneMgr);
    return mSceneMgr;
}

// --------------------------------------------------------------------------
Ogre::RenderWindow *RenderService::getRenderWindow() {
    assert(mRenderWindow);
    return mRenderWindow;
}

// --------------------------------------------------------------------------
Ogre::Viewport *RenderService::getDefaultViewport() {
    assert(mDefaultCamera);
    return mDefaultCamera->getViewport();
}

// --------------------------------------------------------------------------
Ogre::Camera *RenderService::getDefaultCamera() { return mDefaultCamera; }

// --------------------------------------------------------------------------
void RenderService::setScreenSize(bool fullScreen, unsigned int width,
                                  unsigned int height) {
    assert(mRenderWindow);

    mRenderWindow->setFullscreen(fullScreen, width, height);

    mCurrentSize.fullscreen = fullScreen;
    mCurrentSize.width = width;
    mCurrentSize.height = height;

    broadcastScreenSize();
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
    // Property Service should have created us automatically through service
    // masks. So we can register as a link service listener
    LOG_INFO("RenderService::bootstrapFinished()");

    // create the properties the render service uses (built-in)
    createProperties();

    // Get the PropertyService, then the property Position
    // --- Position property listener
    mPropPosition = mPropertyService->getProperty("Position");

    if (mPropPosition == NULL)
        OPDE_EXCEPT("Could not get Position property. Not defined. Fatal",
                    "RenderService::bootstrapFinished");

    // listener to the position property to control the scenenode
    Property::ListenerPtr cposc(
        new ClassCallback<PropertyChangeMsg, RenderService>(
            this, &RenderService::onPropPositionMsg));

    mPropPositionListenerID = mPropPosition->registerListener(cposc);

    // ===== OBJECT SERVICE LISTENER =====
    mObjectService = GET_SERVICE(ObjectService);

    // Listener to object messages
    ObjectService::ListenerPtr objlist(
        new ClassCallback<ObjectServiceMsg, RenderService>(
            this, &RenderService::onObjectMsg));

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
void RenderService::prepareMesh(const Ogre::String &name) {
    String fname = name + ".mesh";

    if (!Ogre::MeshManager::getSingleton().resourceExists(fname)) {
        // Undefine in advance, so there will be no clash
        Ogre::MeshManager::getSingleton().remove(fname);
        // If it is not found
        LOG_DEBUG("RenderService::prepareMesh: Mesh definition for %s not "
                  "found, loading manually from .bin file...",
                  name.c_str());
        // do a create

        try {
            Ogre::MeshPtr mesh1 = Ogre::MeshManager::getSingleton().create(
                fname, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                true, mManualBinFileLoader.get());
        } catch (FileNotFoundException) {
            LOG_ERROR("RenderService::prepareMesh: Could not find the "
                      "requested model %s",
                      name.c_str());
        } catch (Opde::FileException) {
            LOG_ERROR("RenderService::prepareMesh: Could not load the "
                      "requested model %s",
                      name.c_str());
        }
    }
}

// --------------------------------------------------------------------------
void RenderService::createObjectModel(int id) {
    Ogre::String idstr = StringConverter::toString(id);

    Entity *ent =
        mSceneMgr->createEntity("Object" + idstr, DEFAULT_RAMP_OBJECT_NAME);

    // bind the ent to the node of the obj.
    SceneNode *node = NULL;

    try {
        node = getSceneNode(id);
    } catch (BasicException) {
        LOG_ERROR("RenderService: Could not get the sceneNode for object %d! "
                  "Not attaching object model!",
                  id);
        mSceneMgr->destroyEntity(ent);
        return;
    }

    assert(node != NULL);

    SceneNode *enode = mSceneMgr->createSceneNode("ObjMesh" + idstr);
    enode->setPosition(Vector3::ZERO);
    enode->setOrientation(Quaternion::IDENTITY);
    node->addChild(enode);

    enode->attachObject(ent);

    prepareEntity(ent);

    EntityInfoPtr ei(new EntityInfo(mSceneMgr, ent, enode));
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
void RenderService::setObjectModel(int id, const std::string &name) {
    EntityInfo *ei = _getEntityInfo(id);

    LOG_VERBOSE(
        "RenderService::setObjectModel: Object model for %d - setting to %s",
        id, name.c_str());

    prepareMesh(name);

    // if there was an error finding the entity info, the object does not exist
    if (!ei) {
        LOG_VERBOSE("RenderService: Object model could not be set for %d, "
                    "object not found",
                    id);
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
        ent = mSceneMgr->createEntity("Object" + idstr + name, name + ".mesh");
    } catch (Ogre::Exception &e) {
        // TODO: This could also be handled by not setting the new entity at all
        LOG_ERROR("RenderService: Could not load model %s for obj %d : %s",
                  name.c_str(), id, e.getFullDescription().c_str());
        ent = mSceneMgr->createEntity("Object" + idstr + "Ramp",
                                      DEFAULT_RAMP_OBJECT_NAME);
    }

    prepareEntity(ent);

    // will destroy the previous
    ei->setEntity(ent);

    // Last step - refresh the skip
    ei->setSkip(false);
}

// --------------------------------------------------------------------------
void RenderService::prepareEntity(Ogre::Entity *e) {
    e->_initialise(true);

    e->setCastShadows(true);

    // Update the bones to be manual
    // TODO: This should only apply for non-ai meshes!
    if (e->hasSkeleton()) {

        Skeleton::BoneIterator bi = e->getSkeleton()->getBoneIterator();

        while (bi.hasMoreElements()) {
            Bone *n = bi.getNext();

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
    SceneNode *sn = getSceneNode(objID);

    if (sn) {
        sn->attachObject(mDefaultCamera);
    }
}

// --------------------------------------------------------------------------
void RenderService::detachCamera() {
    if (mDefaultCamera->isAttached()) {
        SceneNode *sn = mDefaultCamera->getParentSceneNode();
        sn->detachObject(mDefaultCamera);
    }
}

// --------------------------------------------------------------------------
EntityInfo *RenderService::_getEntityInfo(int oid) {
    ObjectEntityMap::iterator it = mEntityMap.find(oid);

    if (it != mEntityMap.end()) {
        return (it->second.get());
    }

    return NULL;
}

// --------------------------------------------------------------------------
// ---- Scene Node handling routines ----------------------------------------
// --------------------------------------------------------------------------
Ogre::SceneNode *RenderService::getSceneNode(int objID) {
    if (objID > 0) {
        ObjectToNode::iterator snit = mObjectToNode.find(objID);

        if (snit != mObjectToNode.end()) {
            return snit->second;
        }
    }

    // Throwing exceptions is not a good idea
    return NULL;
    // OPDE_EXCEPT("Could not find scenenode for object. Does object exist?",
    // "ObjectService::getSceneNodeForObject");
}

// --------------------------------------------------------------------------
void RenderService::onPropPositionMsg(const PropertyChangeMsg &msg) {
    // Update the scene node's position and orientation
    if (msg.objectID <= 0) // no action for archetypes
        return;

    switch (msg.change) {
    case PROP_ADDED:
    case PROP_CHANGED: {
        try {
            // Find the scene node by it's object id, and update the position
            // and orientation
            Ogre::SceneNode *node = getSceneNode(msg.objectID);

            if (node == NULL)
                return;

            Variant pos;
            mPropPosition->get(msg.objectID, "position", pos);
            Variant ori;
            mPropPosition->get(msg.objectID, "facing", ori);

            node->setPosition(pos.toVector());
            node->setOrientation(ori.toQuaternion());

        } catch (BasicException &e) {
            LOG_ERROR(
                "RenderService: Exception while setting position of object: %s",
                e.getDetails().c_str());
        }

        break;
    }

    default:
        break;
    }
}

// --------------------------------------------------------------------------
void RenderService::onObjectMsg(const ObjectServiceMsg &msg) {
    if (msg.objectID <= 0) // no action for archetypes
        return;

    switch (msg.type) {
    case OBJ_CREATE_STARTED: {
        // create scenenode, return
        std::string nodeName("Object");

        nodeName += Ogre::StringConverter::toString(msg.objectID);

        // Use render service to create a scene node for the object
        Ogre::SceneNode *snode = mSceneMgr->createSceneNode(nodeName);

        assert(snode != NULL);

        // Attach the node to the root SN of the scene
        mSceneMgr->getRootSceneNode()->addChild(snode);

        mObjectToNode.insert(std::make_pair(msg.objectID, snode));

        // set the default model - default ramp
        createObjectModel(msg.objectID);

        return;
    }

    case OBJ_DESTROYED: {
        // destroy all the entities/lights attached, then the scenenode
        // Light and spotlight are removed because of property removal,
        // TODO: no modelName -> default model (mesh) for the object
        // in original dark
        // if not present, so after we construct the same behavior,
        // destroy the entity here

        removeObjectModel(msg.objectID);

        // find the SN, destroy if found
        ObjectToNode::iterator oit = mObjectToNode.find(msg.objectID);

        if (oit != mObjectToNode.end()) {
            std::string nodeName("Object");
            nodeName += Ogre::StringConverter::toString(msg.objectID);
            mSceneMgr->destroySceneNode(oit->second->getName());
        }

        return;
    }

    default:
        break; // nothing, ignore
    }
}

// --------------------------------------------------------------------------
void RenderService::prepareHardcodedMedia() {
    // 1. The default texture - Jorge (recreated to be nearly the same visually)
    TexturePtr jorgeTex = TextureManager::getSingleton().getByName("jorge.png");

    if (!jorgeTex) {
        DataStreamPtr stream(new MemoryDataStream(
            JORGE_TEXTURE_PNG, JORGE_TEXTURE_PNG_SIZE, false));

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

// --------------------------------------------------------------------------
void RenderService::createRampMesh() {
    // Code copied from Ogre3D wiki, and modified (Thanks to the original author
    // for saving my time!)
    /// Create the mesh via the MeshManager
    Ogre::MeshPtr msh = MeshManager::getSingleton().createManual(
        DEFAULT_RAMP_OBJECT_NAME,
        ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    /// Create one submesh
    SubMesh *sub = msh->createSubMesh();
    sub->setMaterialName("BaseWhite");

    // Not exactly accurate for 1:1:2 side size ratio, but well...
    const float sqrt13 = 0.577350269f; /* sqrt(1/3) */

    const size_t nVertices = 6;
    const size_t vbufCount = 3 * 2 * nVertices;

    float vertices[vbufCount] = {
        -2.0,    -1.0,    -1.0,                      // 0 position
        sqrt13,  sqrt13,  sqrt13,  -2.0, 1.0,  -1.0, // 1 position
        sqrt13,  -sqrt13, sqrt13,  -2.0, 1.0,  1.0,  // 2 position
        sqrt13,  -sqrt13, -sqrt13, -2.0, -1.0, 1.0,  // 3 position
        sqrt13,  sqrt13,  -sqrt13, 2.0,  1.0,  -0.3, // 4 position
        -sqrt13, -sqrt13, sqrt13,  2.0,  -1.0, -0.3, // 5 position
        -sqrt13, sqrt13,  sqrt13};

    const size_t ibufCount = 24;
    unsigned short faces[ibufCount] = {0, 1, 2, 0, 2, 3, 5, 1, 0, 5, 4, 1,
                                       5, 3, 2, 5, 2, 4, 1, 4, 2, 5, 0, 3};

    /// Create vertex data structure for 8 vertices shared between submeshes
    msh->sharedVertexData = new VertexData();
    msh->sharedVertexData->vertexCount = nVertices;

    /// Create declaration (memory format) of vertex data
    VertexDeclaration *decl = msh->sharedVertexData->vertexDeclaration;
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
            offset, msh->sharedVertexData->vertexCount,
            HardwareBuffer::HBU_STATIC_WRITE_ONLY);

    /// Upload the vertex data to the card
    vbuf->writeData(0, vbuf->getSizeInBytes(), vertices, true);

    /// Set vertex buffer binding so buffer 0 is bound to our vertex buffer
    VertexBufferBinding *bind = msh->sharedVertexData->vertexBufferBinding;
    bind->setBinding(0, vbuf);

    /// Allocate index buffer of the requested number of vertices (ibufCount)
    HardwareIndexBufferSharedPtr ibuf =
        HardwareBufferManager::getSingleton().createIndexBuffer(
            HardwareIndexBuffer::IT_16BIT, ibufCount,
            HardwareBuffer::HBU_STATIC_WRITE_ONLY);

    /// Upload the index data to the card
    ibuf->writeData(0, ibuf->getSizeInBytes(), faces, true);

    /// Set parameters of the submesh
    sub->useSharedVertices = true;
    sub->indexData->indexBuffer = ibuf;
    sub->indexData->indexCount = ibufCount;
    sub->indexData->indexStart = 0;

    /// Set bounding information (for culling)
    msh->_setBounds(AxisAlignedBox(-2, -1, -1, 2, 1, 1));
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
    mPropModelName.reset(new ModelNameProperty(this, mPropertyService.get()));
    mPropertyService->registerProperty(mPropModelName.get());

    // RenderAlpha property - single float prop
    mRenderAlphaProperty.reset(
        new RenderAlphaProperty(this, mPropertyService.get()));
    mPropertyService->registerProperty(mRenderAlphaProperty.get());

    // HasRefs - single bool prop
    mHasRefsProperty.reset(new HasRefsProperty(this, mPropertyService.get()));
    mPropertyService->registerProperty(mHasRefsProperty.get());

    // RenderType property - single unsigned int property with enum
    mRenderTypeProperty.reset(new RenderTypeProperty(this, mPropertyService.get()));
    mPropertyService->registerProperty(mRenderTypeProperty.get());

    // ZBias - z bias for depth fighting avoidance
    if (mConfigService->getGameType() >
        ConfigService::GAME_TYPE_T1) { // only t1 does not have ZBIAS
        mZBiasProperty.reset(new ZBiasProperty(this, mPropertyService.get()));
        mPropertyService->registerProperty(mZBiasProperty.get());
    }

    // Scale property
    mPropScale.reset(new ModelScaleProperty(this, mPropertyService.get()));
    mPropertyService->registerProperty(mPropScale.get());
}

// --------------------------------------------------------------------------
void RenderService::broadcastScreenSize() {
    RenderServiceMsg msg;

    msg.msg_type = RENDER_WINDOW_SIZE_CHANGE;
    msg.size = mCurrentSize;

    broadcastMessage(msg);
}

//-------------------------- Factory implementation
std::string RenderServiceFactory::mName = "RenderService";

RenderServiceFactory::RenderServiceFactory() : ServiceFactory(){};

const std::string &RenderServiceFactory::getName() { return mName; }

Service *RenderServiceFactory::createInstance(ServiceManager *manager) {
    return new RenderService(manager, mName);
}

const size_t RenderServiceFactory::getSID() { return RenderService::SID; }
} // namespace Opde
