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
 *
 *		$Id$
 *
 *****************************************************************************/

#include "config.h"

#include <iostream>
#include <sstream>
#include <list>

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "OpdeException.h"
#include "logger.h"

using namespace std;

namespace Opde {

template<> ServiceManager* Singleton<ServiceManager>::ms_Singleton = 0;
const uint ServiceManager::msMaxServiceSID = 128;

ServiceManager::ServiceManager(uint serviceMask) :
    mServiceFactories(),
    mServiceInstances(),
    mBootstrapFinished(false),
    mGlobalServiceMask(serviceMask)
{
    mServiceInstances.grow(msMaxServiceSID);
    mServiceFactories.grow(msMaxServiceSID);

    for (size_t idx = 0; idx < msMaxServiceSID; ++idx) {
        mServiceFactories[idx] = NULL;
        mServiceInstances[idx].reset();
    }
}

ServiceManager::~ServiceManager() {
    // Will release all services
    LOG_DEBUG("ServiceManager: Releasing all services");

    LOG_INFO("ServiceManager: Shutting down all services");

    // Shutdown loop. Resolver for some possible problems
    for (size_t idx = 0; idx < mServiceInstances.size(); ++idx) {
        ServicePtr& sp = mServiceInstances[idx];

        if (sp) {
            LOG_INFO("ServiceManager: * Shutting down service '%s' with ref. count %d", sp->getName().c_str(), sp.use_count());
            sp->shutdown();
        }
    }

    LOG_INFO("ServiceManager: Releasing all services");

    for (size_t idx = 0; idx < mServiceInstances.size(); ++idx) {
        ServicePtr& sp = mServiceInstances[idx];

        if (sp) {
            LOG_INFO("ServiceManager: * Releasing service '%s' with ref. count %d", sp->getName().c_str(), sp.use_count());
            sp.reset();
        }
    }

    mServiceInstances.clear();
    LOG_DEBUG("ServiceManager: Services released");

    mServiceFactories.clear();
}

ServiceManager& ServiceManager::getSingleton(void) {
    assert( ms_Singleton );  return ( *ms_Singleton );
}

ServiceManager* ServiceManager::getSingletonPtr(void) {
    return ms_Singleton;
}


//------------------ Main implementation ------------------
void ServiceManager::addServiceFactory(ServiceFactory* factory) {
    LOG_INFO("ServiceManager: Registered service factory for %s (%d - mask %X)", factory->getName().c_str(), factory->getSID(), factory->getMask());

    size_t fsid = factory->getSID();

    if (fsid > msMaxServiceSID)
        OPDE_EXCEPT("ServiceFactory SID beyond maximum for " + factory->getName(), "ServiceManager::addServiceFactory");

    if (mServiceFactories[fsid] != NULL)
        OPDE_EXCEPT("ServiceFactory SID already taken by " + mServiceFactories[fsid]->getName() + " could not issue it to " + factory->getName(), "ServiceManager::addServiceFactory");

    mServiceFactories[fsid] = factory;
}

ServiceFactory* ServiceManager::findFactory(size_t sid) {
    return mServiceFactories[sid];
}

ServicePtr ServiceManager::findService(size_t sid) {
    return mServiceInstances[sid];
}

ServicePtr ServiceManager::createInstance(size_t sid) {
    ServiceFactory* factory = mServiceFactories[sid];

    if (factory != NULL) { // Found a factory for the Service name
        size_t fsid = factory->getSID();
        assert(fsid == sid);

        LOG_INFO("ServiceManager: Creating service %s (%d)", factory->getName().c_str(), sid);

        if (!(factory->getMask() & mGlobalServiceMask))
            OPDE_EXCEPT("Creation of service " + factory->getName() + " was not permitted by mask. Please consult OPDE log for details", "ServiceManager::createInstance");

        ServicePtr ns(factory->createInstance(this));

        if(mServiceInstances[fsid])
            LOG_INFO("ServiceManager: Service already created?! (%s (%d))", factory->getName().c_str(), sid);

        assert(mServiceInstances[fsid].isNull());
        mServiceInstances[fsid] = ns;

        if (!ns->init()) {
            OPDE_EXCEPT("Initialization of service " + factory->getName() + " failed. Fatal. Please consult OPDE log for details", "ServiceManager::createInstance");
        }

        if (mBootstrapFinished) // Bootstrap already over, call the after-bootstrap init too
            ns->bootstrapFinished();

        return ns;
    } else {
        std::ostringstream oss;
        oss << "ServiceFactory with ID " << sid << string(" not found");

        OPDE_EXCEPT(oss.str(), "OpdeServiceManager::getService");
    }
}

ServicePtr ServiceManager::getService(size_t sid) {
    ServicePtr service = mServiceInstances[sid];

    if (service)
        return service;
    else {
        LOG_DEBUG("ServiceManager: Service instance for %d not found, will create.", sid);
        return createInstance(sid);
    }
}

void ServiceManager::createByMask(uint mask) {
    for (size_t idx = 0; idx < mServiceInstances.size(); ++idx) {
        ServiceFactory* factory = mServiceFactories[idx];

        if (!factory)
            continue;

        // if the mask fits and the service is permitted by global service mask
        if ((factory->getMask() & mask) && (factory->getMask() & mGlobalServiceMask)) {
            ServicePtr service = getService(factory->getSID());
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

    std::list<ServicePtr> toBootstrap;

    for (size_t idx = 0; idx < mServiceInstances.size() ; ++idx ) {
        ServicePtr& sp = mServiceInstances[idx];

        if (sp)
            toBootstrap.push_back(sp);
    }

    mBootstrapFinished = true;

    std::list<ServicePtr>::iterator tit = toBootstrap.begin();

    for (; tit != toBootstrap.end() ; ++tit ) {
        LOG_DEBUG("ServiceManager: Bootstrap finished: informing %s", (*tit)->getName().c_str());
        (*tit)->bootstrapFinished();
    };
}

}
