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

using namespace std;

namespace Opde {
	
	// --------------------------------------------------------------------------
	PropertyGroup::PropertyGroup(const std::string& name, const std::string& chunk_name, DTypeDefPtr type, uint ver_maj, uint ver_min) :
			mPropertyListeners(), 
			mName(name), 
			mChunkName(chunk_name),
			mType(type),
			mVerMaj(ver_maj),
			mVerMin(ver_min) {
		
	}
	
	// --------------------------------------------------------------------------
	PropertyGroup::~PropertyGroup() {
		mPropertyListeners.clear(); // those which did not ask for removal
		
		
	}
	
	// --------------------------------------------------------------------------
	PropertyDataPtr PropertyGroup::getData(int obj_id) {
		PropertyStore::const_iterator it = mPropertyStore.find(obj_id);
		
		if (it != mPropertyStore.end())
			return it->second;
		else
			LOG_ERROR("PropertyGroup::getData : Property for object ID %d was not found in group %s", obj_id, mName.c_str());
	}
	
	// --------------------------------------------------------------------------
	void PropertyGroup::load(FileGroup* db) {
		// TODO: Code
	}
	
	
	// --------------------------------------------------------------------------
	void PropertyGroup::save(FileGroup* db, uint saveMask) {
		// TODO: Code
	}
	
	// --------------------------------------------------------------------------
	bool PropertyGroup::createProperty(int obj_id) {
		PropertyDataPtr propd = new PropertyData(obj_id, mType);
		
		pair<PropertyStore::iterator, bool> res = mPropertyStore.insert(make_pair(obj_id, propd));
		
		if (res.second) {
			// TODO: Notify inheritor!
			
			PropertyChangeMsg msg;
			
			msg.change = PROP_ADDED;
			msg.objectID = obj_id;
			msg.data = propd;
			
			broadcastPropertyMessage(msg);
			
			return true;
		} else {
			return false;
		}
	}
		
	// --------------------------------------------------------------------------
	bool PropertyGroup::removeProperty(int obj_id) {
		size_t erased = mPropertyStore.erase(obj_id);
		
		if (erased) {
			// TODO: Notify inheritor!
			
			PropertyChangeMsg msg;
			
			msg.change = PROP_REMOVED;
			msg.objectID = obj_id;
			msg.data = PropertyDataPtr(); // NULL that means
			
			broadcastPropertyMessage(msg);
			
			return true;
		} else {
			return false;
		}
	}

	
	// --------------------------------------------------------------------------
	void PropertyGroup::broadcastPropertyMessage(const PropertyChangeMsg& msg) const {
		PropertyListeners::iterator it = mPropertyListeners.begin();
		
		for (; it != mPropertyListeners.end(); ++it) {
			// Call the method on the listener pointer 
			((*it)->listener->*(*it)->method)(msg);
		}
	}
	
	// --------------------------------------------------------------------------
	void PropertyGroup::registerListener(PropertyChangeListenerPtr* listener) {
		mPropertyListeners.insert(listener);
	}
	
	// --------------------------------------------------------------------------
	void PropertyGroup::unregisterListener(PropertyChangeListenerPtr* listener) {
		mPropertyListeners.erase(listener);
	}

	
}
 
