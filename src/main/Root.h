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

#include "compat.h"
#include "integers.h"
#include "OpdeSingleton.h"
#include "logger.h"
#include "OgreOpdeLogConnector.h"
#include "OpdeServiceManager.h"
#include "ServiceCommon.h"

#include <OgreRoot.h>
#include <OgreLogManager.h>

// Script compilers
#include "DTypeScriptCompiler.h"
#include "PLDefScriptCompiler.h"

namespace Opde {
	
	/** OPDE core class. Used to initialize the whole engine. Singleton */
	class Root : public Singleton<Root> {
		public:
			/** Initializes the opde core
			* @param serviceMask the mask of the services which should be used (others will be ignored and unreachable) */
			Root(uint serviceMask = SERVICE_ALL);
			
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
			void addResourceLocation(const std::string& archName, const std::string& typeName, const std::string& secName, bool recursive = false);

			/// Only a wrapper around the Ogre::ResourceGroupManager::removeResourceLocation
			void removeResourceLocation(const std::string& archName, const std::string& typeName, const std::string& secName);


			/** To be called when bootstrapping process was finished */
			void bootstrapFinished();
			
			Logger* getLogger() { return mLogger; };
			ServiceManager* getServiceManager() { return mServiceMgr; };
			
		protected:
			void registerServiceFactories();


			Logger* mLogger;
			ServiceManager* mServiceMgr;
			Ogre::Root* mOgreRoot;
			Ogre::LogManager* mOgreLogManager;
			OgreOpdeLogConnector* mOgreOpdeLogConnector;
			
			DTypeScriptCompiler* mDTypeScriptCompiler;
			PLDefScriptCompiler* mPLDefScriptCompiler;
			
			const unsigned int mServiceMask;		
	};
	
} // namespace Opde

#endif
