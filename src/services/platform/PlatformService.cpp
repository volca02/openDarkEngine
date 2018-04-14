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
 *	  $Id$
 *
 *****************************************************************************/

#include "PlatformService.h"
#include "ServiceCommon.h"
#include "logger.h"

#ifdef WIN32
#include "Win32Platform.h"
#else
#ifdef UNIX
#include "UnixPlatform.h"
#else
#ifdef APPLE
#include "ApplePlatform.h"
#else
#error Unknown platform!
#endif
#endif
#endif

using namespace std;

namespace Opde {

/*----------------------------------------------------*/
/*-------------------- PlatformService ---------------*/
/*----------------------------------------------------*/
template <>
const size_t ServiceImpl<PlatformService>::SID = __SERVICE_ID_PLATFORM;

PlatformService::PlatformService(ServiceManager *manager,
                                 const std::string &name)
    : ServiceImpl<PlatformService>(manager, name) {
#ifdef WIN32
    mPlatform = new Win32Platform(this);
#else
#ifdef UNIX
    mPlatform = new UnixPlatform(this);
#else
#ifdef APPLE
    mPlatform = new ApplePlatform(this);
#else
#error Unknown platform!
#endif
#endif
#endif
}

//------------------------------------------------------
PlatformService::~PlatformService() { delete mPlatform; }

//------------------------------------------------------
std::string PlatformService::getGlobalConfigPath() const {
    return mPlatform->getGlobalConfigPath();
}

//------------------------------------------------------
std::string PlatformService::getUserConfigPath() const {
    return mPlatform->getUserConfigPath();
}

//------------------------------------------------------
std::string PlatformService::getDirectorySeparator() const {
    return mPlatform->getDirectorySeparator();
}

//------------------------------------------------------
bool PlatformService::init() {
    LOG_INFO("PlatformService: Global config path : '%s'",
             getGlobalConfigPath().c_str());
    LOG_INFO("PlatformService: User config path   : '%s'",
             getUserConfigPath().c_str());
    return true;
}

//------------------------------------------------------
void PlatformService::bootstrapFinished() {}

//------------------------------------------------------
void PlatformService::shutdown() {}

//-------------------------- Factory implementation
std::string PlatformServiceFactory::mName = "PlatformService";

PlatformServiceFactory::PlatformServiceFactory() : ServiceFactory(){};

const std::string &PlatformServiceFactory::getName() { return mName; }

const uint PlatformServiceFactory::getMask() { return SERVICE_CORE; }

const size_t PlatformServiceFactory::getSID() { return PlatformService::SID; }

Service *PlatformServiceFactory::createInstance(ServiceManager *manager) {
    return new PlatformService(manager, mName);
}

} // namespace Opde
