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

#include "ServiceCommon.h"
#include "InputService.h"
#include "OpdeException.h"
#include "logger.h"

#include <OgreResourceGroupManager.h>

using namespace std;
using namespace Ogre;

namespace Opde {

    /*-----------------------------------------------------*/
    /*-------------------- InputService -------------------*/
    /*-----------------------------------------------------*/
    InputService::InputService(ServiceManager *manager) : Service(manager) {
    }

    //------------------------------------------------------
    InputService::~InputService() {
    }


    //------------------------------------------------------
    bool InputService::init() {
		return true;
    }

    //------------------------------------------------------
    void InputService::bootstrapFinished() {
    	// So the services will be able to catch up
	ServiceManager::getSingleton().createByMask(SERVICE_INPUT_LISTENER);
    }



    //-------------------------- Factory implementation
    std::string InputServiceFactory::mName = "InputService";

    InputServiceFactory::InputServiceFactory() : ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
    };

    const std::string& InputServiceFactory::getName() {
		return mName;
    }

    Service* InputServiceFactory::createInstance(ServiceManager* manager) {
	return new InputService(manager);
    }

}
