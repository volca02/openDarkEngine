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

 
#include "LinkService.h"
#include "BinaryService.h"
#include "logger.h"

using namespace std;

namespace Opde {
	
	/*----------------------------------------------------*/
	/*-------------------- LinkService -------------------*/
	/*----------------------------------------------------*/
	LinkService::LinkService(ServiceManager *manager) : Service(manager) {
	}
	
	//------------------------------------------------------
	LinkService::~LinkService() {
		mRelationNameMap.clear();
		mRelationIDMap.clear();
		mNameToFlavor.clear();
		mFlavorToName.clear();
	}
	
	//------------------------------------------------------
	void LinkService::load(DarkFileGroup* db) {
		try {
			_load(db);
		} catch (FileException &e) {
			OPDE_EXCEPT("Caught a FileException while loading the links : " + e.getDetails(), "LinkService::load");
		}
				
	}
	
	//------------------------------------------------------
	void LinkService::save(DarkFileGroup* db, uint saveMask) {
		// Iterates through all the relations. Writes the name into the Relations file, and writes the relation's data using relation->save(db, saveMask) call
		
		// I do not want to have gaps in the relations indexing, which is implicit given the record order
		int order = 1;
		
		FilePtr rels = db->createFile("Relations", mRelVMaj, mRelVMin);
		
		RelationIDMap::iterator it = mRelationIDMap.begin();
		
		for (; it != mRelationIDMap.end(); ++it, ++order) {
			if (order != it->first)
				OPDE_EXCEPT("Index order mismatch, could not continue...", "LinkService::save");
			
			// Write the relation's name
			char title[32];
			
			// get the name, and write
			const std::string& rname = it->second->getName();
			
			rname.copy(title, 32 - 1);
			
			// write
			rels->write(title, 32);
			
			// write the relation
			it->second->save(db, saveMask);
		}
	}
	
	//------------------------------------------------------
	int LinkService::nameToFlavor(const std::string& name) {
		NameToFlavor::const_iterator it = mNameToFlavor.find(name);
		
		if (it != mNameToFlavor.end()) {
			return it->second;
		} else {
			LOG_DEBUG("LinkService: Relation not found : %s", name.c_str());
			return 0; // just return 0, so no exception will be thrown
		}
	}
	
	//------------------------------------------------------
	RelationPtr LinkService::createRelation(const std::string& name, DTypeDefPtr type, bool hidden) {
		RelationPtr nr = new Relation(name, type, hidden);
		
		std::pair<RelationNameMap::iterator, bool> res = mRelationNameMap.insert(make_pair(name, nr));
		
		if (!res.second)
			OPDE_EXCEPT("Failed to insert new instance of Relation", "LinkService::createRelation");
		
		return nr;
	}
	
	//------------------------------------------------------
	void LinkService::_load(DarkFileGroup* db) {
		LOG_INFO("LinkService: Loading link definitions from file group '%s'", db->getName().c_str());
		
		// First, try to build the Relation Name -> flavor and reverse records
		/*
		The Relations chunk should be present, and the same for all File Groups
		As we do not know if something was already initialised or not, we just request mapping and see if it goes or not
		*/
		BinaryServicePtr bs = ServiceManager::getSingleton().getService("BinaryService").as<BinaryService>();
		
		FilePtr rels = db->getFile("Relations");
		
		uint count = rels->size() / 32;
		
		LOG_DEBUG("LinkService: Loading Relations map (%d items - %d size)", count, rels->size());
		
		for (int i = 1; i <= count; i++) {
			char text[32];
			
			rels->read(text, 32);
			
			// Look for relation with the specified Name
			
			// TODO: Look for the relation name. Have to find it.
			RelationNameMap::iterator rnit = mRelationNameMap.find(text);
			
			if (rnit == mRelationNameMap.end()) 
				OPDE_EXCEPT(string("Could not find relation ") + text + " predefined. Could not continue", "LinkService::_load");
			
			RelationPtr rel = rnit->second;
			
			// Request the mapping to ID
			if (!requestRelationFlavorMap(i, text, rel))
				OPDE_EXCEPT(string("Could not map relation ") + text + " to flavor. Name/ID conflict", "LinkService::_load");
			
			LOG_DEBUG("Mapped relation %s to flavor %d", text, i);
			
			// TODO: request relation ID map, must not fail
			
			// Now load the data of the relation
			LOG_DEBUG("Loading relation %s", text);
			
			rel->load(db);
		}
	}
	
	//------------------------------------------------------
	void LinkService::_clear() {
		// clear all the mappings
		mRelationIDMap.clear();
		mFlavorToName.clear();
		mNameToFlavor.clear();
		
		// clear all the relations
		RelationNameMap::iterator it = mRelationNameMap.begin();
		
		for (; it != mRelationNameMap.end(); ++it ) {
			//  request clear on the relation
			it->second->clear();
		}
	}
	
	//------------------------------------------------------
	bool LinkService::requestRelationFlavorMap(int id, const std::string& name, RelationPtr rel) {
		std::pair<FlavorToName::iterator, bool> res1 = mFlavorToName.insert(make_pair(id, name));
		
		if (!res1.second) {
			if (res1.first->second != name)
				return false;
		}
		
		std::pair<NameToFlavor::iterator, bool> res2 = mNameToFlavor.insert(make_pair(name, id));
		
		if (!res2.second) {
			if (res2.first->second != id)
				return false;
		}
		
		std::pair<RelationIDMap::iterator, bool> res3 = mRelationIDMap.insert(make_pair(id, rel));
		
		if (!res3.second) { // failed to map the relation's instance to the ID
			if (res3.first->second != rel)
				return false;
		}
		
		rel->setID(id);
		
		return true;
	}
	
	//------------------------------------------------------
	void LinkService::registerLinkListener(const std::string& relname, LinkChangeListenerPtr* listener) {
		RelationNameMap::iterator rnit = mRelationNameMap.find(relname);
			
		if (rnit == mRelationNameMap.end()) 
			OPDE_EXCEPT(string("Could not find relation named ") + relname, "LinkService::registerLinkListener");
		else
			rnit->second->registerListener(listener);
	}
			
	//------------------------------------------------------
	void LinkService::unregisterLinkListener(const std::string& relname, LinkChangeListenerPtr* listener) {
		RelationNameMap::iterator rnit = mRelationNameMap.find(relname);
			
		if (rnit == mRelationNameMap.end()) 
			OPDE_EXCEPT(string("Could not find relation named ") + relname, "LinkService::unregisterLinkListener");
		else
			rnit->second->unregisterListener(listener);
	}
	
	//------------------------------------------------------
	RelationPtr LinkService::getRelation(const std::string& name) {
		RelationNameMap::iterator rnit = mRelationNameMap.find(name);
			
		if (rnit == mRelationNameMap.end()) 
			return RelationPtr();
		else
			return rnit->second;
	}
	
	//-------------------------- Factory implementation
	std::string LinkServiceFactory::mName = "LinkService";
	
	LinkServiceFactory::LinkServiceFactory() : ServiceFactory() { 
		ServiceManager::getSingleton().addServiceFactory(this);
	};
	
	const std::string& LinkServiceFactory::getName() {
		return mName;
	}
	
	Service* LinkServiceFactory::createInstance(ServiceManager* manager) {
		return new LinkService(manager);
	}
	
}
