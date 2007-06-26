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

using namespace std;

namespace Opde {
	
	/*--------------------------------------------------------*/
	/*--------------------- PropertyService ------------------*/
	/*--------------------------------------------------------*/
	PropertyService::PropertyService(ServiceManager *manager) : Service(manager) {
		
	}
	
	// --------------------------------------------------------------------------
	PropertyService::~PropertyService() {
		
	}
	
	// --------------------------------------------------------------------------
	PropertyGroupPtr PropertyService::createPropertyGroup(const std::string& name, const std::string& chunk_name, DTypeDefPtr type, uint ver_maj, uint ver_min) {
		PropertyGroupPtr nr = new PropertyGroup(name, chunk_name, type, ver_maj, ver_min);
		
		std::pair<PropertyGroupMap::iterator, bool> res = mPropertyGroupMap.insert(make_pair(name, nr));
		
		if (!res.second)
			OPDE_EXCEPT("Failed to insert new instance of PropertyGroup, name already allocated : " + name, 
				    "PropertyService::createPropertyGroup");
		
		return nr;
	}
	
	// --------------------------------------------------------------------------
	void PropertyService::load(FileGroup* db) {
		// We just give the db to all registered groups
		PropertyGroupMap::iterator it = mPropertyGroupMap.begin();
		
		for (; it != mPropertyGroupMap.end(); ++it) {
			it->second->load(db);
		}
	}
			
	// --------------------------------------------------------------------------
	void PropertyService::save(FileGroup* db, uint saveMask) {
		// We just give the db to all registered groups
		// We just give the db to all registered groups
		PropertyGroupMap::iterator it = mPropertyGroupMap.begin();
		
		for (; it != mPropertyGroupMap.end(); ++it) {
			it->second->save(db, saveMask);
		}
	}
	
	// --------------------------------------------------------------------------
	void PropertyService::clear() {
		// We just give the db to all registered groups
		// We just give the db to all registered groups
		PropertyGroupMap::iterator it = mPropertyGroupMap.begin();
		
		for (; it != mPropertyGroupMap.end(); ++it) {
			it->second->clear();
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
		return new PropertyService(manager);
	}
}
