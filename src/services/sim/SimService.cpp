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


#include "SimService.h"
#include "OpdeException.h"
#include "ServiceCommon.h"

using namespace std;

namespace Opde {

	/*----------------------------------------------------*/
	/*-------------------- Sim Service -------------------*/
	/*----------------------------------------------------*/
	SimService::SimService(ServiceManager *manager, const std::string& name) : Service(manager, name) {
	}

    	//------------------------------------------------------
	bool SimService::init() {
	    return true;
	}

	//------------------------------------------------------
	SimService::~SimService() {
	}

	//------------------------------------------------------
	void SimService::loopStep(float deltaTime) {
		mSimTime += mTimeCoeff * deltaTime;
		// TODO: Iterate over listeners, broadcast
	}
	
	//------------------------------------------------------
	void SimService::registerListener(SimListener* listener) {
		// TODO: Code
	}

	//------------------------------------------------------
	void SimService::unregisterListener(SimListener* listener) {
		// TODO: Code
	}
	
	

	//-------------------------- Factory implementation
	std::string SimServiceFactory::mName = "SimService";

	SimServiceFactory::SimServiceFactory() : ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
	};

	const std::string& SimServiceFactory::getName() {
		return mName;
	}

	const uint SimServiceFactory::getMask() {
		return SERVICE_ENGINE;
	}

	Service* SimServiceFactory::createInstance(ServiceManager* manager) {
		return new SimService(manager, mName);
	}

}
