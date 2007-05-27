/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2007 openDarkEngine team
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 * http://www.gnu.org/copyleft/lesser.txt.
 *****************************************************************************/
 
#include "BinaryService.h"
#include "OpdeException.h"

using namespace std;

namespace Opde {
	
	/*------------------------------------------------------*/
	/*-------------------- BinaryService -------------------*/
	/*------------------------------------------------------*/
	BinaryService::BinaryService(ServiceManager* manager) : mTypeGroups(), mEnumGroups(), Service(manager) {
	};

	//------------------------------------	
	BinaryService::~BinaryService() {
		clearAll();
	};

	//------------------------------------	
	void BinaryService::addType(const std::string& group, DTypeDef* def) {
		def->addRef();
		
		std::pair<TypeGroups::iterator, bool> grp = mTypeGroups.insert(TypeGroups::value_type(group, TypeMap()));
		
		grp.first->second.insert(TypeMap::value_type(def->name(), def));
	}

	//------------------------------------	
	void BinaryService::addEnum(const std::string& group, const std::string& name, DEnum* enm) {
		enm->addRef();
		
		std::pair<EnumGroups::iterator, bool> grp = mEnumGroups.insert(EnumGroups::value_type(group, EnumMap()));
		
		grp.first->second.insert(EnumMap::value_type(name, enm));
	}

	//------------------------------------	
	DTypeDef* BinaryService::getType(const std::string& name) const {
		// If the separator is present, split group - typename
		std::pair<string, string> path = splitPath(name);
		
		return getType(path.first, path.second);
	}

	//------------------------------------	
	DTypeDef* BinaryService::getType(const std::string& group, const std::string& name) const {
		// find the group
		TypeGroups::const_iterator git = mTypeGroups.find(group);
		
		if (git != mTypeGroups.end()) {
			// search for the type in the group
			TypeMap::const_iterator it = git->second.find(name);
			
			if (it != git->second.end()) {
				(*it).second->addRef();
				return ((*it).second);
			} else
				OPDE_EXCEPT(string("Could not find type ") + name + " in group " + group, "BinaryService::getType");
		} else 
			OPDE_EXCEPT(string("Could not find group ") + group, "BinaryService::getType");
	}
	
	//------------------------------------	
	DEnum* BinaryService::getEnum(const std::string& name) const {
		// If the separator is present, split group - typename
		std::pair<string, string> path = splitPath(name);
		
		return getEnum(path.first, path.second);
	}

	//------------------------------------	
	DEnum* BinaryService::getEnum(const std::string& group, const std::string& name) const {
		// find the group
		EnumGroups::const_iterator git = mEnumGroups.find(group);
		
		if (git != mEnumGroups.end()) {
			// search for the type in the group
			EnumMap::const_iterator it = git->second.find(name);
			
			if (it != git->second.end()) {
				(*it).second->addRef();
				return ((*it).second);
			} else
				OPDE_EXCEPT(string("Could not find enum ") + name + " in group " + group, "BinaryService::getEnum");
		} else 
			OPDE_EXCEPT(string("Could not find group ") + group, "BinaryService::getEnum");
	}

	//------------------------------------	
	void BinaryService::clear() {
		// Release all the enumerations and type definitions in all groups
		TypeGroups::iterator git = mTypeGroups.begin();
		
		for (; git != mTypeGroups.end(); git++) {
			// iterate all the dtypedefs, release and clear after this is done
			
			TypeMap::iterator it = ((TypeMap)git->second).begin();
			
			for (; it != git->second.end(); it++) {
				it->second->release();
			}
			
			git->second.clear();
		}
		
		EnumGroups::iterator egit = mEnumGroups.begin();
		
		for (; egit != mEnumGroups.end(); egit++) {
			// iterate all the dtypedefs, release and clear after this is done
			
			EnumMap::iterator it = ((EnumMap)egit->second).begin();
			
			for (; it != egit->second.end(); it++) {
				it->second->release();
			}
			
			egit->second.clear();
		}
	}
	
	//------------------------------------	
	void BinaryService::clearAll() {
		clear();
		
		// clear all the groups too
		mTypeGroups.clear();
		mEnumGroups.clear();
	}
	
	//------------------------------------	
	std::pair<std::string, std::string> BinaryService::splitPath(const std::string& path) const {
		// Split the command on the first space... make it a Command PARAMETERS
		size_t sep_pos = path.find(BINARY_GROUP_SEPARATOR);
		
		string group_part = "";
		string name_part = path;

		if (sep_pos != string::npos) { 
			// First substring to command, second to params
			group_part = path.substr(0,sep_pos);
			name_part = path.substr(sep_pos+1, path.length() - (sep_pos + 1));
			
			if (name_part.find(BINARY_GROUP_SEPARATOR) != string::npos)
				OPDE_EXCEPT("More than one separator found in path","BinaryService::splitPath");
		}	
		
		return std::pair<std::string, std::string>(group_part, name_part);
	}
	
	//-------------------------- Factory implementation
	BinaryServiceFactory::BinaryServiceFactory() { 
		ServiceName = "BinaryService";
		
		ServiceManager::getSingleton().addServiceFactory(this);
	};
	
	Service* BinaryServiceFactory::createInstance(ServiceManager* manager) {
		return new BinaryService(manager);
	}
	
}
