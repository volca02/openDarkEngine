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
 *	  $Id$
 *
 *****************************************************************************/

#ifndef __ROOT_H
#define __ROOT_H

#include "config.h"

#include "compat.h"
#include "integers.h"
#include "OpdeSingleton.h"
#include "logger.h"
#include "OgreOpdeLogConnector.h"
#include "OpdeServiceManager.h"
#include "ServiceCommon.h"
#include "ConsoleBackend.h"
#include "ConsoleFrontend.h"

#include <OgreRoot.h>
#include <OgreLogManager.h>

// Script compilers
#include "DTypeScriptCompiler.h"
#include "PLDefScriptCompiler.h"

// script loaders
#include "DTypeScriptLoader.h"
#include "PLDefScriptLoader.h"

namespace Opde {
	
	/** OPDE core class. Used to initialize the whole engine. Singleton */
	class OPDELIB_EXPORT Root : public Singleton<Root> {
		public:
			/** Initializes the opde core
			* @param serviceMask the mask of the services which should be used (others will be ignored and unreachable) 
			* @param logFileName - optional log file name - when specified, logging to file will be initialized automatically
			*/
			Root(uint serviceMask = SERVICE_ALL, char* logFileName = NULL);
			
			/** stops the opde core, does cleanup */
			~Root();

			// ----------------- Methods used for boostrapping --------------------
			/// Loads a config file, given it's file name, which contains resource locations configuration
			void loadResourceConfig(const std::string& fileName);
			
			/// Loads dynamic type definitions from a given file name and group name
			void loadDTypeScript(const std::string& fileName, const std::string& groupName);
			
			/// Loads property and link definitions
			void loadPLDefScript(const std::string& fileName, const std::string& groupName);
			
			/// Loads a config file with opde settings
			void loadConfigFile(const std::string& fileName);

			/// Only a wrapper around the Ogre::ResourceGroupManager::addResourceLocation
			void addResourceLocation(const std::string& name, const std::string& typeName, const std::string& secName, bool recursive = false);

			/// Only a wrapper around the Ogre::ResourceGroupManager::removeResourceLocation
			void removeResourceLocation(const std::string& name, const std::string& secName);


			/** To be called when bootstrapping process was finished */
			void bootstrapFinished();
			
			/** @returns a pointer to the logger used */
			Logger* getLogger() { return mLogger; };
			
			/** @returns a pointer to the service manager */
			ServiceManager* getServiceManager() { return mServiceMgr; };
			
			/** Creates a new logger instance that logs to a file (Logger will be automatically destroyed on termination) */
			void logToFile(const std::string& fname);
			
			/** A shortcut to set loglevel. Valid values are 0-4 */
			void setLogLevel(int level);
			
			/// registers custom script loaders with ogre, meaning the custom scripts will get loaded automatically
			void registerCustomScriptLoaders();
			
		protected:
			/// Registers all the service factories to the Service Manger
			void registerServiceFactories();

			/// Creates all the loop modes
			void setupLoopModes();

			Logger* mLogger;
			ServiceManager* mServiceMgr;
			Ogre::Root* mOgreRoot;
			Ogre::LogManager* mOgreLogManager;
			OgreOpdeLogConnector* mOgreOpdeLogConnector;
			
			/// @deprecated
			ConsoleBackend* mConsoleBackend;
			
			DTypeScriptCompiler* mDTypeScriptCompiler;
			PLDefScriptCompiler* mPLDefScriptCompiler;
			
			typedef std::list< LogListener* > LogListenerList;
			
			
			LogListenerList mLogListeners;
			
			const unsigned int mServiceMask;		
			
			/// Loader for the DType scripts. Only used if 
			DTypeScriptLoader* mDTypeScriptLdr;

			/// Loader for the PLDef scripts
			PLDefScriptLoader* mPLDefScriptLdr;
			
			Ogre::ArchiveFactory* mDirArchiveFactory;
			
			Ogre::ArchiveFactory* mCrfArchiveFactory;
	};
	
} // namespace Opde

#endif
