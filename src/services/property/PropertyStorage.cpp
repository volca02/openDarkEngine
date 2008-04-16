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

#include "PropertyStorage.h"
#include "logger.h"

using namespace std;

namespace Opde {
	// --------------------------------------------------------------------------
	// --------------- Default Property Storage ---------------------------------
	// --------------------------------------------------------------------------

	StructuredPropertyStorage::StructuredPropertyStorage(const DTypeDefPtr& type, bool useDataCache) : 
		mTypeDef(type), 
		mUseDataCache(useDataCache) {
	}
	
	// --------------------------------------------------------------------------
	bool StructuredPropertyStorage::isEmpty() {
		return mPropertyMap.empty();
	}

	// --------------------------------------------------------------------------
	void StructuredPropertyStorage::clear() {
		mPropertyMap.clear();
	}

	// --------------------------------------------------------------------------
	bool StructuredPropertyStorage::readFromFile(FilePtr& file, int objID) {
		PropertyDataPtr pd = getPropForObject(objID);
		
		if (pd.isNull()) {
			uint32_t size;
			
			// Read the size
			file->readElem(&size, sizeof(uint32_t));
			
			// compare sizes
			if (size != mTypeDef->size())
				LOG_ERROR("Property size mismatch: %d definition, %d file source", mTypeDef->size(), size);
			
			// create a new property data
			pd = _createProp(objID);

			pd->read(file, size);
			
			mPropertyMap.insert(std::make_pair(objID, pd));
			
			return true;
		} else {
			// still have to at least skip the data
			uint32_t size;
			
			// Read the size
			file->readElem(&size, sizeof(uint32_t));
			
			file->seek(size, File::FSEEK_CUR);
			
			LOG_ERROR("Property already defined for object %d", objID);
		}
		
		return false;
	}

	// --------------------------------------------------------------------------
	bool StructuredPropertyStorage::writeToFile(FilePtr& file, int objID)	{
		PropertyDataPtr pd = getPropForObject(objID);
		
		if (!pd.isNull()) {
			uint32_t size = pd->size();
			
			// Write the size
			file->writeElem(&size, sizeof(uint32_t));
			
			// write the data itself
			pd->serialize(file);
			
			return true;
		}
		
		return false;
	}

	// --------------------------------------------------------------------------
	bool StructuredPropertyStorage::setPropField(int objID, const std::string& field, const DVariant& value) {
		PropertyDataPtr pd = getPropForObject(objID);
		
		if (!pd.isNull()) {
			// delegate to pd to set the field
			pd->set(field, value);
			
			return true;
		}
		
		return false;
	}

	// --------------------------------------------------------------------------
	bool StructuredPropertyStorage::getPropField(int objID, const std::string& field, DVariant& target) {
		PropertyDataPtr pd = getPropForObject(objID);
		
		if (!pd.isNull()) {
			// delegate to pd to get the field val.
			// TODO: Faster using a pass by reference construct?
			target = pd->get(field);
			
			return true;
		}
		
		return false;
	}

	// --------------------------------------------------------------------------	
	bool StructuredPropertyStorage::hasProp(int objID) {
		PropertyMap::iterator it = mPropertyMap.find(objID);
		
		return (it != mPropertyMap.end());
	}
	
	// --------------------------------------------------------------------------	
	bool StructuredPropertyStorage::cloneProp(int srcID, int dstID) {
		// clone prop data
		if (!hasProp(srcID) || hasProp(dstID))
			return false;
			
		PropertyDataPtr pd = getPropForObject(srcID);
		
		PropertyDataPtr nd = new PropertyData(dstID, *pd, mUseDataCache);
		
		// insert into map for the new object
		std::pair<PropertyMap::iterator, bool> res  = mPropertyMap.insert(std::make_pair(dstID, nd));
			
		return res.second;
	}

	// --------------------------------------------------------------------------
	bool StructuredPropertyStorage::destroyProp(int objID) {
		PropertyMap::iterator it = mPropertyMap.find(objID);
		
		if (it != mPropertyMap.end()) {
			mPropertyMap.erase(it);
			return true;
		}
		
		return false;
	}

	// --------------------------------------------------------------------------
	bool StructuredPropertyStorage::createProp(int objID) {
		return !(_createProp(objID).isNull());
	}
	
	// --------------------------------------------------------------------------
	PropertyDataPtr StructuredPropertyStorage::_createProp(int objID) {
		PropertyMap::iterator it = mPropertyMap.find(objID);
		
		if (it == mPropertyMap.end()) {
			PropertyDataPtr propd = new PropertyData(objID, mTypeDef, mUseDataCache);
			
			mPropertyMap.insert(std::make_pair(objID, propd));
			
			return propd;
		}
		
		return it->second;
	}

	// --------------------------------------------------------------------------	
	IntIteratorPtr StructuredPropertyStorage::getAllStoredObjects() {
		return new PropertyDataKeyIterator(mPropertyMap);
	}
	
	// --------------------------------------------------------------------------
	PropertyDataPtr StructuredPropertyStorage::getPropForObject(int objID) {
		PropertyMap::iterator it = mPropertyMap.find(objID);
		
		if (it != mPropertyMap.end()) {
			return it->second;
		} else {
			return NULL;
		}
	}
	
	// --------------------------------------------------------------------------
	// --------------- String Property Storage ----------------------------------
	// --------------------------------------------------------------------------
	StringPropertyStorage::StringPropertyStorage() {
	}

	// --------------------------------------------------------------------------
	bool StringPropertyStorage::createProp(int objID) {
		return _createProp(objID, "");
	}
	
	// --------------------------------------------------------------------------
	bool StringPropertyStorage::destroyProp(int objID) {
		StringPropertyMap::iterator it = mStringPropMap.find(objID);
		
		if (it != mStringPropMap.end()) {
			mStringPropMap.erase(it);
			
			// true, erase went ok
			return true;
		}
		
		return false;
	}
	
	// --------------------------------------------------------------------------
	bool StringPropertyStorage::hasProp(int objID) {
		StringPropertyMap::iterator it = mStringPropMap.find(objID);
		
		return (it != mStringPropMap.end());
	}
	
	// --------------------------------------------------------------------------
	bool StringPropertyStorage::cloneProp(int srcID, int dstID) {
		StringPropertyMap::iterator it = mStringPropMap.find(srcID);
		
		if (it != mStringPropMap.end()) {
			std::pair<StringPropertyMap::iterator, bool> res = mStringPropMap.insert(std::make_pair(dstID, it->second));
			
			return res.second;
		}
		
		return false;
	}
	
	// --------------------------------------------------------------------------
	bool StringPropertyStorage::getPropField(int objID, const std::string& field, DVariant& target) {
		assert(field=="");
		
		StringPropertyMap::iterator it = mStringPropMap.find(objID);
		
		if (it != mStringPropMap.end()) {
			target = it->second;
			
			return true;
		}
		
		return false;
	}
	
	// --------------------------------------------------------------------------
	bool StringPropertyStorage::setPropField(int objID, const std::string& field, const DVariant& value) {
		assert(field=="");
		
		StringPropertyMap::iterator it = mStringPropMap.find(objID);
		
		if (it != mStringPropMap.end()) {
			it->second = value.toString();
			
			return true;
		}
		
		return false;
	}
	
	// --------------------------------------------------------------------------
	bool StringPropertyStorage::writeToFile(FilePtr& file, int objID) {
		StringPropertyMap::iterator it = mStringPropMap.find(objID);
		
		if (it != mStringPropMap.end()) {
			const std::string& str = it->second;
			
			uint32_t size = str.size();
			uint32_t outer_size = size + sizeof(uint32_t);
			
			// Write the size (first size is the size of the string + 4 bytes of the internal size)
			file->writeElem(&outer_size, sizeof(uint32_t));
			
			file->writeElem(&size, sizeof(uint32_t));
			
			// write the data itself
			file->write(str.c_str(), size);
			
			return true;
		}
		
		return false;
	}
	
	// --------------------------------------------------------------------------
	bool StringPropertyStorage::readFromFile(FilePtr& file, int objID) {
		StringPropertyMap::iterator it = mStringPropMap.find(objID);
		
		if (it == mStringPropMap.end()) {
			uint32_t outer_size, size;
			
			file->readElem(&outer_size, sizeof(uint32_t));
			file->readElem(&size, sizeof(uint32_t));
			
			assert(outer_size == (size + sizeof(uint32_t)));
			
			// prepare the string temp buffer
			char* str = new char[size + 1];
			
			str[size] = 0; // terminate to be sure
			
			file->read(str, size);
			
			std::string sobj(str);
			
			_createProp(objID, sobj);
			
			delete[] str;
			
			return true;
		} else {
			// skip the prop...
			uint32_t size;
			
			file->readElem(&size, sizeof(uint32_t));
			file->seek(size, File::FSEEK_CUR);
			
			LOG_ERROR("Property (str) already defined for object %d", objID);
		}
		
		return false;
	}

	// --------------------------------------------------------------------------
	bool StringPropertyStorage::_createProp(int objID, const std::string& text) {
		std::pair<StringPropertyMap::iterator, bool> res = mStringPropMap.insert(std::make_pair(objID, text));
		
		return res.second;
	}
	
	// --------------------------------------------------------------------------
	void StringPropertyStorage::clear() {
		mStringPropMap.clear();
	}
	
	// --------------------------------------------------------------------------
	bool StringPropertyStorage::isEmpty() {
		return mStringPropMap.empty();
	}
	
	// --------------------------------------------------------------------------
	IntIteratorPtr StringPropertyStorage::getAllStoredObjects() {
		return new StringPropertyMapKeyIterator(mStringPropMap);
	}

}
