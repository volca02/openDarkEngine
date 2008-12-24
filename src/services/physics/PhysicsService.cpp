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


#include "PhysicsService.h"
#include "OpdeException.h"
#include "ServiceCommon.h"

using namespace std;

namespace Opde {

	/*----------------------------------------------------*/
	/*-------------------- Physics Service ---------------*/
	/*----------------------------------------------------*/
	PhysicsService::PhysicsService(ServiceManager *manager, const std::string& name) : Service(manager, name) {
	    mDbService = GET_SERVICE(DatabaseService);
	}

    //------------------------------------------------------
	bool PhysicsService::init() {
	    return true;
	}

	//------------------------------------------------------
	PhysicsService::~PhysicsService() {
	}

	//-------------------------- Factory implementation
	std::string PhysicsServiceFactory::mName = "PhysicsService";

	PhysicsServiceFactory::PhysicsServiceFactory() : ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
	};

	const std::string& PhysicsServiceFactory::getName() {
		return mName;
	}

	const uint PhysicsServiceFactory::getMask() {
		return SERVICE_ENGINE;
	}

	Service* PhysicsServiceFactory::createInstance(ServiceManager* manager) {
		return new PhysicsService(manager, mName);
	}

}
