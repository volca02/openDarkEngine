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

#include "ConfigService.h"
#include "OpdeException.h"
#include "logger.h"
#include "ServiceCommon.h"

#include <OgreConfigFile.h>
#include <OgreException.h>

using namespace std;
using namespace Ogre;

namespace Opde {

/*----------------------------------------------------*/
/*-------------------- ConfigService -----------------*/
/*----------------------------------------------------*/
template<> const size_t ServiceImpl<ConfigService>::SID = __SERVICE_ID_CONFIG;

ConfigService::ConfigService(ServiceManager *manager, const std::string& name) : ServiceImpl<ConfigService>(manager, name),
    mConfigPathOverride("") {
}

//------------------------------------------------------
ConfigService::~ConfigService() {
}

//------------------------------------------------------
bool ConfigService::init() {
    mPlatformService = GET_SERVICE(PlatformService);
    return true;
}

//------------------------------------------------------
void ConfigService::shutdown() {
    mPlatformService.reset();
}

//------------------------------------------------------
void ConfigService::setParamDescription(const std::string& param, const std::string& desc) {
    mConfigKeyDescriptions[param] = desc;
}

//------------------------------------------------------
void ConfigService::setParam(const std::string& param, const std::string& value) {
    Parameters::iterator it = mParameters.find(param);

    if (it != mParameters.end()) {
        it->second = value;
    } else {
        mParameters.insert(make_pair(param, value));
    }
}

//------------------------------------------------------
DVariant ConfigService::getParam(const std::string& param, const DVariant &dflt)
{
    Parameters::const_iterator it = mParameters.find(param);

    Parameters::const_iterator dit = mConfigKeyDescriptions.find(param);

    // warn if param has no desc specified
    if (dit == mConfigKeyDescriptions.end()) {
        LOG_ERROR("ConfigService: Warning: Config key '%s' has no description specified", param.c_str());
    }

    if (it != mParameters.end()) {
        return DVariant(it->second);
    } else {
        return dflt;
    }
}

//------------------------------------------------------
bool ConfigService::getParam(const std::string& param, DVariant& tgt) {
    Parameters::const_iterator it = mParameters.find(param);

    if (it != mParameters.end()) {
        tgt = it->second;
        return true;
    } else {
        return false;
    }
}

//------------------------------------------------------
bool ConfigService::hasParam(const std::string& param) {
    Parameters::const_iterator it = mParameters.find(param);

    if (it != mParameters.end()) {
        return true;
    } else {
        return false;
    }
}

//------------------------------------------------------
bool ConfigService::loadParams(const std::string& cfgfile) {
    // do we have an override set?
    if (mConfigPathOverride != "") {
        return loadFromFile(mConfigPathOverride + mPlatformService->getDirectorySeparator() + cfgfile);
    } else {
        // first the global config is loaded
        bool globalok = loadFromFile(mPlatformService->getGlobalConfigPath()
                                     + mPlatformService->getDirectorySeparator()
                                     + cfgfile);

        // then overriden by local one
        bool userok = loadFromFile(mPlatformService->getUserConfigPath() \
                                   + mPlatformService->getDirectorySeparator()
                                   + cfgfile);

        return userok || globalok;
    }
}

//------------------------------------------------------
void ConfigService::setConfigPathOverride(const std::string& cfgpath) {
    mConfigPathOverride = cfgpath;
}

//------------------------------------------------------
ConfigService::GameType ConfigService::getGameType() {
    GameType gt = GAME_TYPE_INVALID;

    DVariant val;

    if (getParam("game_type", val)) {
        if (val.toString() == "t1")
            gt = GAME_TYPE_T1;
        else if (val.toString() == "t2")
            gt = GAME_TYPE_T2;
        else if (val.toString() == "ss2")
            gt = GAME_TYPE_SS2;
    }

    return gt;
}

//------------------------------------------------------
std::string ConfigService::getLanguage() {
    DVariant val = "english";

    // a trick - if not found, will use the previous
    // otherwise it will replace.
    getParam("language", val);

    return val.toString();
}

//------------------------------------------------------
std::string ConfigService::getLocalisedResourcePath(const std::string& origPath) {
    std::string path, fname;

    StringUtil::splitFilename(origPath, fname, path);

    return path + getLanguage() + mPlatformService->getDirectorySeparator() + fname;
}

//------------------------------------------------------
void ConfigService::logAllParameters() {
    Parameters::const_iterator dit = mConfigKeyDescriptions.begin();

    LOG_INFO("ConfigService: Configuration parameters:");

    while (dit != mConfigKeyDescriptions.end()) {
        LOG_INFO("ConfigService:\t%s - %s", dit->first.c_str(), dit->second.c_str());
    }
}

//------------------------------------------------------
bool ConfigService::loadFromFile(const std::string& cfgfile) {
    try  {  // load a few options
        LOG_INFO("ConfigService: Loading config file from '%s'", cfgfile.c_str());

        Ogre::ConfigFile cf;
        cf.load(cfgfile);

        // Get the iterator over values - no section
        ConfigFile::SettingsIterator it = cf.getSettingsIterator();


        while (it.hasMoreElements()) {
            std::string key = it.peekNextKey();
            std::string val = it.peekNextValue();

            setParam(key, val);

            it.moveNext();
        }

        return true;
    }
    catch (Ogre::Exception e) {
        LOG_ERROR("Config file '%s' was not found", cfgfile.c_str());
        return false;
        // Guess the file didn't exist
    }
}

//-------------------------- Factory implementation
std::string ConfigServiceFactory::mName = "ConfigService";

ConfigServiceFactory::ConfigServiceFactory() : ServiceFactory() {
};

const std::string& ConfigServiceFactory::getName() {
    return mName;
}

const uint ConfigServiceFactory::getMask() {
    return SERVICE_CORE;
}

const size_t ConfigServiceFactory::getSID() {
    return ConfigService::SID;
}

Service* ConfigServiceFactory::createInstance(ServiceManager* manager) {
    return new ConfigService(manager, mName);
}

}
