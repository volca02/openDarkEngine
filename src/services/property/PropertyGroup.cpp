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

#include "PropertyGroup.h"
#include "InheritService.h"

using namespace std;

namespace Opde {

	// --------------------------------------------------------------------------
	PropertyGroup::PropertyGroup(const std::string& name, const std::string& chunk_name, DTypeDefPtr type, uint ver_maj, uint ver_min, string inheritorName) :
			mName(name),
			mChunkName(chunk_name),
			mType(type),
			mVerMaj(ver_maj),
			mVerMin(ver_min) {

		// Find the inheritor by the name, and assign too
		InheritServicePtr inhs = ServiceManager::getSingleton().getService("InheritService").as<InheritService>();
		mInheritor = inhs->createInheritor(inheritorName);

		// And as a final step, register as inheritor listener
		Inheritor::ListenerPtr cil = new ClassCallback<InheritValueChangeMsg, PropertyGroup>(this, &PropertyGroup::onInheritChange);

        mInheritorListenerID = mInheritor->registerListener(cil);
	}

	// --------------------------------------------------------------------------
	PropertyGroup::~PropertyGroup() {
		clear();

		if (! mInheritor.isNull())
			mInheritor->unregisterListener(mInheritorListenerID);
	}

	// --------------------------------------------------------------------------
	PropertyDataPtr PropertyGroup::getData(int obj_id) {
		PropertyStore::const_iterator it = mPropertyStore.find(_getEffectiveObject(obj_id));

		if (it != mPropertyStore.end())
			return it->second;
		else
			LOG_ERROR("PropertyGroup::getData : Property for object ID %d was not found in group %s", obj_id, mName.c_str());
			return NULL;
	}

	// --------------------------------------------------------------------------
	void PropertyGroup::load(FileGroupPtr db) {
		// Open the chunk specified by "P$" + mChunkName
		FilePtr fprop;

		string pchn = "P$" + mChunkName;

		try {
			fprop = db->getFile(pchn);
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
			// Load the size
			fprop->readElem(&size, sizeof(uint32_t));

			if (id == 0)
				LOG_ERROR("PropertyGroup: P$%s : Obj ID == 0", mChunkName.c_str());

			LOG_VERBOSE("PropertyGroup: P$%s : Loading property %s for obj %d (Sizes: real %d, type %d)", mChunkName.c_str(), mName.c_str(), id, size, mType->size());
			// create the property
			PropertyDataPtr prop = new PropertyData(id, mType, fprop, size, mUseDataCache);

			if (!_addProperty(prop))
				LOG_ERROR("PropertyGroup: P$%s : Cannot add property for obj %d (already there)", mChunkName.c_str(), id);
		}
	}


	// --------------------------------------------------------------------------
	void PropertyGroup::save(FileGroupPtr db, uint saveMask) {
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
		int id;
		uint32_t size;

		PropertyStore::const_iterator it = mPropertyStore.begin();

		for (; it != mPropertyStore.end() ; ++it) {

			id = it->second->id();

			// Determine if the prop should be included, based on it's mask
			uint objmask = 0;

			if (id < 0)
				objmask = 1;

			if (id > 0)
				objmask = 2;

			if (!(saveMask & objmask))
				continue;

			size= it->second->size();
			// load the id
			fprop->writeElem(&id, sizeof(uint32_t));
			// Load the size
			fprop->writeElem(&size, sizeof(uint32_t));

			LOG_DEBUG("PropertyGroup: P$%s : Writing property %s for obj %d", mChunkName.c_str(), mName.c_str(), id, size, mType->size());

			it->second->serialize(fprop);

		}
	}

	// --------------------------------------------------------------------------
	void PropertyGroup::clear() {
		PropertyChangeMsg msg;

		msg.change = PROP_GROUP_CLEARED;
		msg.objectID = 0;

		broadcastMessage(msg);

		mPropertyStore.clear();
		mInheritor->clear();
	}

	// --------------------------------------------------------------------------
	bool PropertyGroup::createProperty(int obj_id) {
		PropertyDataPtr propd = new PropertyData(obj_id, mType, mUseDataCache);

		return _addProperty(propd);
	}

	// --------------------------------------------------------------------------
	bool PropertyGroup::createProperty(int obj_id, DTypePtr data) {
		// simply compare the type pointers...
		if (data->type() != mType)
			OPDE_EXCEPT("Incompatible types when creating property data", "PropertyGroup::createProperty");

		PropertyDataPtr propd = new PropertyData(obj_id, data, mUseDataCache);

		return _addProperty(propd);
	}

	// --------------------------------------------------------------------------
	bool PropertyGroup::removeProperty(int obj_id) {
		size_t erased = mPropertyStore.erase(obj_id);

		if (erased) {
			mInheritor->setImplements(obj_id, false);

			return true;
		} else {
			return false;
		}
	}

	// --------------------------------------------------------------------------
	bool PropertyGroup::cloneProperty(int obj_id, int src_id) {
		PropertyDataPtr pd = getData(src_id);

		if (!pd.isNull()) {
		    // TODO: clone the data!
		    // FIX: Implement this method
		    OPDE_EXCEPT("Property cloning is not yet implemented!", "PropertyGroup::cloneProperty");
			/*PropertyDataPtr propd = new PropertyData(obj_id, *pd);

			return _addProperty(propd);*/
		}
		return false;
	}


	// --------------------------------------------------------------------------
	bool PropertyGroup::_addProperty(PropertyDataPtr propd) {
		pair<PropertyStore::iterator, bool> res = mPropertyStore.insert(make_pair(propd->id(), propd));

		if (res.second) {
            mInheritor->setImplements(propd->id(), true);

			return true;
		} else {
			return false;
		}
	}

    // --------------------------------------------------------------------------
    void PropertyGroup::onInheritChange(const InheritValueChangeMsg& msg) {
            // Consult the inheritor value change, and build a property change message

            /* The broadcast of the property change is not done directly in the methods above but here.
            The reason for this is that only the inheritor knows the real character of the change, and the objects that the change inflicted
            */

            PropertyChangeMsg pmsg;

            switch (msg.change) {
                case INH_VAL_ADDED: // Property was added to an object
                    pmsg.change = PROP_ADDED;
                    pmsg.objectID = msg.objectID;
                    break;
                case INH_VAL_CHANGED:
                    pmsg.change = PROP_CHANGED;
                    pmsg.objectID = msg.objectID;
                    break;
                case INH_VAL_REMOVED:
                    pmsg.change = PROP_REMOVED;
                    pmsg.objectID = msg.objectID;
                default:
                    return;
            }

            broadcastMessage(pmsg);
    }
    
    // --------------------------------------------------------------------------
    void PropertyGroup::objectDestroyed(int id) {
    	removeProperty(id);
    }

}

