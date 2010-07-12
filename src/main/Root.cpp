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
#include "BinaryService.h"
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
#include "GUIService.h"
#include "PlatformService.h"
#include "PlayerService.h"

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
#ifdef SCRIPT_COMPILERS
			mDTypeScriptCompiler(NULL),
			mPLDefScriptCompiler(NULL),
			mDTypeScriptLdr(NULL),
			mPLDefScriptLdr(NULL),
#endif
			mServiceMask(serviceMask),
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
		mCrfArchiveFactory = new Ogre::CrfArchiveFactory();

		Ogre::ArchiveManager::getSingleton().addArchiveFactory(mDirArchiveFactory);
		Ogre::ArchiveManager::getSingleton().addArchiveFactory(mCrfArchiveFactory);

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
#ifdef SCRIPT_COMPILERS
		LOG_INFO("Root: Registering custom script compilers");
		mDTypeScriptCompiler = new DTypeScriptCompiler();
		mPLDefScriptCompiler = new PLDefScriptCompiler();
#endif
		setupLoopModes();
	}

	// -------------------------------------------------------
	Root::~Root() {
		LOG_INFO("Root: openDarkEngine is shutting down");
#ifdef SCRIPT_COMPILERS
		// if those are used, delete them
		delete mDTypeScriptLdr;
		delete mPLDefScriptLdr;

		delete mDTypeScriptCompiler;
		delete mPLDefScriptCompiler;
#endif
		// Archive manager has no way to remove the archive factories...
		delete mServiceMgr;
		
		// delete all the service factories
		ServiceFactoryList::iterator sit = mServiceFactories.begin();

		while (sit != mServiceFactories.end()) {
			delete *sit++;
		}
		
		mServiceFactories.clear();

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
#ifdef SCRIPT_COMPILERS
		// the classes register themselves to ogre
		if (!mDTypeScriptLdr)
			mDTypeScriptLdr = new DTypeScriptLoader();

		if (!mPLDefScriptLdr)
			mPLDefScriptLdr = new PLDefScriptLoader();
#else
		OPDE_EXCEPT("Opde Script Compilers not compiled in!", "Root::registerCustomScriptLoaders");
#endif
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
#ifdef SCRIPT_COMPILERS	
		// try to open the given resource stream
		Ogre::DataStreamPtr str = Ogre::ResourceGroupManager::getSingleton().openResource(fileName, groupName, true, NULL);
		mPLDefScriptCompiler->parseScript(str, groupName);
#else
		OPDE_EXCEPT("Opde Script Compilers not compiled in!", "Root::loadPLDefScript");
#endif
	}

	// -------------------------------------------------------
	void Root::loadDTypeScript(const std::string& fileName, const std::string& groupName) {
#ifdef SCRIPT_COMPILERS
		Ogre::DataStreamPtr str = Ogre::ResourceGroupManager::getSingleton().openResource(fileName, groupName, true, NULL);
		mDTypeScriptCompiler->parseScript(str, groupName);
#else
		OPDE_EXCEPT("Opde Script Compilers not compiled in!", "Root::loadDTypeScript");
#endif
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
		mServiceFactories.push_back(new WorldRepServiceFactory());
		mServiceFactories.push_back(new BinaryServiceFactory());
		mServiceFactories.push_back(new GameServiceFactory());
		mServiceFactories.push_back(new PhysicsServiceFactory());
		mServiceFactories.push_back(new ConfigServiceFactory());
		mServiceFactories.push_back(new LinkServiceFactory());
		mServiceFactories.push_back(new PropertyServiceFactory());
		mServiceFactories.push_back(new InheritServiceFactory());
		mServiceFactories.push_back(new RenderServiceFactory());
		mServiceFactories.push_back(new DatabaseServiceFactory());
		mServiceFactories.push_back(new InputServiceFactory());
		mServiceFactories.push_back(new LoopServiceFactory());
		mServiceFactories.push_back(new ObjectServiceFactory());
		mServiceFactories.push_back(new LightServiceFactory());
		mServiceFactories.push_back(new MaterialServiceFactory());
		mServiceFactories.push_back(new DrawServiceFactory());
		mServiceFactories.push_back(new RoomServiceFactory());
		// mServiceFactories.push_back(new GUIServiceFactory());
		mServiceFactories.push_back(new PlatformServiceFactory());
		
		ServiceFactoryList::iterator it = mServiceFactories.begin();
		
		while (it != mServiceFactories.end()) {
			mServiceMgr->addServiceFactory(*it++);
		}
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
