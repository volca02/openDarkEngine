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
#include "LoopService.h"
#include "OpdeException.h"
#include "logger.h"

namespace Opde {

    /*-----------------------------------------------------*/
    /*-------------------- LoopService -------------------*/
    /*-----------------------------------------------------*/
    LoopService::LoopService(ServiceManager *manager, const std::string& name) : Service(manager, name) {
    }

    //------------------------------------------------------
    LoopService::~LoopService() {
    }

	//------------------------------------------------------
	bool LoopService::init() {
	}

    //-------------------------- Factory implementation
    std::string LoopServiceFactory::mName = "LoopService";

    LoopServiceFactory::LoopServiceFactory() : ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
    };

    const std::string& LoopServiceFactory::getName() {
		return mName;
    }

    Service* LoopServiceFactory::createInstance(ServiceManager* manager) {
	return new LoopService(manager, mName);
    }

}
