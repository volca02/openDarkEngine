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
 

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "OpdeException.h"


using namespace std;

namespace Opde {
	
	template<> ServiceManager* Singleton<ServiceManager>::ms_Singleton = 0;
		
	ServiceManager::ServiceManager() {
		
	}
	
	ServiceManager& ServiceManager::getSingleton(void) {
		assert( ms_Singleton );  return ( *ms_Singleton );  
	}
	
	ServiceManager* ServiceManager::getSingletonPtr(void) {
		return ms_Singleton;
	}
	
	
	//------------------ Main implementation ------------------
	void ServiceManager::addServiceFactory(ServiceFactory* factory) {
		
		// std::cout << "Registering Factory named '" << factory->ServiceName << "'" << std::endl;
		
		serviceFactories.insert(make_pair(factory->ServiceName, factory));
	}
	
	ServiceFactory* ServiceManager::findFactory(const std::string& name) {
		
		ServiceFactoryMap::const_iterator factory_it = serviceFactories.find(name);
		
		if (factory_it != serviceFactories.end()) {
			return factory_it->second;
		} else {
			return NULL;
		}
	}
	
	Service* ServiceManager::findService(const std::string& name) {
		
		ServiceInstanceMap::iterator service_it = serviceInstances.find(name);
		
		if (service_it != serviceInstances.end()) {
			return service_it->second;
		} else {
			return NULL;
		}
	}
	
	Service* ServiceManager::createInstance(const std::string& name) {
		ServiceFactory* factory = findFactory(name);
		
		if (factory != NULL) { // Found a factory for the Service name
			return factory->createInstance(this);
		} else {
			OPDE_EXCEPT(string("Factory named ") + name + string(" not found"), "OpdeServiceManager::getService");
		}
	}
	
	Service* ServiceManager::getService(const std::string& name) {
		Service *service = findService(name);
		
		if (service != NULL) 
			return service;
		else {
			return createInstance(name);
		}
	}
	
} 
