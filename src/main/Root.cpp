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
 *	  $Id$
 *
 *****************************************************************************/

#include "config.h"

#include "Root.h"
#include "OgreResourceGroupManager.h"

#include "CustomImageCodec.h"

#include "ConfigService.h"
#include <OgreConfigFile.h>
#include <OgreResourceGroupManager.h>

#include "filelog.h"

#include "OpdeServiceManager.h"

// Services
#include "WorldRepService.h"
#include "GameService.h"
#include "PhysicsService.h"
#include "RenderService.h"
#include "InputService.h"
#include "LoopService.h"
#include "ObjectService.h"
#include "LinkService.h"
#include "PropertyService.h"
#include "DatabaseService.h"
#include "MaterialService.h"
#include "LightService.h"
#include "DrawService.h"
#include "RoomService.h"

// base
#include "ManualBinFileLoader.h"
#include "ManualFonFileLoader.h"
#include "ProxyArchive.h"
#include "logger.h"
#include "OpdeException.h"
#include "ConsoleFrontend.h"
#include "stdlog.h"



namespace Opde {
	// -------------------------------------------------------
	// singleton related
	template<> Root* Singleton<Root>::ms_Singleton = 0;

	// -------------------------------------------------------
	Root::Root(uint serviceMask, const char* logFileName) :
			mLogger(NULL),
			mServiceMgr(NULL),
			mOgreRoot(NULL),
			mOgreLogManager(NULL),
			mConsoleBackend(NULL),
			mDTypeScriptCompiler(NULL),
			mPLDefScriptCompiler(NULL),
			mServiceMask(serviceMask),
			mDTypeScriptLdr(NULL),
			mPLDefScriptLdr(NULL),
			mDirArchiveFactory(NULL),
			mCrfArchiveFactory(NULL),
			mResourceGroupManager(NULL),
			mArchiveManager(NULL) {

		mLogger = new Logger();

		if (logFileName) {
			logToFile(logFileName);
		}

		LOG_INFO("Root: Starting openDarkEngine %d.%d.%d (%s), build %s, %s", OPDE_VER_MAJOR, OPDE_VER_MINOR, OPDE_VER_PATCH, OPDE_CODE_NAME, __DATE__, __TIME__);

		mServiceMgr = new ServiceManager(mServiceMask);
		
		LOG_INFO("Root: Created a ServiceManager instance with global mask %X", mServiceMask);

		LOG_INFO("Root: Hooking up the Ogre logging");
		// To supress logging of OGRE (we'll use a plugin for our logger for Ogre logs)
		// we need to create a Ogre::LogManager here on our own
		mOgreLogManager = new Ogre::LogManager();
		mOgreLogManager->createLog("Ogre.log", true, false, true);

		mOgreOpdeLogConnector = new OgreOpdeLogConnector(mLogger);

		// create our logger's ogre log listener interface. Connect together
		mOgreLogManager->getDefaultLog()->addListener(mOgreOpdeLogConnector);

		// Only initialize ogre if rendering is in serviceMask
		if (serviceMask & SERVICE_RENDERER) {
			LOG_INFO("Root: Initializing the Ogre's Root object");
			mOgreRoot = new Ogre::Root();
		} else {
			LOG_INFO("Root: Initializing the Ogre's Resource system");
			// only initialize Archive manager and ResourceGroupManager
			mResourceGroupManager = new Ogre::ResourceGroupManager();
			mArchiveManager = new Ogre::ArchiveManager();
		}

		LOG_INFO("Root: Registering custom archive factories");
		// register the factories
		mDirArchiveFactory = new Ogre::CaseLessFileSystemArchiveFactory();
		// TODO: Decide if this should be used or not
		// mCrfArchiveFactory = new Ogre::CrfArchiveFactory();

		Ogre::ArchiveManager::getSingleton().addArchiveFactory(mDirArchiveFactory);
		// Ogre::ArchiveManager::getSingleton().addArchiveFactory(mCrfArchiveFactory);

		if (serviceMask & SERVICE_RENDERER) {
			LOG_INFO("Root: Hooking up custom 8 bit image codecs");
			// if custom image hooks are to be included, setup now
			Ogre::CustomImageCodec::startup();
		}

		LOG_INFO("Root: Creating console backend");

		mConsoleBackend = new ConsoleBackend();

		LOG_INFO("Root: Registering Service factories");
		// Now we need to register all the service factories
		registerServiceFactories();

		LOG_INFO("Root: Registering custom script compilers");
		mDTypeScriptCompiler = new DTypeScriptCompiler();
		mPLDefScriptCompiler = new PLDefScriptCompiler();

		setupLoopModes();
	}

	// -------------------------------------------------------
	Root::~Root() {
		LOG_INFO("Root: openDarkEngine is shutting down");

		// if those are used, delete them
		delete mDTypeScriptLdr;
		delete mPLDefScriptLdr;

		delete mDTypeScriptCompiler;
		delete mPLDefScriptCompiler;

		// Archive manager has no way to remove the archive factories...

		delete mServiceMgr;

		delete mConsoleBackend;

		if (mServiceMask & SERVICE_RENDERER) {
				Ogre::CustomImageCodec::shutdown();
		}
		
		delete mOgreRoot;
		delete mResourceGroupManager;
		delete mArchiveManager;

		LogListenerList::iterator it = mLogListeners.begin();

		for (;it != mLogListeners.end(); ++it) {
			mLogger->unregisterLogListener(*it);
			delete *it;
		}
		mLogListeners.clear();

		delete mDirArchiveFactory;
		delete mCrfArchiveFactory;

		// As the last thing - release the logger
		delete mLogger;

		delete mOgreOpdeLogConnector;

		delete mOgreLogManager;


	}


	// -------------------------------------------------------
	void Root::registerCustomScriptLoaders() {
		// TODO: bindings

		// the classes register themselves to ogre
		if (!mDTypeScriptLdr)
			mDTypeScriptLdr = new DTypeScriptLoader();

		if (!mPLDefScriptLdr)
			mPLDefScriptLdr = new PLDefScriptLoader();
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
	void Root::loadConfigFile(const std::string& fileName) {
		ConfigServicePtr cfp = GET_SERVICE(ConfigService);

		cfp->loadParams(fileName);
	}

	// -------------------------------------------------------
	void Root::loadPLDefScript(const std::string& fileName, const std::string& groupName) {
		// try to open the given resource stream
		Ogre::DataStreamPtr str = Ogre::ResourceGroupManager::getSingleton().openResource(fileName, groupName, true, NULL);
		mPLDefScriptCompiler->parseScript(str, groupName);
	}

	// -------------------------------------------------------
	void Root::loadDTypeScript(const std::string& fileName, const std::string& groupName) {
		Ogre::DataStreamPtr str = Ogre::ResourceGroupManager::getSingleton().openResource(fileName, groupName, true, NULL);
		mDTypeScriptCompiler->parseScript(str, groupName);
	}

	// -------------------------------------------------------
	void Root::loadResourceConfig(const std::string& fileName) {
	    Ogre::ConfigFile cf;
		cf.load(fileName);

		// Go through all sections & settings in the file
		Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

		Ogre::String secName, typeName, archName;

		while (seci.hasMoreElements()) {
			secName = seci.peekNextKey();
			Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
			Ogre::ConfigFile::SettingsMultiMap::iterator i;

			for (i = settings->begin(); i != settings->end(); ++i) {
				typeName = i->first;
				archName = i->second;
				Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
					archName, typeName, secName);
			}
		}
	}

	// -------------------------------------------------------
	void Root::addResourceLocation(const std::string& name, const std::string& typeName, const std::string& secName, bool recursive) {
		Ogre::ResourceGroupManager::getSingleton().addResourceLocation(name, typeName, secName, recursive);
	}

	// -------------------------------------------------------
	void Root::removeResourceLocation(const std::string& name, const std::string& secName) {
		Ogre::ResourceGroupManager::getSingleton().removeResourceLocation(name, secName);
	}

	// -------------------------------------------------------
	void Root::registerServiceFactories() {
		// register all the service factories
		// The factories are deleted in service manager

		// TODO: WE ALL KNOW this is VERY WRONG way to work with memory allocation

		/* The right way would be for example:
		1. ServiceFactoryPtr - shared_ptr<ServiceFactory>;
		2. Work everywhere with the shared_ptr instead
		*/

		new WorldRepServiceFactory();
		new BinaryServiceFactory();
		new GameServiceFactory();
		new PhysicsServiceFactory();
		new ConfigServiceFactory();
		new LinkServiceFactory();
		new PropertyServiceFactory();
		new InheritServiceFactory();
		new RenderServiceFactory();
		new DatabaseServiceFactory();
		new InputServiceFactory();
		new LoopServiceFactory();
		new ObjectServiceFactory();
		new LightServiceFactory();
		new MaterialServiceFactory();
		new DrawServiceFactory();
		new RoomServiceFactory();
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
		}
	}

	// -------------------------------------------------------
	void Root::logToFile(const std::string& fname) {
		LogListener* flog = new FileLog(fname);

		mLogger->registerLogListener(flog);
		mLogListeners.push_back(flog);
	}

	// -------------------------------------------------------
	void Root::setLogLevel(int level) {
		// Call mLogger to setup the log level
		mLogger->setLogLevel(level);
	}
}
