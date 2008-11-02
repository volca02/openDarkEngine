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

#include "RenderService.h"

#include <OgreStringConverter.h>
#include <OgreMath.h>

using namespace std;
using namespace Ogre;

namespace Opde {
	/*------------------------------------------------------*/
	/*-------------------- ObjectService -------------------*/
	/*------------------------------------------------------*/
	ObjectService::ObjectService(ServiceManager *manager, const std::string& name) : Service(manager, name), 
			mAllocatedObjects(-6144, 2048),
			mDatabaseService(NULL),
			mObjVecVerMaj(0), // Seems to be the same for all versions
			mObjVecVerMin(2),
			mSceneMgr(NULL),
			mSymNameStorage(NULL) {

	}

	//------------------------------------------------------
	ObjectService::~ObjectService() {
	}


	//------------------------------------------------------
	int ObjectService::create(int archetype) {
		int newID = getFreeID(false);
		
		_beginCreateObject(newID, archetype);
		_endCreateObject(newID);
		
		return newID;
	}
	
	//------------------------------------------------------
	int ObjectService::beginCreate(int archetype) {
		int newID = getFreeID(false);
		
		_beginCreateObject(newID, archetype);
		
		return newID;
	}
	
	//------------------------------------------------------
	void ObjectService::endCreate(int objID) {
		_endCreateObject(objID);
	}
	
	//------------------------------------------------------
	bool ObjectService::exists(int objID) {
		return mAllocatedObjects[objID];
	}
	
	
	//------------------------------------------------------
	Vector3 ObjectService::position(int objID) {
		DVariant res;

		if (mPropPosition->get(objID, "position", res)) {
			return res.toVector();
		} else
			return Vector3::ZERO;
	}
	
	//------------------------------------------------------
	Quaternion ObjectService::orientation(int objID) {
		DVariant res;
		
		if (mPropPosition->get(objID, "orientation", res)) {
			return res.toQuaternion();
		} else
			return Quaternion::IDENTITY;
		
	}

	//------------------------------------------------------
	std::string ObjectService::getName(int objID) {
		DVariant res;
		
		if (mPropSymName->get(objID, "", res)) {
			return res.toString();
		} else
			return "";
	}
	
	//------------------------------------------------------
	void ObjectService::setName(int objID, const std::string& name) {
		// First look if the name is used
		int prevusage = named(name); 
		
		if (mSymNameStorage->nameUsed(name) != 0 && prevusage != objID) {
			LOG_ERROR("Tried to set name '%s' to object %d which was alredy used by object %d", name.c_str(), objID, prevusage);
			return;
		}
		
		mPropSymName->set(objID, "", name);
	}
	
	//------------------------------------------------------
	int ObjectService::named(const std::string& name) {
		return mSymNameStorage->objectNamed(name);
	}

	//------------------------------------------------------
	void ObjectService::teleport(int id, const Vector3& pos, const Quaternion& ori, bool relative) {
		// First look if we exist
		if (relative) {
			Vector3 _pos = position(id) + pos;
			Quaternion _ori = orientation(id) + ori;
			mPropPosition->set(id, "position", _pos);
			mPropPosition->set(id, "facing", _ori);
		} else {
			mPropPosition->set(id, "position", pos);
			mPropPosition->set(id, "facing", ori);
		}
	}

	//------------------------------------------------------
	int ObjectService::addMetaProperty(int id, const std::string& mpName) {
		if (!exists(id)) {
			LOG_DEBUG("Adding MP '%s' on invalid object %d", mpName.c_str(), id);
			return 0;
		}
		
		// TODO: Object type should be reviewed as well
		int mpid = named(mpName);
		
		if (mpid == 0) {
			LOG_DEBUG("Adding invalid MP '%s' to object %d", mpName.c_str(), id);
			return 0;
		}
		
		// realize the mp addition
		mInheritService->addMetaProperty(id, mpid);
		
		return 1;
	}
	
	//------------------------------------------------------
	int ObjectService::removeMetaProperty(int id, const std::string& mpName) {
		if (!exists(id)) {
			LOG_DEBUG("Removing MP '%s' on invalid object %d", mpName.c_str(), id);
			return 0;
		}
		
		// TODO: Object type should be reviewed as well
		int mpid = named(mpName);
		
		if (mpid == 0) {
			LOG_DEBUG("Removing invalid MP '%s' to object %d", mpName.c_str(), id);
			return 0;
		}
		
		// realize the mp addition
		mInheritService->removeMetaProperty(id, mpid);
		
		return 1;
	}
	
	//------------------------------------------------------
	bool ObjectService::hasMetaProperty(int id, const std::string& mpName) {
		if (!exists(id))
			return false;

		// TODO: Object type should be reviewed as well
		int mpid = named(mpName);
		
		if (mpid == 0)
			return false;
		
		// realize the mp addition
		return mInheritService->hasMetaProperty(id, mpid);
	}
	
	//------------------------------------------------------
	void ObjectService::grow(int minID, int maxID) {
		// grow the allocated objects to have enough room for new object flags
		mAllocatedObjects.grow(minID, maxID);
		
		// grow the Properties
		mPropertyService->grow(minID, maxID);
		
		// grow the inheritors
		mInheritService->grow(minID, maxID);

		// TODO: grow the links
	}
	
	//------------------------------------------------------
	bool ObjectService::init() {
		// Builtin properties are constructed: 
		createBuiltinResources();
		
	    return true;
	}


	//------------------------------------------------------
	void ObjectService::createBuiltinResources() {
		mPropertyService = static_pointer_cast<PropertyService>(ServiceManager::getSingleton().getService("PropertyService"));
		
		// DonorType property (single integer property, built in):
		DataStoragePtr stor = new IntDataStorage();
		mPropDonorType = mPropertyService->createPropertyGroup("DonorType", "DonorType", "never", stor);
		// version of the property tag
		mPropDonorType->setChunkVersions(2, 4);

		// symbolic name builtin property
		mSymNameStorage = new SymNamePropertyStorage();
		mPropSymName = mPropertyService->createPropertyGroup("SymbolicName", "SymbolicName", "never", mSymNameStorage);
		mPropSymName->setChunkVersions(2, 17);

		// TODO: Position
	}
	
	//------------------------------------------------------
	void ObjectService::bootstrapFinished() {
		// Ensure link listeners are created
		mServiceManager->createByMask(SERVICE_OBJECT_LISTENER);

    		// Register as a database listener
		mDbCallback = new ClassCallback<DatabaseChangeMsg, ObjectService>(this, &ObjectService::onDBChange);

		mDatabaseService = static_pointer_cast<DatabaseService>(ServiceManager::getSingleton().getService("DatabaseService"));
		mDatabaseService->registerListener(mDbCallback, DBP_OBJECT);
		
		mInheritService = static_pointer_cast<InheritService>(ServiceManager::getSingleton().getService("InheritService"));
		mLinkService = static_pointer_cast<LinkService>(ServiceManager::getSingleton().getService("LinkService"));
		mPropertyService = static_pointer_cast<PropertyService>(ServiceManager::getSingleton().getService("PropertyService"));
		
		// For SceneNodes
		mPropPosition = mPropertyService->getPropertyGroup("Position");

		if (mPropPosition == NULL)
            OPDE_EXCEPT("Could not get Position property group. Not defined. Fatal", "ObjectService::bootstrapFinished");
	}
	
	//------------------------------------------------------
	void ObjectService::shutdown() {
		if (!mDatabaseService.isNull())
	        mDatabaseService->unregisterListener(mDbCallback);
	        
		mPropertyService = NULL;
		mLinkService = NULL;
		mInheritService = NULL;
		
		mPropPosition = NULL;
		mPropSymName = NULL;
		mSymNameStorage = NULL; 
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
	void ObjectService::_load(const FileGroupPtr& db, uint loadMask) {
		// Load min, max obj id, then the rest of the FilePtr as a bitmap data. Those then are unpacked to ease the use
		
		int32_t minID, maxID;
		
		FilePtr f = db->getFile("ObjVec");
		
		// Load min and max
		f->readElem(&minID, 4);
		f->readElem(&maxID, 4);
		
		LOG_DEBUG("ObjectService: ObjVec MinID %d, MaxID %d", minID, maxID);

		if ((minID & 0x07) != 0) {
			// compensate the start bits. not aligned to byte
			LOG_INFO("ObjectService: ObjVec is not aligned!");
		}
		
		// Load and unpack bitmap
		// Calculate the objvec bitmap size
		size_t bsize = f->size() - 2 * sizeof(int32_t);
		
		unsigned char* bitmap = new unsigned char[bsize + 1];
		
		for (size_t idx = 0; idx <= bsize; idx++) // fill the whole buf with zeros, even the padding at the end
			bitmap[idx] = 0;
	
		f->read(bitmap, bsize);
		
		// bit array going to be used to merge the objects (from file, and those in mem)
		BitArray fileObjs(bitmap, bsize, minID, maxID);
		
		// grow the system to allow the stored object to flow in
		grow(minID, maxID);
		
		delete[] bitmap; // not needed anymore, was copied into the fileObjs
		
		// current position in the bitmap 
		int id;
		
		int lastID = 0;
		
		/* 
		 - The damn GAM file only contains negative object ID's -

		 * Now this would not be a problem, if there was no default room introduced, with id 1
		 * Now I don't want to do any dirty handling of this issue, so all code has to be prepared for this
		*/

		// Processes one byte a time
		for(id = minID; id < maxID ; ++id) {
			if (fileObjs[id]) {
				LOG_VERBOSE("Found object ID %d", id);
				
				// object should not have existed before
				assert(!mAllocatedObjects[id]);
				
				_prepareForObject(id);
				mAllocatedObjects[id] = true;
				lastID = id;
			}
		}
		
		// Now, inform link service and property service (let them load)
		try {
			mPropertyService->load(db, fileObjs);
		} catch (BasicException& e) {
			LOG_FATAL("Exception while loading properties from mission database : %s", e.getDetails().c_str());
		}
		
		try {
			mLinkService->load(db, mAllocatedObjects); // will load MP links if those exist as well, causing inherited properties to emerge
		} catch (BasicException& e) {
			LOG_FATAL("Exception while loading links from mission database : %s", e.getDetails().c_str());
		}
		
		// Free all id's that are not in use for reuse
		for (int i = 0; i < lastID; i++) {
			// if the bitmaps is zeroed on the position, free the ID for reuse
			if (!fileObjs[i])
				freeID(i);
			// TODO: Check if the ID isn't used in properties/links!
			// TNH's purge bad object's that is
		}
		
			
		for(id = minID; id < maxID ; ++id) {
			if (fileObjs[id])
				_endCreateObject(id);
		}
	}

	//------------------------------------------------------
	void ObjectService::_clear(uint clearMask) {
		// Only bit idx 0 and 1 are used
		bool clearArchetypes = clearMask & 0x01;
		bool clearConcretes = (clearMask & 0x02) || clearArchetypes;

		if (clearMask != 0x03) { // not cleaning up the whole objsys
			int idx = mAllocatedObjects.getMinIndex();
			int max = mAllocatedObjects.getMaxIndex();
			
			if (!clearArchetypes)
				idx = 0;
			
			if (!clearConcretes)
				max = 0;				
			
			for (; idx < max; ++idx) {
				if (mAllocatedObjects[idx])
					_destroyObject(idx); // Will remove properties and links fine
			}
		} else { // Total cleanup
			mLinkService->clear();
			mPropertyService->clear();

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
	void ObjectService::_save(const FileGroupPtr& db, uint saveMask) {
		// only some values should be saved -
		// concrete objects or archetypes
		BitArray objmask(mAllocatedObjects.getMinIndex(), mAllocatedObjects.getMaxIndex());
		
		for (int id = mAllocatedObjects.getMinIndex(); id < mAllocatedObjects.getMaxIndex(); ++id) {
			DVariant v;
			
			if (mAllocatedObjects[id]) { // only gamesys object have donortype...
				if (!mPropertyService->has(id, "DonorType")) {
					if (saveMask & 0x01) // has donortype, was archetype requested?
						objmask[id] = true; // yep, so include this object
				} else {
					if (saveMask & 0x02) // has no donortype, was concrete requested?
						objmask[id] = true; // yep, include this obj
				}
			}
		}

		FilePtr ovf = db->createFile("ObjVec", mObjVecVerMaj, mObjVecVerMin);
			
		int32_t minid = objmask.getMinIndex();
		int32_t maxid = objmask.getMaxIndex();
		
		size_t siz = objmask.getByteSize();
		
		char* buf = new char[siz];
		
		objmask.fillBuffer(buf);
		
		ovf->writeElem(&minid, 4);
		ovf->writeElem(&maxid, 4);
		ovf->write(buf, siz);
		
		delete[] buf;
		
		// serialize the properties and links
		mLinkService->save(db, saveMask);
		mPropertyService->save(db, objmask);
	}
	
	//------------------------------------------------------
	void ObjectService::_beginCreateObject(int objID, int archetypeID) {
		// We'll inform property service and link service
		// NOTE: Those properties which aren't archetype->concrete inherited will be copied
		// TODO: Links of some kind should be copied as well (particle attachment, targetting obj). Rules?! To be tested in-game with no game db backup
		_prepareForObject(objID);
		
		if (!exists(archetypeID)) {
			OPDE_EXCEPT("Given archetype ID does not exist!", "ObjectService::_beginCreateObject");
		}
		
		// Use inherit service to set archetype for the new object
		mInheritService->setArchetype(objID, archetypeID);
		
		// TODO: Copy the uninheritable properties (i.e. ask each special property to do it's work)
	}
	
	//------------------------------------------------------
	void ObjectService::_endCreateObject(int objID) {
		// allocate the ID
		mAllocatedObjects[objID] = true;
		
		// Prepare the message
		ObjectServiceMsg m;
		
		m.type = OBJ_CREATED;
		m.objectID = objID;
		
		// Broadcast the change
		broadcastMessage(m);
	}
	
	//------------------------------------------------------
	void ObjectService::_destroyObject(int objID) {
		if (mAllocatedObjects[objID]) {
			// Inform LinkService and PropertyService (those are somewhat slaves of ours. Other services need to listen)
			mLinkService->objectDestroyed(objID);
			mPropertyService->objectDestroyed(objID);
			
			// Insert the id into free id's
			mAllocatedObjects[objID] = false;
			
			// Insert into free id's
			freeID(objID);
			
			// Prepare the message
			ObjectServiceMsg m;
			
			m.type = OBJ_DESTROYED;
			m.objectID = objID;
			
			// Broadcast the change
			broadcastMessage(m);
		}
	}
	
	//------------------------------------------------------
	void ObjectService::_prepareForObject(int objID) {
		// Broadcast ObjectBeginCreate message - let handlers initialize data...
		// Prepare the message
		ObjectServiceMsg m;
		
		m.type = OBJ_CREATE_STARTED;
		m.objectID = objID;
		
		// Broadcast the change
		broadcastMessage(m);
	}
	
	//------------------------------------------------------
	int ObjectService::getFreeID(bool archetype) {
		// first look into the stack of free id's
		// TODO: Grow all the related structures as well!
		if (archetype) {
			if (!mFreeArchetypeIDs.empty()) {
				int id = mFreeArchetypeIDs.top();
				mFreeArchetypeIDs.pop();
				
				return id;
			} else {
				int idx = mAllocatedObjects.getMinIndex() - 1; // New min ID

				// wanted a new id, let's grow for them!
				grow(idx - 256, mAllocatedObjects.getMaxIndex());
				
				LOG_INFO("Beware: Grew archetype id's by 256 to %d", mAllocatedObjects.getMinIndex());
				return idx;
			}
		} else {
			if (!mFreeConcreteIDs.empty()) {
				int id = mFreeConcreteIDs.top();
				mFreeConcreteIDs.pop();
				
				return id;
			} else {
				int idx = mAllocatedObjects.getMaxIndex() + 1; // New max ID

				// wanted a new id, let's grow for them!
				grow(mAllocatedObjects.getMinIndex(), idx + 256);

				LOG_INFO("Beware: Grew concrete id's by 256 to %d", mAllocatedObjects.getMinIndex());
				return idx;
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
		ConfigServicePtr cfp = static_pointer_cast<ConfigService>(ServiceManager::getSingleton().getService("ConfigService"));
		
		DVariant val;
		// Config Values: obj_min, obj_max
		
		int minID, maxID;
		
		if (cfp->getParam("obj_min", val)) {
			minID = val.toInt();
		} else {
			// a sane default (?)
			minID = -6144;
		}
		
		if (cfp->getParam("obj_max", val)) {
			maxID = val.toInt();
		} else {
			maxID = 2048;
		}
		
		mAllocatedObjects.reset(minID, maxID);
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
	    return SERVICE_DATABASE_LISTENER | SERVICE_CORE;
	}

}
