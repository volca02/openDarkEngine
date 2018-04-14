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

#include "ScriptService.h"
#include "OpdeException.h"
#include "ServiceCommon.h"
#include "bindings.h"
#include "logger.h"

#include <OgreDataStream.h>
#include <OgreResourceGroupManager.h>

using namespace std;

namespace Opde {

/*------------------------------------------------------*/
/*-------------------- ScriptService -------------------*/
/*------------------------------------------------------*/
template <> const size_t ServiceImpl<ScriptService>::SID = __SERVICE_ID_SCRIPT;

ScriptService::ScriptService(ServiceManager *manager, const std::string &name)
    : ServiceImpl<Opde::ScriptService>(manager, name){

      };

//------------------------------------
ScriptService::~ScriptService(){
    // Release all scripts and script modules
};

//------------------------------------
void ScriptService::runScript(const std::string &filename) {
    // Get the script as a memory pointer first, using the Ogre's resource
    // system Then use python to run the script. If it communicates with opde
    // any way, Py_INCREF will guarantee consistency on those interacting
    // pieces.

    // TODO: Compiled code v.s. code to compile (.py / .pyc)

    // TODO: Group name for scripts! (Using default for now)
    Ogre::DataStreamPtr fdata =
        Ogre::ResourceGroupManager::getSingleton().openResource(
            filename,
            Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
    const Ogre::String &text = fdata->getAsString();

    // Now run the script using python
    PythonLanguage::runScriptPtr(text.c_str());
}

//------------------------------------
void ScriptService::addObjectScriptModule(const ObjectScriptModulePtr &module) {
    // List of modules to be released
    mScriptModules.push_back(module);

    // Association of module's scripts.
    const std::set<std::string> &modnames = module->getScriptNames();

    // map the names
    std::set<std::string>::const_iterator it = modnames.begin();
    std::set<std::string>::const_iterator end = modnames.end();

    for (; it != end; ++it) {
        mapModuleScriptName(module, *it);
    }
}

//------------------------------------
void ScriptService::mapModuleScriptName(const ObjectScriptModulePtr &mod,
                                        const std::string &name) {
    // rewrite any previous mapping, but warn if exists
    ScriptNameToModule::const_iterator it = mScriptNameMap.find(name);

    if (it != mScriptNameMap.end())
        LOG_ERROR("ScriptService: Warning: Script named %s alread mapped to "
                  "module %s",
                  name.c_str(), mod->getName().c_str());

    mScriptNameMap[name] = mod;
}

//------------------------------------
bool ScriptService::init() { return true; }

//------------------------------------
void ScriptService::bootstrapFinished() {}

//------------------------------------
void ScriptService::shutdown() {}

/*-----------------------------------------------------*/
/*-------------------- ObjectScript -------------------*/
/*-----------------------------------------------------*/
ObjectScript::ObjectScript(int id) : mID(id) {}

//------------------------------------
ObjectScript::~ObjectScript() {}

/*-----------------------------------------------------*/
/*----------------- ObjectScriptModule ----------------*/
/*-----------------------------------------------------*/
ObjectScriptModule::ObjectScriptModule(std::string &name) {}

//------------------------------------
ObjectScriptModule::~ObjectScriptModule() {}

//-------------------------- Factory implementation
std::string ScriptServiceFactory::mName = "ScriptService";

ScriptServiceFactory::ScriptServiceFactory() : ServiceFactory() {}

const std::string &ScriptServiceFactory::getName() { return mName; }

Service *ScriptServiceFactory::createInstance(ServiceManager *manager) {
    return new ScriptService(manager, mName);
}

const uint ScriptServiceFactory::getMask() { return SERVICE_ENGINE; }

const size_t ScriptServiceFactory::getSID() { return ScriptService::SID; }
} // namespace Opde
