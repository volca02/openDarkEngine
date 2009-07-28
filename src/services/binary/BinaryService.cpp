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
 *	  $Id$
 *
 *****************************************************************************/


#include "BinaryService.h"
#include "OpdeException.h"
#include "logger.h"
#include "ConsoleBackend.h"
#include "ServiceCommon.h"

using namespace std;

namespace Opde {

	/*------------------------------------------------------*/
	/*-------------------- BinaryService -------------------*/
	/*------------------------------------------------------*/
	template<> const size_t ServiceImpl<BinaryService>::SID = __SERVICE_ID_BINARY;
	
	BinaryService::BinaryService(ServiceManager* manager, const std::string& name) : ServiceImpl<BinaryService>(manager, name), mTypeGroups(), mEnumGroups() {
		Opde::ConsoleBackend::getSingleton().registerCommandListener("dtdesc", dynamic_cast<ConsoleCommandListener*>(this));
		Opde::ConsoleBackend::getSingleton().setCommandHint("dtdesc", "Describe a dynamic type (param : dtype path)");
	};

	//------------------------------------
	BinaryService::~BinaryService() {
		clearAll();
	};


	//------------------------------------
	bool BinaryService::init() {
	    return true;
	}

	//------------------------------------
	void BinaryService::addType(const std::string& group, const DTypeDefPtr& def) {
		LOG_DEBUG("BinaryService: Inserting type %s/%s", group.c_str(), def->name().c_str());

		std::pair<TypeGroups::iterator, bool> grp = mTypeGroups.insert(TypeGroups::value_type(group, TypeMap()));

		grp.first->second.insert(TypeMap::value_type(def->name(), def));
	}

	//------------------------------------
	void BinaryService::addEnum(const std::string& group, const std::string& name, const DEnumPtr& enm) {
		LOG_DEBUG("BinaryService: Inserting enum %s/%s", group.c_str(), name.c_str());

		std::pair<EnumGroups::iterator, bool> grp = mEnumGroups.insert(EnumGroups::value_type(group, EnumMap()));

		grp.first->second.insert(EnumMap::value_type(name, enm));
	}

	//------------------------------------
	DTypeDefPtr BinaryService::getType(const std::string& name) const {
		// If the separator is present, split group - typename
		std::pair<string, string> path = splitPath(name);

		return getType(path.first, path.second);
	}

	//------------------------------------
	DTypeDefPtr BinaryService::getType(const std::string& group, const std::string& name) const {
		// find the group
		TypeGroups::const_iterator git = mTypeGroups.find(group);

		if (git != mTypeGroups.end()) {
			// search for the type in the group
			TypeMap::const_iterator it = git->second.find(name);

			if (it != git->second.end()) {
				return ((*it).second);
			} else
				OPDE_EXCEPT(string("Could not find type ") + name + " in group " + group, "BinaryService::getType");
		} else
			OPDE_EXCEPT(string("Could not find group ") + group, "BinaryService::getType");
	}

	//------------------------------------
	DEnumPtr BinaryService::getEnum(const std::string& name) const {
		// If the separator is present, split group - typename
		std::pair<string, string> path = splitPath(name);

		return getEnum(path.first, path.second);
	}

	//------------------------------------
	DEnumPtr BinaryService::getEnum(const std::string& group, const std::string& name) const {
		// find the group
		EnumGroups::const_iterator git = mEnumGroups.find(group);

		if (git != mEnumGroups.end()) {
			// search for the type in the group
			EnumMap::const_iterator it = git->second.find(name);

			if (it != git->second.end()) {
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
			git->second.clear();
		}

		EnumGroups::iterator egit = mEnumGroups.begin();

		for (; egit != mEnumGroups.end(); egit++) {
			// iterate all the dtypedefs, release and clear after this is done
			egit->second.clear();
		}
	}

	//------------------------------------
	void BinaryService::clearAll() {
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

	void BinaryService::commandExecuted(std::string command, std::string parameters) {
		if (command == "dtdesc") {
			DTypeDefPtr type;

			try {
				type = getType(parameters);
			} catch (BasicException) {
				LOG_ERROR("Type not found : %s", parameters.c_str());
				return;
			}


			DTypeDef::const_iterator it = type->begin();

			// describe the type. Iterate through the fields, write field name, size, type
			for (; it != type->end(); it++) {
				LOG_INFO("[%d] %s (%d)",it->offset, it->name.c_str(), it->type->size());
			}
		} else LOG_ERROR("Command %s not understood by BinaryService", command.c_str());
	}


	//-------------------------- Factory implementation
	std::string BinaryServiceFactory::mName = "BinaryService";

	BinaryServiceFactory::BinaryServiceFactory() : ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
	};

	const std::string& BinaryServiceFactory::getName() {
		return mName;
	}
	
	const uint BinaryServiceFactory::getMask() {
		return SERVICE_CORE;
	}
	
	const size_t BinaryServiceFactory::getSID() {
		return BinaryService::SID;
	}

	Service* BinaryServiceFactory::createInstance(ServiceManager* manager) {
		return new BinaryService(manager, mName);
	}

}
