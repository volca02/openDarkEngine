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


#include "PropertyService.h"
#include "BinaryService.h"
#include "logger.h"
#include "ServiceCommon.h"

using namespace std;

namespace Opde {

	/*--------------------------------------------------------*/
	/*--------------------- PropertyService ------------------*/
	/*--------------------------------------------------------*/
	PropertyService::PropertyService(ServiceManager *manager, const std::string& name) : Service(manager, name) {
		// Ensure listeners are created
		mServiceManager->createByMask(SERVICE_PROPERTY_LISTENER);
	}

	// --------------------------------------------------------------------------
	PropertyService::~PropertyService() {
        if (!mDatabaseService.isNull())
            mDatabaseService->unregisterListener(mDbCallback);
	}

    // --------------------------------------------------------------------------
    bool PropertyService::init() {
        return true;
    }

	// --------------------------------------------------------------------------
	void PropertyService::bootstrapFinished() {
	    // Register as a database listener
		mDbCallback = new ClassCallback<DatabaseChangeMsg, PropertyService>(this, &PropertyService::onDBChange);

	    mDatabaseService = ServiceManager::getSingleton().getService("DatabaseService").as<DatabaseService>();
		mDatabaseService->registerListener(mDbCallback, DBP_PROPERTY);
	}

	// --------------------------------------------------------------------------
	PropertyGroupPtr PropertyService::createPropertyGroup(const std::string& name, const std::string& chunk_name, DTypeDefPtr type, uint ver_maj, uint ver_min, string inheritorName) {
		PropertyGroupPtr nr = new PropertyGroup(name, chunk_name, type, ver_maj, ver_min, inheritorName);

		std::pair<PropertyGroupMap::iterator, bool> res = mPropertyGroupMap.insert(make_pair(name, nr));

		if (!res.second)
			OPDE_EXCEPT("Failed to insert new instance of PropertyGroup, name already allocated : " + name,
				    "PropertyService::createPropertyGroup");

		LOG_INFO("PropertyService::createPropertyGroup: Created a property group %s (With chunk name %s)", name.c_str(), chunk_name.c_str());

		return nr;
	}


    //------------------------------------------------------
	void PropertyService::onDBChange(const DatabaseChangeMsg& m) {
	    if (m.change == DBC_DROPPING) {
	        _clear();
	    } else if (m.change == DBC_LOADING) {
            try {
                // Skip if target is savegame, and mission file is encountered
                if (m.dbtarget == DBT_SAVEGAME && m.dbtype == DBT_MISSION)
                    return;

                _load(m.db);
            } catch (FileException &e) {
                OPDE_EXCEPT("Caught a FileException while loading the links : " + e.getDetails(), "PropertyService::onDBChange");
            }
	    } else if (m.change == DBC_SAVING) {
            try {
                uint savemask = 0xF;

                if (m.dbtarget == DBT_SAVEGAME)
                    savemask = 0x0E; // No archetypes

                _save(m.db, savemask);

            } catch (FileException &e) {
                OPDE_EXCEPT("Caught a FileException while loading the links : " + e.getDetails(), "PropertyService::onDBChange");
            }
	    }
	}

	// --------------------------------------------------------------------------
	void PropertyService::_load(FileGroupPtr db) {
		// We just give the db to all registered groups
		PropertyGroupMap::iterator it = mPropertyGroupMap.begin();

		for (; it != mPropertyGroupMap.end(); ++it) {
			try {
				LOG_DEBUG("PropertyService: Loading property group %s", it->first.c_str());
				it->second->load(db);
			} catch (BasicException &e) {
				LOG_FATAL("PropertyService: Caught a fatal exception while loading PropertyGroup %s : %s", it->first.c_str(), e.getDetails().c_str() );
			}
		}

	}

	// --------------------------------------------------------------------------
	void PropertyService::_save(FileGroupPtr db, uint saveMask) {
		// We just give the db to all registered groups
		PropertyGroupMap::iterator it = mPropertyGroupMap.begin();

		for (; it != mPropertyGroupMap.end(); ++it) {
			it->second->save(db, saveMask);
		}
	}

	// --------------------------------------------------------------------------
	void PropertyService::_clear() {
		PropertyGroupMap::iterator it = mPropertyGroupMap.begin();

		for (; it != mPropertyGroupMap.end(); ++it) {
			it->second->clear();
		}
	}

	// --------------------------------------------------------------------------
	PropertyGroupPtr PropertyService::getPropertyGroup(const std::string& name) {
	    PropertyGroupMap::iterator it = mPropertyGroupMap.find(name);

		if (it != mPropertyGroupMap.end()) {
		    return it->second;
		} else
            return NULL;

	}
	
	// --------------------------------------------------------------------------
	bool PropertyService::has(int obj_id, const std::string& propName) {
		PropertyGroupPtr prop = getPropertyGroup(propName);
		
		if (!prop.isNull()) {
			return prop->has(obj_id);
		}
		
		return false;
	}
    
	// --------------------------------------------------------------------------
    bool PropertyService::owns(int obj_id, const std::string& propName) {
		PropertyGroupPtr prop = getPropertyGroup(propName);
		
		if (!prop.isNull()) {
			return prop->has(obj_id);
		}
		
		return false;
    }
    
	// --------------------------------------------------------------------------
    void PropertyService::set(int obj_id, const std::string& propName, const std::string& propField, const DVariant& value) {
		PropertyGroupPtr prop = getPropertyGroup(propName);
		
		if (!prop.isNull()) {
			prop->set(obj_id, propField, value);
			return;
		}
		
		LOG_ERROR("Invalid or undefined property name '%s' on call to PropertyService::set", propName.c_str());
    }
	
	// --------------------------------------------------------------------------
	DVariant PropertyService::get(int obj_id, const std::string& propName, const std::string& propField) {
				PropertyGroupPtr prop = getPropertyGroup(propName);
		
		if (!prop.isNull()) {
			return prop->get(obj_id, propField);
		}
		
		LOG_ERROR("Invalid or undefined property name '%s' on call to PropertyService::get. Returning invalid Variant", propName.c_str());
		
		return DVariant();
	}

	// --------------------------------------------------------------------------
	void PropertyService::objectDestroyed(int id) {
		PropertyGroupMap::iterator it = mPropertyGroupMap.begin();

		for (; it != mPropertyGroupMap.end(); ++it) {
			it->second->objectDestroyed(id);
		}
	}

	//-------------------------- Factory implementation
	std::string PropertyServiceFactory::mName = "PropertyService";

	PropertyServiceFactory::PropertyServiceFactory() : ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
	};

	const std::string& PropertyServiceFactory::getName() {
		return mName;
	}

	Service* PropertyServiceFactory::createInstance(ServiceManager* manager) {
		return new PropertyService(manager, mName);
	}

	const uint PropertyServiceFactory::getMask() {
	    return SERVICE_DATABASE_LISTENER;
	}
}
