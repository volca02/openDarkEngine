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


#include "PropertyService.h"
#include "BinaryService.h"
#include "logger.h"
#include "ServiceCommon.h"

using namespace std;

namespace Opde {
	/// helper string iterator over map keys
	class PropertyGroupMapKeyIterator : public StringIterator {
		public:
			PropertyGroupMapKeyIterator(PropertyService::PropertyGroupMap& pGroupMap) :
                            mPGroupMap(pGroupMap) {
				mIter = mPGroupMap.begin();
				mEnd = mPGroupMap.end();
            }

            virtual const std::string& next() {
				assert(!end());
				
				const std::string& s = mIter->first;

				++mIter;

				return s;
            }

            virtual bool end() const {
                return (mIter == mEnd);
            }

        protected:
			PropertyService::PropertyGroupMap::iterator mIter, mEnd;
            PropertyService::PropertyGroupMap& mPGroupMap;
	};
	
	/*--------------------------------------------------------*/
	/*--------------------- PropertyService ------------------*/
	/*--------------------------------------------------------*/
	template<> const size_t ServiceImpl<PropertyService>::SID = __SERVICE_ID_PROPERTY;
	
	PropertyService::PropertyService(ServiceManager *manager, const std::string& name) : ServiceImpl< Opde::PropertyService >(manager, name) {
		// Ensure listeners are created
		// Create the standard property storage factories...
	}

	// --------------------------------------------------------------------------
	PropertyService::~PropertyService() {
		PropertyList::iterator it = mOwnedProperties.begin();
		
		for( ; it != mOwnedProperties.end(); ++it) {
			delete *it;
		}

		mOwnedProperties.clear();
		mPropertyGroupMap.clear();
	}

	// --------------------------------------------------------------------------
	void PropertyService::shutdown() {
		PropertyGroupMap::iterator it = mPropertyGroupMap.begin();
		
		for( ; it != mPropertyGroupMap.end(); ++it) {
			it->second->shutdown();
		}
	}

	// --------------------------------------------------------------------------
	bool PropertyService::init() {
		mServiceManager->createByMask(SERVICE_PROPERTY_LISTENER);
		return true;
	}

	// --------------------------------------------------------------------------
	void PropertyService::bootstrapFinished() {
	}

	// --------------------------------------------------------------------------
	PropertyGroup* PropertyService::createPropertyGroup(const std::string& name, const std::string& chunkName, std::string inheritorName, const DataStoragePtr& storage) {
		PropertyGroup* nr;
		try {
			nr = new PropertyGroup(this, name, chunkName, storage, inheritorName);
		} catch (...) {
			OPDE_EXCEPT("Failed to create property group for " + name,  "PropertyService::createPropertyGroup");
		}
		
		std::pair<PropertyGroupMap::iterator, bool> res = mPropertyGroupMap.insert(make_pair(name, nr));
		
		if (!res.second) {
			delete nr;
			
			OPDE_EXCEPT("Failed to insert new instance of PropertyGroup, name already allocated : " + name,
				    "PropertyService::createPropertyGroup");
		}
		
		// insert the pointer into the to be freed list
		mOwnedProperties.push_back(nr);
		
		LOG_INFO("PropertyService::createPropertyGroup: Created a property group %s (With chunk name %s)", name.c_str(), chunkName.c_str());

		return nr;
	}
	
	// --------------------------------------------------------------------------
	void PropertyService::registerPropertyGroup(PropertyGroup* group) {
		mPropertyGroupMap[group->getName()] = group; // so we'll overwrite if it exists already
	}
	
	// --------------------------------------------------------------------------		
	void PropertyService::unregisterPropertyGroup(PropertyGroup* group) {
		// try to find it by name, remove
		assert(group);
		
		PropertyGroupMap::iterator it = mPropertyGroupMap.find(group->getName());
		
		if (it != mPropertyGroupMap.end()) {
			mPropertyGroupMap.erase(it);
		}
	}
	
	// --------------------------------------------------------------------------
	void PropertyService::load(const FileGroupPtr& db, const BitArray& objMask) {
		// We just give the db to all registered groups
		PropertyGroupMap::iterator it = mPropertyGroupMap.begin();

		for (; it != mPropertyGroupMap.end(); ++it) {
			try {
				LOG_INFO("PropertyService: Loading property group %s", it->first.c_str());
				it->second->load(db, objMask);
			} catch (BasicException &e) {
				LOG_FATAL("PropertyService: Caught a fatal exception while loading PropertyGroup %s : %s", it->first.c_str(), e.getDetails().c_str() );
			}
		}
	}
	
	// --------------------------------------------------------------------------
	void PropertyService::save(const FileGroupPtr& db, const BitArray& objMask) {
		// We just give the db to all registered groups
		PropertyGroupMap::iterator it = mPropertyGroupMap.begin();

		for (; it != mPropertyGroupMap.end(); ++it) {
			it->second->save(db, objMask);
		}
	}

	// --------------------------------------------------------------------------
	void PropertyService::clear() {
		PropertyGroupMap::iterator it = mPropertyGroupMap.begin();

		for (; it != mPropertyGroupMap.end(); ++it) {
			it->second->clear();
		}
	}

	// --------------------------------------------------------------------------
	StringIteratorPtr PropertyService::getAllPropertyNames() {
		return StringIteratorPtr(new PropertyGroupMapKeyIterator(mPropertyGroupMap));
	}
	
	//------------------------------------------------------
	void PropertyService::grow(int minID, int maxID) {
		// grow all the properties
		PropertyGroupMap::iterator it = mPropertyGroupMap.begin();

		for (; it != mPropertyGroupMap.end(); ++it) {
			it->second->grow(minID, maxID);
		}
	}
	
	// --------------------------------------------------------------------------
	PropertyGroup* PropertyService::getPropertyGroup(const std::string& name) {
	    PropertyGroupMap::iterator it = mPropertyGroupMap.find(name);

		if (it != mPropertyGroupMap.end()) {
		    return it->second;
		} else
            return NULL;

	}
	
	// --------------------------------------------------------------------------
	bool PropertyService::has(int obj_id, const std::string& propName) {
		PropertyGroup* prop = getPropertyGroup(propName);
		
		if (prop != NULL) {
			return prop->has(obj_id);
		}
		
		return false;
	}
    
	// --------------------------------------------------------------------------
	bool PropertyService::owns(int obj_id, const std::string& propName) {
		PropertyGroup* prop = getPropertyGroup(propName);
		
		if (prop != NULL) {
			return prop->owns(obj_id);
		}
		
		return false;
	}
    
	// --------------------------------------------------------------------------
	bool PropertyService::set(int obj_id, const std::string& propName, const std::string& propField, const DVariant& value) {
		PropertyGroup* prop = getPropertyGroup(propName);
		
		if (prop != NULL) {
			return prop->set(obj_id, propField, value);
		}
		
		LOG_ERROR("Invalid or undefined property name '%s' on call to PropertyService::set", propName.c_str());
		return false;
	}
	
	// --------------------------------------------------------------------------
	bool PropertyService::get(int obj_id, const std::string& propName, const std::string& propField, DVariant& target) {
		PropertyGroup* prop = getPropertyGroup(propName);
		
		if (prop != NULL) {
			return prop->get(obj_id, propField, target);
		}
		
		return false;
	}

	// --------------------------------------------------------------------------
	DataFieldDescIteratorPtr PropertyService::getFieldDescIterator(const std::string& propName) {
		PropertyGroup* prop = getPropertyGroup(propName);
		
		if (prop != NULL) {
			return prop->getFieldDescIterator();
		}
		
		LOG_ERROR("Invalid or undefined property name '%s' on call to PropertyService::getFieldDescIterator", propName.c_str());
		return DataFieldDescIteratorPtr(NULL); // NULL iterator, no fun for the caller
	}

	// --------------------------------------------------------------------------
	void PropertyService::objectDestroyed(int id) {
		PropertyGroupMap::iterator it = mPropertyGroupMap.begin();

		for (; it != mPropertyGroupMap.end(); ++it) {
			it->second->objectDestroyed(id);
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
		return new PropertyService(manager, mName);
	}

	const uint PropertyServiceFactory::getMask() {
	    return SERVICE_DATABASE_LISTENER | SERVICE_CORE;
	}
	
	const size_t PropertyServiceFactory::getSID() {
		return PropertyService::SID;
	}
}
