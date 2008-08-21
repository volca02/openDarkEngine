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

#include "PropertyGroup.h"
#include "InheritService.h"

using namespace std;

namespace Opde {

	// --------------------------------------------------------------------------
	PropertyGroup::PropertyGroup(PropertyService* owner, const std::string& name, const std::string& chunk_name, 
								 const DataStoragePtr& storage, std::string inheritorName) :
			mName(name),
			mChunkName(chunk_name),
			mVerMaj(1),
			mVerMin(1),
			mPropertyStorage(NULL),
			mOwner(owner),
			mBuiltin(false) {

		// Find the inheritor by the name, and assign too
		InheritServicePtr inhs = ServiceManager::getSingleton().getService("InheritService").as<InheritService>();
		mInheritor = inhs->createInheritor(inheritorName);

		// And as a final step, register as inheritor listener
		Inheritor::ListenerPtr cil = new ClassCallback<InheritValueChangeMsg, PropertyGroup>(this, &PropertyGroup::onInheritChange);

        mInheritorListenerID = mInheritor->registerListener(cil);
        
        // create the property storage for this PropertyGroup
        mPropertyStorage = storage;
	}

	// --------------------------------------------------------------------------
	PropertyGroup::~PropertyGroup() {
		clear();

		if (! mInheritor.isNull())
			mInheritor->unregisterListener(mInheritorListenerID);
	}
	
	// --------------------------------------------------------------------------
	void PropertyGroup::setPropertyStorage(const DataStoragePtr& newStorage) {
		// see if we had any data in the current
		if (!mPropertyStorage->isEmpty()) {
			LOG_ERROR("Property storage replacement for %s: Previous property storage had some data. This could mean something bad could happen...", mName.c_str());
		}
		
		mPropertyStorage = newStorage;
	}
	
	// --------------------------------------------------------------------------
	void PropertyGroup::load(const FileGroupPtr& db) {
		// Open the chunk specified by "P$" + mChunkName
		FilePtr fprop;

		string pchn = "P$" + mChunkName;

		try {
			fprop = db->getFile(pchn);
			
			const DarkDBChunkHeader &hdr = db->getFileHeader(pchn);
			
			// compare the versions, log differences
			if (hdr.version_high != mVerMaj || hdr.version_low != mVerMin) {
				LOG_ERROR("Property group %s version mismatch : %d.%d expected, %d.%d encountered", pchn.c_str(), mVerMaj, mVerMin, hdr.version_high, hdr.version_low);
			}
		} catch (BasicException& e) {
			LOG_FATAL("PropertyGroup::load : Could not find the property chunk %s", pchn.c_str());
			return;
		}

		// Can't calculate the count of the properties, as they can have any size
		// load. Each record has: OID, size (32 bit uint's)
		int id = 0xDEADBABE;
		uint32_t size;
		
		while (!fprop->eof()) {
			// load the id
			fprop->readElem(&id, sizeof(uint32_t));
			
			size = fprop->tell();
			
			// Use property storage to load the property
			if (mPropertyStorage->readFromFile(fprop, id, true)) {
				_addProperty(id);
			} else {
				LOG_ERROR("There was an error loading property %s for object %d. Property was not loaded", mName.c_str(), id);
			}
		}
	}


	// --------------------------------------------------------------------------
	void PropertyGroup::save(const FileGroupPtr& db, const BitArray& objMask) {
		// Open the chunk specified by "P$" + mChunkName
		FilePtr fprop;

		string pchn = "P$" + mChunkName;

		try {
			fprop = db->createFile(pchn, mVerMaj, mVerMin);
		} catch (BasicException& e) {
			LOG_FATAL("PropertyGroup::save : Could not create property chunk %s", pchn.c_str());
			return;
		}

		// Can't calculate the count of the properties, as they can have any size
		// load. Each record has: OID, size (32 bit uint's)
		IntIteratorPtr idit = mPropertyStorage->getAllStoredObjects();

		while (!idit->end()) {
			int id = idit->next();

			if (!objMask.get(id))
				continue;

			if (!mPropertyStorage->writeToFile(fprop, id, true))
				LOG_ERROR("There was an error writing property %s for object %d. Property was not loaded", mName.c_str(), id);
		}
	}

	// --------------------------------------------------------------------------
	void PropertyGroup::clear() {
		PropertyChangeMsg msg;

		msg.change = PROP_GROUP_CLEARED;
		msg.objectID = 0;

		broadcastMessage(msg);

		mPropertyStorage->clear();
		mInheritor->clear();
	}

	// --------------------------------------------------------------------------
	bool PropertyGroup::createProperty(int obj_id) {
		if (mPropertyStorage->create(obj_id)) {
			_addProperty(obj_id);
			
			return true;
		}
		
		return false;
	}

	// --------------------------------------------------------------------------
	bool PropertyGroup::removeProperty(int obj_id) {
		if (mPropertyStorage->destroy(obj_id)) {
			mInheritor->setImplements(obj_id, false);
			
			return true;
		}
		
		return false;
		
	}

	// --------------------------------------------------------------------------
	bool PropertyGroup::cloneProperty(int obj_id, int src_id) {
		bool had = false;
		
		if (mPropertyStorage->has(obj_id)) {
			// delete first
			mPropertyStorage->destroy(obj_id);
			had = true;
		}
		
		
		if (mPropertyStorage->clone(src_id, obj_id))  {
			// went ok, the target now includes the property didn't previously
			if (!had)
				_addProperty(obj_id);
				
			return true;
		} else {
			return false;
		}
	}
	
	// --------------------------------------------------------------------------
	bool PropertyGroup::set(int id, const std::string& field, const DVariant& value) {
		return mPropertyStorage->setField(id, field, value);
	}

	// --------------------------------------------------------------------------
	bool PropertyGroup::get(int id, const std::string& field, DVariant& target) {
		int effID = _getEffectiveObject(id);
		return mPropertyStorage->getField(effID, field, target);
	}

	// --------------------------------------------------------------------------
	void PropertyGroup::_addProperty(int objID) {
        mInheritor->setImplements(objID, true);
	}

    // --------------------------------------------------------------------------
    void PropertyGroup::onInheritChange(const InheritValueChangeMsg& msg) {
            // Consult the inheritor value change, and build a property change message

            /* The broadcast of the property change is not done directly in the methods above but here.
            The reason for this is that only the inheritor knows the real character of the change, and the objects that the change inflicted
            */

            PropertyChangeMsg pmsg;
            pmsg.objectID = msg.objectID;

            switch (msg.change) {
                case INH_VAL_ADDED: // Property was added to an object
                    pmsg.change = PROP_ADDED;
                    break;
                case INH_VAL_CHANGED: // property changed inherit src or value
                    pmsg.change = PROP_CHANGED;
                    break;
                case INH_VAL_REMOVED: // property does not exist any more on the object (and not inherited)
                    pmsg.change = PROP_REMOVED;
                    break;
                default:
                    return;
            }

            broadcastMessage(pmsg);
    }
    
    // --------------------------------------------------------------------------
    void PropertyGroup::objectDestroyed(int id) {
    	removeProperty(id);
    }

	// --------------------------------------------------------------------------
	DataFieldDescIteratorPtr PropertyGroup::getFieldDescIterator(void) {
		return mPropertyStorage->getFieldDescIterator();
	}
}

