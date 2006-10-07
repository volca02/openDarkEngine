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
 
#ifndef __OPDESERVICEMANAGER_H
#define __OPDESERVICEMANAGER_H

#include "OpdeServiceFactory.h"
#include "OpdeService.h"
#include "OpdeSingleton.h"
#include <map>

namespace Opde {
	 
	/** Central manager for the Services. Each service must have the OpdeServiceFactory implemented, and must implement OpdeService class. 
	* @see OpdeServiceFactory
	* @see OpdeService 
	*/
	class OpdeServiceManager : public Singleton<OpdeServiceManager> {
		private:
			typedef std::map< std::string, OpdeServiceFactory* > ServiceFactoryMap;
			typedef std::map< std::string, OpdeService* > ServiceInstanceMap;
		
			ServiceFactoryMap serviceFactories;
			ServiceInstanceMap serviceInstances;
		
			OpdeServiceFactory* findFactory(const std::string& name);
			OpdeService* findService(const std::string& name);
			OpdeService* createInstance(const std::string& name);
		public:
			OpdeServiceManager();

			// Singleton releted
			static OpdeServiceManager& getSingleton(void);
			static OpdeServiceManager* getSingletonPtr(void);
		
			/** Registration for the services */
			void addServiceFactory(OpdeServiceFactory* factory);
		
			/** Returns the service, named name, pointer.
			* @param name The service type name (The name returned by the OpdeServiceFactory)
			* @see OpdeServiceFactory
			*/
			OpdeService* getService(const std::string& name);
	};
}


#endif
