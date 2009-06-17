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
 *
 *		$Id$
 *
 *****************************************************************************/

#include "config.h"

#include <iostream>
#include <vector>

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "OpdeException.h"
#include "logger.h"

using namespace std;

namespace Opde {

	template<> ServiceManager* Singleton<ServiceManager>::ms_Singleton = 0;

	ServiceManager::ServiceManager(uint serviceMask) :
		mServiceFactories(),
		mServiceInstances(),
		mBootstrapFinished(false),
		mGlobalServiceMask(serviceMask) {
	}

	ServiceManager::~ServiceManager() {
		// Will release all services
		LOG_DEBUG("ServiceManager: Releasing all services");

		ServiceInstanceMap::iterator s_it;

		s_it = mServiceInstances.begin();

		LOG_INFO("ServiceManager: Shutting down all services");

		// Shutdown loop. Resolver for some possible problems
		for (; s_it != mServiceInstances.end(); ++s_it) {
			LOG_INFO(" * Shutting down service '%s'", s_it->first.c_str());
			s_it->second->shutdown();
		}

		s_it = mServiceInstances.begin();
		LOG_INFO("ServiceManager: Releasing all services");

		for (; s_it != mServiceInstances.end(); ++s_it) {
			LOG_INFO(" * Releasing service %s (ref. count %d)", s_it->first.c_str(), s_it->second.getRefCount());

			/*if (s_it->second.getRefCount() > 0)
				LOG_FATAL(" * Service '%s' has reference count > 1. It won't probably be released immediately!", s_it->first.c_str());*/

			s_it->second.setNull();
		}

		mServiceInstances.clear();
		LOG_DEBUG("ServiceManager: Services released");

		// delete all factories registered

		LOG_DEBUG("ServiceManager: Deleting all service factories");
		ServiceFactoryMap::iterator factory_it = mServiceFactories.begin();

		for (; factory_it != mServiceFactories.end(); ++factory_it) {
			delete factory_it->second;
		}

		mServiceFactories.clear();
		LOG_DEBUG("ServiceManager: Service factories deleted");
	}

	ServiceManager& ServiceManager::getSingleton(void) {
		assert( ms_Singleton );  return ( *ms_Singleton );
	}

	ServiceManager* ServiceManager::getSingletonPtr(void) {
		return ms_Singleton;
	}


	//------------------ Main implementation ------------------
	void ServiceManager::addServiceFactory(ServiceFactory* factory) {
		mServiceFactories.insert(make_pair(factory->getName(), factory));
	}

	ServiceFactory* ServiceManager::findFactory(const std::string& name) {

		ServiceFactoryMap::const_iterator factory_it = mServiceFactories.find(name);

		if (factory_it != mServiceFactories.end()) {
			return factory_it->second;
		} else {
			return NULL;
		}
	}

	ServicePtr ServiceManager::findService(const std::string& name) {

		ServiceInstanceMap::iterator service_it = mServiceInstances.find(name);

		if (service_it != mServiceInstances.end()) {
			return service_it->second;
		} else {
			return ServicePtr();
		}
	}

	ServicePtr ServiceManager::createInstance(const std::string& name) {
		ServiceFactory* factory = findFactory(name);

        LOG_DEBUG("ServiceManager: Creating service %s", name.c_str());

		if (factory != NULL) { // Found a factory for the Service name
			if (!(factory->getMask() & mGlobalServiceMask))
			    OPDE_EXCEPT("Initialization of service " + factory->getName() + " was not permitted by mask. Please consult OPDE log for details", "ServiceManager::createInstance");

			ServicePtr ns(factory->createInstance(this));
			mServiceInstances.insert(make_pair(factory->getName(), ns));

			if (!ns->init()) {
			    OPDE_EXCEPT("Initialization of service " + factory->getName() + " failed. Fatal. Please consult OPDE log for details", "ServiceManager::createInstance");
			}

			if (mBootstrapFinished) // Bootstrap already over, call the after-bootstrap init too
                ns->bootstrapFinished();

			return ns;
		} else {
			OPDE_EXCEPT(string("Factory named ") + name + string(" not found"), "OpdeServiceManager::getService");
		}
	}

	ServicePtr ServiceManager::getService(const std::string& name) {
		ServicePtr service = findService(name);

		if (!service.isNull())
			return service;
		else {
    		LOG_DEBUG("ServiceManager: Service instance for %s not found, will create.", name.c_str());
			return createInstance(name);
		}
	}

	void ServiceManager::createByMask(uint mask) {
		ServiceFactoryMap::iterator factory_it = mServiceFactories.begin();

		for (; factory_it != mServiceFactories.end(); ++factory_it) {

			// if the mask fits and the service is permitted by global service mask
			if ((factory_it->second->getMask() & mask) && (factory_it->second->getMask() & mGlobalServiceMask)) {
			    ServicePtr service = getService(factory_it->second->getName());
			}
		}
	}

	void ServiceManager::bootstrapFinished() {
	    if (mBootstrapFinished) // just do this once
            return;

	    // Here's a catch: The services can create other services while bootstrapping
	    // process is performed. That means the service map can be modified,
	    // and so it's not guaranteed that the bootstrap will be called on those.
	    // For a fix, we set the mBootstrapFinished flag in advance, which means
	    // createInstance will also bootstrap

	    std::vector<ServicePtr> toBootstrap;

	    ServiceInstanceMap::iterator it = mServiceInstances.begin();

	    for (; it != mServiceInstances.end() ; ++it )
	    	toBootstrap.push_back(it->second);

	    mBootstrapFinished = true;

	    std::vector<ServicePtr>::iterator tit = toBootstrap.begin();

        for (; tit != toBootstrap.end() ; ++tit ) {
            LOG_DEBUG("ServiceManager: Bootstrap finished: informing %s", (*tit)->getName().c_str());

            (*tit)->bootstrapFinished();
        };
	}

}
