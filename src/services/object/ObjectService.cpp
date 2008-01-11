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
 *	$Id$
 *
 *****************************************************************************/

#include "config.h"
#include "ServiceCommon.h"
#include "ObjectService.h"
#include "ConfigService.h"
#include "logger.h"

using namespace std;

namespace Opde {

	/*------------------------------------------------------*/
	/*-------------------- ObjectService -------------------*/
	/*------------------------------------------------------*/
	ObjectService::ObjectService(ServiceManager *manager, const std::string& name) : Service(manager, name), 
			mDatabaseService(NULL),
			mObjVecVerMaj(0), // Seems to be the same for all versions
			mObjVecVerMin(2) {
	}

	//------------------------------------------------------
	ObjectService::~ObjectService() {
            if (!mDatabaseService.isNull())
	        mDatabaseService->unregisterListener(mDbCallback);
	}


	//------------------------------------------------------
	int ObjectService::createObject(int archetype) {
		int newID = getFreeID(false);
		
		_beginCreateObject(newID, archetype);
		_endCreateObject(newID);
		
		return newID;
	}
	
	//------------------------------------------------------
	int ObjectService::beginCreateObject(int archetype) {
		int newID = getFreeID(false);
		
		_beginCreateObject(newID, archetype);
		
		return newID;
	}
	
	//------------------------------------------------------
	void ObjectService::endCreateObject(int objID) {
		_endCreateObject(objID);
	}


	//------------------------------------------------------
	bool ObjectService::init() {
	    return true;
	}

	//------------------------------------------------------
	void ObjectService::bootstrapFinished() {
		// Ensure link listeners are created
		mServiceManager->createByMask(SERVICE_OBJECT_LISTENER);

    		// Register as a database listener
		mDbCallback = new ClassCallback<DatabaseChangeMsg, ObjectService>(this, &ObjectService::onDBChange);

		mDatabaseService = ServiceManager::getSingleton().getService("DatabaseService").as<DatabaseService>();
		mDatabaseService->registerListener(mDbCallback, DBP_LINK);
		
		mInheritService = ServiceManager::getSingleton().getService("InheritService").as<InheritService>();
		mLinkService = ServiceManager::getSingleton().getService("LinkService").as<LinkService>();
		mPropertyService = ServiceManager::getSingleton().getService("PropertyService").as<PropertyService>();
	}

	//------------------------------------------------------
	void ObjectService::onDBChange(const DatabaseChangeMsg& m) {
	    if (m.change == DBC_DROPPING) {
		// TODO: Clear the object mapping, broadcast the 'object are being released' change
		_clear(0x03);
	    } else if (m.change == DBC_LOADING) {
		// TODO: Load the object mapping bitmap (16 categories, although just 0 abstract, 1 concrete are in use)
		// NOTE: Just the given range is used (based on the bitmap of the file - bit 0 abstract objs, bit 1 concrete objs)
		// NOTE: Upon loading a savegame, skip the mission file as the property and link services do it
		// NOTE: A good idea would probably be to introduce loadmask (for quicksaves - archetypes would stay intact f.e.)
		// NOTE: Original dark always saves complete bitmap, although the mission loadout will overwrite the part which is concrete
		// NOTE: Use the same config parameter name here to avoid confusion (If needed, that is): obj_min, obj_max (ID's)
		// NOTE: Two pass search for free ID's would be nice: first look into the stack of free id's, then iterate the object bitmap and find first zero
		uint loadMask = 0x03;
		
		// No loading of objsys in mission if savegame is the target...
		if (m.dbtarget == DBT_SAVEGAME && m.dbtype == DBT_MISSION)
			return;
		
		// Not loading an empty mission, ignore the concretes of the GAMESYS!
		if (m.dbtype == DBT_GAMESYS && m.dbtarget != DBT_GAMESYS) {
			loadMask = 0x01;
		}
		
		if (m.dbtype == DBT_MISSION || m.dbtype == DBT_SAVEGAME)
			loadMask = 0x02;
		
		_load(m.db, loadMask);
	    } else if (m.change == DBC_SAVING) {
			// Write the object allocation bitmap, whole (as original dark does it)
			uint savemask = 0x03; // whole system (archetypes and concretes)
			
			if (m.dbtarget == DBT_GAMESYS) 
				savemask = 0x01;
				
			if (m.dbtarget == DBT_MISSION) 
				savemask = 0x02;
				
			_save(m.db, savemask);
	    }
	}
	
	//------------------------------------------------------
	void ObjectService::_load(FileGroupPtr db, uint loadMask) {
		// Load min, max obj id, then the rest of the FilePtr as a bitmap data. Those then are unpacked to ease the use
		
		int32_t minID, maxID;
		
		bool archetypeOnly = (loadMask==0x01);
			
		FilePtr f = db->getFile("ObjVec");
		
		// Load min and max
		f->readElem(&minID, 4);
		f->readElem(&maxID, 4);
		
		// set the min and max values. Only set min id if loading archetype, and reverse
		if (loadMask & 0x01)
			mMinID = minID;
			
		if (loadMask & 0x02)
			mMaxID = maxID;
		
		// Load and unpack bitmap
		// Calculate the objvec bitmap size
		size_t bsize = mMaxID - mMinID;
		
		if ((bsize & 0x07) != 0) {// alignment of size needed
			bsize += (8 - (bsize & 0x07));
		}

		char* bitmap = new char[bsize];
		
		// act position in the bitmap
		size_t s = 0;
		
		if (loadMask == 0x02) { // only concrete
			// recalc the s to start on obj. id. 0
			s = -minID >> 3;
			
			if ((minID & 0x07) != 0) {
				// compensate the start bits. not aligned to byte
				LOG_INFO("ObjectService: ObjVec is not aligned!");
			}
		}
		
		int lastID = 0;
		
		// Processes one byte a time
		while (s < bsize) {
			char b = bitmap[s];
			
			int32_t id = s * 8 + minID;
			
			if (id >= 0 && archetypeOnly)
				break;
			
			while (b) {
				if (b & 1) {
					_endCreateObject(id);
					lastID = id;
				}
				
				b >>= 1;
				id++;
			}
			
			// Next byte, please!
		}
		
		// Free all id's that are not in use for reuse
		for (int i = 0; i < lastID; i++) {
			// if the bitmaps is zeroed on the position, free the ID for reuse
			if ((bitmap[(i - minID) >> 3] & (1 << (i & 0x07))) == 0)
				freeID(i);
			
			// TODO: Check if the ID isn't used in properties/links!
			// TNH's purge bad object's that is
		}
		
		// Done loading the allocation, dealloc the bitmap
		delete[] bitmap;
	}

	//------------------------------------------------------
	void ObjectService::_clear(uint clearMask) {
		// Only bit idx 0 and 1 are used
		bool clearArchetypes = clearMask & 0x01;
		bool clearConcretes = (clearMask & 0x02) || clearArchetypes;

		if (clearMask != 0x03) { // not cleaning up the whole objsys
			ObjectAllocation::iterator it = mAllocatedObjects.begin();
			
			while (it != mAllocatedObjects.end()) {
				ObjectAllocation::iterator cur = it++;
				
				if ((*cur < 0 && clearArchetypes) || (*cur > 0 && clearConcretes))
					_destroyObject(*cur); // TODO: Will this work I wonder? The iterator should still be valid
			}
		} else { // Total cleanup
			resetMinMaxID();
			mAllocatedObjects.clear();
			
			while (mFreeArchetypeIDs.size())
				mFreeArchetypeIDs.pop();
				
			while (mFreeConcreteIDs.size())
				mFreeConcreteIDs.pop();
			
			// Broadcast the total cleanup
			ObjectServiceMsg m;
			
			m.type = OBJ_SYSTEM_CLEARED;
			broadcastMessage(m);
		}
	}
	
	//------------------------------------------------------
	void ObjectService::_save(FileGroupPtr db, uint saveMask) {
		// If the min or max values are not aligned, then align them (min should get lower, max higher!)
		// By aligning, I mean that 0 will get to bit index 0 of some byte (so the low 3 bits of the min value should be 0)
		// Dark always saves the full bitmap, it seems. Let's do it the same
		int err = mMinID & 0x07;
		
		if (err != 0) {
			
			mMinID -= 8 - err;
		}

		size_t bsize = mMaxID - mMinID;
		
		if ((bsize & 0x07) != 0) {// alignment of size needed
			bsize += (8 - (bsize & 0x07));
		}
		
		char* bitmap = new char[bsize];
		memset(bitmap, 0, bsize);
		
		ObjectAllocation::const_iterator it = mAllocatedObjects.begin();
		
		while (it != mAllocatedObjects.end()) {
			// set the appropriate bit
			int id = *it;
			
			bitmap[id >> 3] |= 1 << (id & 0x07);
		}
		
		// create the ObjVec in the db
		FilePtr ovf = db->createFile("ObjVec", mObjVecVerMaj, mObjVecVerMin);
		
		ovf->writeElem(&mMinID, 4);
		ovf->writeElem(&mMaxID, 4);
		ovf->write(bitmap, bsize);
		
		delete[] bitmap;
	}
	
	//------------------------------------------------------
	void ObjectService::_beginCreateObject(int objID, int archetypeID) {
		// We'll inform property service and link service
		// NOTE: Those properties which aren't archetype->concrete inherited will be copied
		// TODO: Links of some kind should be copied as well (particle attachment, targetting obj). Rules?! To be tested in-game with no game db backup
		if (!exists(archetypeID)) {
			OPDE_EXCEPT("Given archetype ID does not exist!", "ObjectService::_beginCreateObject");
		}
		
		// Use inherit service to set archetype for the new object
		mInheritService->setArchetype(objID, archetypeID);
		
		// TODO: Copy the uninheritable properties
	}
	
	//------------------------------------------------------
	void ObjectService::_endCreateObject(int objID) {
		// allocate the ID
		mAllocatedObjects.insert(objID);
		
		// Prepare the message
		ObjectServiceMsg m;
		
		m.type = OBJ_CREATED;
		m.id = objID;
		
		// Broadcast the change
		broadcastMessage(m);
	}
	
	//------------------------------------------------------
	void ObjectService::_destroyObject(int objID) {
		// remove from set
		ObjectAllocation::iterator it = mAllocatedObjects.find(objID);
		
		if (it != mAllocatedObjects.end()) {
			// TODO: Inform link and property service first(?)
			
			// Insert the id into free id's
			mAllocatedObjects.erase(it);
			
			// Insert into free id's
			freeID(objID);
			
			// Inform LinkService and PropertyService (those are somewhat slaves of ours. Other services need to listen)
			mLinkService->objectDestroyed(objID);
			mPropertyService->objectDestroyed(objID);
			
			// Prepare the message
			ObjectServiceMsg m;
			
			m.type = OBJ_DESTROYED;
			m.id = objID;
			
			// Broadcast the change
			broadcastMessage(m);
		}
	}
	
	//------------------------------------------------------
	int ObjectService::getFreeID(bool archetype) {
		// first look into the stack of free id's
		if (archetype) {
			if (mFreeArchetypeIDs.size() > 0) {
				int id = mFreeArchetypeIDs.top();
				mFreeArchetypeIDs.pop();
				
				return id;
			} else {
				return --mMinID; // New minimal ID objects
			}
		} else {
			if (mFreeConcreteIDs.size() > 0) {
				int id = mFreeConcreteIDs.top();
				mFreeConcreteIDs.pop();
				
				return id;
			} else {
				return ++mMaxID; // New minimal ID objects
			}
		}
	}
	
	//------------------------------------------------------
	void ObjectService::freeID(int objID) {
		if (objID < 0) {
			mFreeArchetypeIDs.push(objID);
		} else {
			mFreeConcreteIDs.push(objID);
		}
	}
	
	//------------------------------------------------------
	void ObjectService::resetMinMaxID() {
		ConfigServicePtr cfp = ServiceManager::getSingleton().getService("ConfigService").as<ConfigService>();
		
		DVariant val;
		// Config Values: obj_min, obj_max
		
		if (cfp->getParam("obj_min", val)) {
			mMinID = val.toInt();
		} else {
			// a sane default (?)
			mMinID = -6144;
		}
		
		if (cfp->getParam("obj_min", val)) {
			mMinID = val.toInt();
		} else {
			mMaxID = 2048;
		}
	}
	
	
	//-------------------------- Factory implementation
	std::string ObjectServiceFactory::mName = "ObjectService";

	ObjectServiceFactory::ObjectServiceFactory() : ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
	};

	const std::string& ObjectServiceFactory::getName() {
		return mName;
	}

	Service* ObjectServiceFactory::createInstance(ServiceManager* manager) {
		return new ObjectService(manager, mName);
	}

	const uint ObjectServiceFactory::getMask() {
	    return SERVICE_DATABASE_LISTENER;
	}

}
