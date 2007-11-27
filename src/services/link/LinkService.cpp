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
#include "ServiceCommon.h"

using namespace std;

namespace Opde {

	/*----------------------------------------------------*/
	/*-------------------- LinkService -------------------*/
	/*----------------------------------------------------*/
	LinkService::LinkService(ServiceManager *manager, const std::string& name) : Service(manager, name), mDatabaseService(NULL) {
	}

	//------------------------------------------------------
	LinkService::~LinkService() {
		mRelationNameMap.clear();
		mRelationIDMap.clear();
		mNameToFlavor.clear();
		mFlavorToName.clear();

        if (!mDatabaseService.isNull())
            mDatabaseService->unregisterListener(mDbCallback);
	}

	//------------------------------------------------------
	bool LinkService::init() {
	    return true;
	}

	//------------------------------------------------------
	void LinkService::bootstrapFinished() {
		// Ensure link listeners are created
		mServiceManager->createByMask(SERVICE_LINK_LISTENER);

        // Register as a database listener
		mDbCallback = new ClassCallback<DatabaseChangeMsg, LinkService>(this, &LinkService::onDBChange);

		mDatabaseService = ServiceManager::getSingleton().getService("DatabaseService").as<DatabaseService>();
		mDatabaseService->registerListener(mDbCallback, DBP_LINK);
	}

	//------------------------------------------------------
	void LinkService::onDBChange(const DatabaseChangeMsg& m) {
	    if (m.change == DBC_DROPPING) {
	        _clear();
	    } else if (m.change == DBC_LOADING) {
            try {
                // Skip if target is savegame, and mission file is encountered
                if (m.dbtarget == DBT_SAVEGAME && m.dbtype == DBT_MISSION)
                    return;

                _load(m.db);
            } catch (FileException &e) {
                OPDE_EXCEPT("Caught a FileException while loading the links : " + e.getDetails(), "LinkService::onDBChange");
            }
	    } else if (m.change == DBC_SAVING) {
            try {
                uint savemask = 0xF;

                if (m.dbtarget == DBT_SAVEGAME)
                    savemask = 0x0E; // No archetypes

                _save(m.db, savemask);

            } catch (FileException &e) {
                OPDE_EXCEPT("Caught a FileException while loading the links : " + e.getDetails(), "LinkService::onDBChange");
            }
	    }
	}

	//------------------------------------------------------
	void LinkService::_save(FileGroupPtr db, uint saveMask) {
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
			LOG_ERROR("LinkService: Relation not found : %s", name.c_str());
			return 0; // just return 0, so no exception will be thrown
		}
	}
	
	//------------------------------------------------------
	std::string LinkService::flavorToName(int flavor) {
		FlavorToName::const_iterator it = mFlavorToName.find(flavor);

		if (it != mFlavorToName.end()) {
			return it->second;
		} else {
			LOG_ERROR("LinkService: Relation not found by flavor : %d", flavor);
			return ""; // just return empty string
		}
	}

	//------------------------------------------------------
	RelationPtr LinkService::createRelation(const std::string& name, DTypeDefPtr type, bool hidden) {
		if (name.substr(0,1) == "~")
			OPDE_EXCEPT("Name conflict: Relation can't use ~ character as the first one, it's reserved for inverse relations. Conflicting name: " + name, "LinkService::createRelation");
		
		std::string inverse = "~" + name;
		
		RelationPtr nr = new Relation(name, type, false, hidden);
		RelationPtr nrinv = new Relation(inverse, type, true, hidden);

		// Assign inverse relations...
		nr->setInverseRelation(nrinv.ptr());
		nrinv->setInverseRelation(nr.ptr());

		// insert both
		std::pair<RelationNameMap::iterator, bool> res = mRelationNameMap.insert(make_pair(name, nr));

		if (!res.second)
			OPDE_EXCEPT("Failed to insert new instance of Relation", "LinkService::createRelation");
			
		// Inverse relation now
		res = mRelationNameMap.insert(make_pair(inverse, nrinv));

		if (!res.second)
			OPDE_EXCEPT("Failed to insert new instance of Relation", "LinkService::createRelation");

		return nr;
	}

	//------------------------------------------------------
	void LinkService::_load(FileGroupPtr db) {
		LOG_INFO("LinkService: Loading link definitions from file group '%s'", db->getName().c_str());

		// First, try to build the Relation Name -> flavor and reverse records
		/*
		The Relations chunk should be present, and the same for all File Groups
		As we do not know if something was already initialised or not, we just request mapping and see if it goes or not
		*/
		BinaryServicePtr bs = ServiceManager::getSingleton().getService("BinaryService").as<BinaryService>();

		FilePtr rels = db->getFile("Relations");

		int count = rels->size() / 32;

		LOG_DEBUG("LinkService: Loading Relations map (%d items - %d size)", count, rels->size());

		for (int i = 1; i <= count; i++) {
			char text[32];

			rels->read(text, 32);

			std::string stxt = text;

			if (stxt.substr(0,1) == "~")
				OPDE_EXCEPT("Conflicting name. Character ~ is reserved for inverse relations. Conflicting name : " + stxt, "LinkService::_load");

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
			
			std::string inverse = "~" + stxt;
			
			// --- Assign the inverse relation as well here:
			rnit = mRelationNameMap.find(inverse);

			if (rnit == mRelationNameMap.end())
				OPDE_EXCEPT(string("Could not find inverse relation ") + inverse + " predefined. Could not continue", "LinkService::_load");

			RelationPtr irel = rnit->second;

			// Request the mapping to ID
			if (!requestRelationFlavorMap(-i, inverse, irel))
				OPDE_EXCEPT(string("Could not map inverse relation ") + inverse + " to flavor. Name/ID conflict", "LinkService::_load");

			LOG_DEBUG("Mapped relation pair %s, %s to flavor %d, %d", text, inverse.c_str(), i, -i);

			// TODO: request relation ID map, must not fail

			// Now load the data of the relation
			LOG_DEBUG("Loading relation %s", text);

			try {
				rel->load(db); // only normal relation is loaded. Inverse is mapped automatically
			} catch (BasicException &e) {
				LOG_FATAL("LinkService: Caught a fatal exception while loading Relation %s : %s", text, e.getDetails().c_str() );
			}

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
	RelationPtr LinkService::getRelation(const std::string& name) {
		RelationNameMap::iterator rnit = mRelationNameMap.find(name);

		if (rnit == mRelationNameMap.end())
			return RelationPtr(); // return NULL pointer
		else
			return rnit->second;
	}

	//------------------------------------------------------
	RelationPtr LinkService::getRelation(int flavor) {
		RelationIDMap::iterator rnit = mRelationIDMap.find(flavor);

		if (rnit == mRelationIDMap.end())
			return RelationPtr(); // return NULL pointer
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
		return new LinkService(manager, mName);
	}

	const uint LinkServiceFactory::getMask() {
	    return SERVICE_DATABASE_LISTENER;
	}

}
