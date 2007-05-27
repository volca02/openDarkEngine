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
	 
	/** Central manager for the Services. Each service must have the ServiceFactory implemented, and must implement Service class. 
	* @see ServiceFactory
	* @see Service 
	*/
	class ServiceManager : public Singleton<ServiceManager>, public NonCopyable {
		private:
			typedef std::map< std::string, ServiceFactory* > ServiceFactoryMap;
			typedef std::map< std::string, Service* > ServiceInstanceMap;
		
			ServiceFactoryMap serviceFactories;
			ServiceInstanceMap serviceInstances;
		
			ServiceFactory* findFactory(const std::string& name);
			Service* findService(const std::string& name);
			Service* createInstance(const std::string& name);
		public:
			ServiceManager();
			
			/// Destructor. Deletes all registered factories
			~ServiceManager();

			// Singleton releted
			static ServiceManager& getSingleton(void);
			static ServiceManager* getSingletonPtr(void);
		
			/** Registration for the services */
			void addServiceFactory(ServiceFactory* factory);
		
			/** Returns the service, named name, pointer.
			* @param name The service type name (The name returned by the ServiceFactory)
			* @see ServiceFactory
			*/
			Service* getService(const std::string& name);
	};
}


#endif
