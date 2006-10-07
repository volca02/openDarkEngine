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
	
	template<> OpdeServiceManager* Singleton<OpdeServiceManager>::ms_Singleton = 0;
		
	OpdeServiceManager::OpdeServiceManager() {
		
	}
	
	OpdeServiceManager& OpdeServiceManager::getSingleton(void) {
		assert( ms_Singleton );  return ( *ms_Singleton );  
	}
	
	OpdeServiceManager* OpdeServiceManager::getSingletonPtr(void) {
		return ms_Singleton;
	}
	
	
	//------------------ Main implementation ------------------
	void OpdeServiceManager::addServiceFactory(OpdeServiceFactory* factory) {
		
		// std::cout << "Registering Factory named '" << factory->ServiceName << "'" << std::endl;
		
		serviceFactories.insert(make_pair(factory->ServiceName, factory));
	}
	
	OpdeServiceFactory* OpdeServiceManager::findFactory(const string& name) {
		
		ServiceFactoryMap::const_iterator factory_it = serviceFactories.find(name);
		
		if (factory_it != serviceFactories.end()) {
			return factory_it->second;
		} else {
			return NULL;
		}
	}
	
	OpdeService* OpdeServiceManager::findService(const string& name) {
		
		ServiceInstanceMap::iterator service_it = serviceInstances.find(name);
		
		if (service_it != serviceInstances.end()) {
			return service_it->second;
		} else {
			return NULL;
		}
	}
	
	OpdeService* OpdeServiceManager::createInstance(const string& name) {
		OpdeServiceFactory* factory = findFactory(name);
		
		if (factory != NULL) { // Found a factory for the Service name
			return factory->createInstance(this);
		} else {
			OPDE_EXCEPT(string("Factory named ") + name + string(" not found"), "OpdeServiceManager::getService");
		}
	}
	
	OpdeService* OpdeServiceManager::getService(const string& name) {
		OpdeService *service = findService(name);
		
		if (service != NULL) 
			return service;
		else {
			return createInstance(name);
		}
	}
	
} 
