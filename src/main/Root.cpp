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
 *    $Id$
 *
 *****************************************************************************/

#include "config.h"

#include "OgreResourceGroupManager.h"
#include "Root.h"

#include "CustomImageCodec.h"

#include "config/ConfigService.h"
#include <OgreConfigFile.h>
#include <OgreResourceGroupManager.h>

#include "filelog.h"

#include "OpdeServiceManager.h"

// Services
#include "camera/CameraService.h"
#include "database/DatabaseService.h"
#include "draw/DrawService.h"
#include "game/GameService.h"
#include "gui/GUIService.h"
#include "input/InputService.h"
#include "light/LightService.h"
#include "link/LinkService.h"
#include "loop/LoopService.h"
#include "material/MaterialService.h"
#include "object/ObjectService.h"
#include "physics/PhysicsService.h"
#include "platform/PlatformService.h"
#include "player/PlayerService.h"
#include "property/PropertyService.h"
#include "render/RenderService.h"
#include "room/RoomService.h"
#include "worldrep/WorldRepService.h"

// Base
#include "ManualBinFileLoader.h"
#include "ManualFonFileLoader.h"
#include "OgreFixedZip.h"
#include "OpdeException.h"
#include "ProxyArchive.h"
#include "logger.h"
#include "stdlog.h"
#include "tracer.h"

namespace Opde {
// -------------------------------------------------------
// singleton related
template <> Root *Singleton<Root>::ms_Singleton = 0;

// -------------------------------------------------------
Root::Root(uint serviceMask, const char *logFileName)
    : mLogger(),
      mServiceMgr(),
      mOgreRoot(),
      mOgreLogManager(),
      mConsoleBackend(),
      mServiceMask(serviceMask),
      mDirArchiveFactory(),
      mResourceGroupManager(),
      mArchiveManager()
{
    mLogger.reset(new Logger());

    if (logFileName) {
        logToFile(logFileName);
    }

    mTracer.reset(new Opde::Tracer());

    LOG_INFO("Root: Starting openDarkEngine %d.%d.%d (%s), build %s, %s",
             OPDE_VER_MAJOR, OPDE_VER_MINOR, OPDE_VER_PATCH, OPDE_CODE_NAME,
             __DATE__, __TIME__);

    mServiceMgr.reset(new ServiceManager(mServiceMask));

    LOG_INFO("Root: Created a ServiceManager instance with global mask %X",
             mServiceMask);

    LOG_INFO("Root: Hooking up the Ogre logging");
    // To supress logging of OGRE (we'll use a plugin for our logger for Ogre
    // logs) we need to create a Ogre::LogManager here on our own
    mOgreLogManager.reset(new Ogre::LogManager());
    mOgreLogManager->createLog("Ogre.log", true, false, true);

    mOgreOpdeLogConnector.reset(new OgreOpdeLogConnector(mLogger.get()));

    // create our logger's ogre log listener interface. Connect together
    mOgreLogManager->getDefaultLog()->addListener(mOgreOpdeLogConnector.get());

    // Only initialize ogre if rendering is in serviceMask
    if (serviceMask & SERVICE_RENDERER) {
        LOG_INFO("Root: Initializing the Ogre's Root object");
        mOgreRoot.reset(new Ogre::Root());
    } else {
        LOG_INFO("Root: Initializing the Ogre's Resource system");
        // only initialize Archive manager and ResourceGroupManager
        mResourceGroupManager.reset(new Ogre::ResourceGroupManager());
        mArchiveManager.reset(new Ogre::ArchiveManager());
    }

    LOG_INFO("Root: Registering custom archive factories");
    // register the factories
    mDirArchiveFactory.reset(new Ogre::CaseLessFileSystemArchiveFactory());
    mZipArchiveFactory.reset(new Ogre::FixedZipArchiveFactory());
    mCrfArchiveFactory.reset(new Ogre::CrfArchiveFactory());

    Ogre::ArchiveManager::getSingleton().addArchiveFactory(
        mDirArchiveFactory.get());
    Ogre::ArchiveManager::getSingleton().addArchiveFactory(
        mZipArchiveFactory.get());
    Ogre::ArchiveManager::getSingleton().addArchiveFactory(
        mCrfArchiveFactory.get());

    if (serviceMask & SERVICE_RENDERER) {
        LOG_INFO("Root: Hooking up custom 8 bit image codecs");
        // if custom image hooks are to be included, setup now
        Ogre::CustomImageCodec::startup();
    }

    LOG_INFO("Root: Creating console backend");

    mConsoleBackend.reset(new ConsoleBackend());
    mConsoleBackend->putMessage("==Console Starting==");

    LOG_INFO("Root: Registering Service factories");
    // Now we need to register all the service factories
    registerServiceFactories();
    setupLoopModes();
}

// -------------------------------------------------------
Root::~Root() {
    LOG_INFO("Root: openDarkEngine is shutting down");
    // Archive manager has no way to remove the archive factories...
    mServiceMgr.reset();

    // delete all the service factories
    mConsoleBackend.reset();

    if (mServiceMask & SERVICE_RENDERER) {
        Ogre::CustomImageCodec::shutdown();
    }

    mOgreRoot.reset();
    mResourceGroupManager.reset();
    mArchiveManager.reset();

    for (auto &listener : mLogListeners) {
        mLogger->unregisterLogListener(listener.get());
    }
    mLogListeners.clear();

    mDirArchiveFactory.reset();
    mCrfArchiveFactory.reset();
    mZipArchiveFactory.reset();

    // As the last thing - release the logger
    mOgreOpdeLogConnector.reset();
    mOgreLogManager.reset();
    mLogger.reset();
}

// -------------------------------------------------------
void Root::bootstrapFinished() {
    LOG_INFO("Root: Bootstrapping finished");
    // Initialize all the remainging services in the service mask
    mServiceMgr->createByMask(mServiceMask);
    // Initialise all resources
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    // report the boostrap finished to all services
    mServiceMgr->bootstrapFinished();
}

// -------------------------------------------------------
void Root::loadConfigFile(const std::string &fileName) {
    ConfigServicePtr cfp = GET_SERVICE(ConfigService);

    cfp->loadParams(fileName);
}

// -------------------------------------------------------
void Root::loadResourceConfig(const std::string &fileName) {
    Ogre::ConfigFile cf;
    cf.load(fileName);

    // Go through all sections & settings in the file
    const auto &settings = cf.getSettingsBySection();

    Ogre::String secName, typeName, archName;

    for (const auto &sec : settings) {
        const Ogre::String &secName = sec.first;
        for (const auto &set : sec.second) {
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                set.second, set.first, secName);
        }
    }
}

// -------------------------------------------------------
void Root::addResourceLocation(const std::string &name,
                               const std::string &typeName,
                               const std::string &secName, bool recursive) {
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        name, typeName, secName, recursive);
}

// -------------------------------------------------------
void Root::removeResourceLocation(const std::string &name,
                                  const std::string &secName) {
    Ogre::ResourceGroupManager::getSingleton().removeResourceLocation(name,
                                                                      secName);
}

// -------------------------------------------------------
void Root::registerServiceFactories() {
    mServiceMgr->registerFactory<WorldRepServiceFactory>();
    mServiceMgr->registerFactory<GameServiceFactory>();
    mServiceMgr->registerFactory<PhysicsServiceFactory>();
    mServiceMgr->registerFactory<ConfigServiceFactory>();
    mServiceMgr->registerFactory<LinkServiceFactory>();
    mServiceMgr->registerFactory<PropertyServiceFactory>();
    mServiceMgr->registerFactory<InheritServiceFactory>();
    mServiceMgr->registerFactory<RenderServiceFactory>();
    mServiceMgr->registerFactory<DatabaseServiceFactory>();
    mServiceMgr->registerFactory<InputServiceFactory>();
    mServiceMgr->registerFactory<LoopServiceFactory>();
    mServiceMgr->registerFactory<ObjectServiceFactory>();
    mServiceMgr->registerFactory<LightServiceFactory>();
    mServiceMgr->registerFactory<MaterialServiceFactory>();
    mServiceMgr->registerFactory<DrawServiceFactory>();
    mServiceMgr->registerFactory<RoomServiceFactory>();
    mServiceMgr->registerFactory<PlatformServiceFactory>();
    mServiceMgr->registerFactory<PlayerServiceFactory>();
    mServiceMgr->registerFactory<CameraServiceFactory>();
    mServiceMgr->registerFactory<SimServiceFactory>();
    // HACK: This thing is here so that we can test opdeScript, but still
    // have direct input in GameStateManager...
    // Reason is that GUIService steals direct input
    if (mServiceMask & SERVICE_GUI)
        mServiceMgr->registerFactory<GUIServiceFactory>();
}

// -------------------------------------------------------
void Root::setupLoopModes() {
    // Loop modes are only setup if not masked by global service mask
    if (mServiceMask & SERVICE_ENGINE) {
        // Loop modes are hardcoded
        LoopServicePtr ls = GET_SERVICE(LoopService);
        // Create all the required loop services
        LoopModeDefinition def;

        // Loop mode that does no engine processing at all
        def.id = 1;
        def.name = "GUIOnlyLoopMode";
        def.mask = LOOPMODE_INPUT | LOOPMODE_RENDER;

        ls->createLoopMode(def);

        // Loop mode that runs all the loop clients
        def.id = 0xFF;
        def.name = "AllClientsLoopMode";
        def.mask = LOOPMODE_MASK_ALL_CLIENTS;

        ls->createLoopMode(def);
        ls->requestLoopMode("AllClientsLoopMode");
    }
}

// -------------------------------------------------------
void Root::logToFile(const std::string &fname) {
    // TODO: Ugly. Let the logger hold the listeners?
    mLogListeners.emplace_back(new FileLog(fname));
    mLogger->registerLogListener(mLogListeners.back().get());
}

// -------------------------------------------------------
void Root::logToStderr() {
    mLogListeners.emplace_back(new StdLog());
    mLogger->registerLogListener(mLogListeners.back().get());
}

// -------------------------------------------------------
void Root::setLogLevel(int level) {
    // Call mLogger to setup the log level
    mLogger->setLogLevel(level);
}
} // namespace Opde
