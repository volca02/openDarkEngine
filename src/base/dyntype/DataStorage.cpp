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

#include "DataStorage.h"
#include "logger.h"

using namespace std;

namespace Opde {
	// --------------------------------------------------------------------------
	// --------------- Various utility classes ----------------------------------
	// --------------------------------------------------------------------------
	DTypeDefFieldDesc::DTypeDefFieldDesc(const DTypeDefPtr& type) {
		// prepare the list
		DTypeDef::const_iterator it = type->begin();
		
		while (it != type->end()) {
			const DTypeDef::FieldDef& fd = *(it++);
			
			DataFieldDesc dfd;
			dfd.name = fd.name;
			dfd.size = fd.type->size();
			dfd.type = fd.type->getDataType();
			dfd.enumerator = fd.type->getEnum().ptr();
			
			mDataFieldDescList.push_back(dfd);
		}
	}
	
	DataFieldDescIteratorPtr DTypeDefFieldDesc::getIterator() {
		return new DataFieldDescListIterator(mDataFieldDescList);
	}
	
	/// iterator over a single DataFieldDesc element. Useful for single-fielded properties, such as varstr properties
	class SingleFieldDescIterator : public DataFieldDescIterator {
	    public:
            SingleFieldDescIterator(const DataFieldDesc& desc) : mDesc(desc), mEnd(false) {};
            
            virtual const DataFieldDesc& next() { assert(!mEnd); mEnd = true; return mDesc; };
            
            virtual bool end() const { std::cerr << "SFDI " << this << " " << mEnd << std::endl; return mEnd; };
            
        protected:
            const DataFieldDesc& mDesc;
            bool mEnd;
	};
	
	// --------------------------------------------------------------------------
	// --------------- Structured Data Storage (DType based) --------------------
	// --------------------------------------------------------------------------

	StructuredDataStorage::StructuredDataStorage(const DTypeDefPtr& type, bool useDataCache) : 
		mTypeDef(type), 
		mUseDataCache(useDataCache),
		mFieldDesc(type) {
	}
	
	// --------------------------------------------------------------------------
	bool StructuredDataStorage::isEmpty() {
		return mDataMap.empty();
	}

	// --------------------------------------------------------------------------
	void StructuredDataStorage::clear() {
		mDataMap.clear();
	}

	// --------------------------------------------------------------------------
	bool StructuredDataStorage::readFromFile(FilePtr& file, int objID, bool sizeStored) {
		DTypePtr pd = getDataForObject(objID);
		
		if (pd.isNull()) {
			uint32_t size;
			
			// Read the size
			if (sizeStored)
				file->readElem(&size, sizeof(uint32_t));
			else
				size = mTypeDef->size();
			
			// compare sizes
			if (size != mTypeDef->size())
				LOG_ERROR("Data size mismatch: %d definition, %d file source", mTypeDef->size(), size);
			
			// create a new data
			pd = _create(objID);

			pd->read(file, size);
			
			mDataMap.insert(std::make_pair(objID, pd));
			
			return true;
		} else {
			// still have to at least skip the data
			uint32_t size;
			
			// Read the size
			file->readElem(&size, sizeof(uint32_t));
			
			file->seek(size, File::FSEEK_CUR);
			
			LOG_ERROR("Data already defined for object %d", objID);
		}
		
		return false;
	}

	// --------------------------------------------------------------------------
	bool StructuredDataStorage::writeToFile(FilePtr& file, int objID, bool sizeStored)	{
		DTypePtr pd = getDataForObject(objID);
		
		if (!pd.isNull()) {
			uint32_t size = pd->size();
			
			// Write the size
			if (sizeStored)
				file->writeElem(&size, sizeof(uint32_t));
			
			// write the data itself
			pd->serialize(file);
			
			return true;
		}
		
		return false;
	}

	// --------------------------------------------------------------------------
	bool StructuredDataStorage::setField(int objID, const std::string& field, const DVariant& value) {
		DTypePtr pd = getDataForObject(objID);
		
		if (!pd.isNull()) {
			// delegate to pd to set the field
			pd->set(field, value);
			
			return true;
		}
		
		return false;
	}

	// --------------------------------------------------------------------------
	bool StructuredDataStorage::getField(int objID, const std::string& field, DVariant& target) {
		DTypePtr pd = getDataForObject(objID);
		
		if (!pd.isNull()) {
			// delegate to pd to get the field val.
			// TODO: Faster using a pass by reference construct?
			target = pd->get(field);
			
			return true;
		}
		
		return false;
	}

	// --------------------------------------------------------------------------	
	bool StructuredDataStorage::has(int objID) {
		DataMap::iterator it = mDataMap.find(objID);
		
		return (it != mDataMap.end());
	}
	
	// --------------------------------------------------------------------------	
	bool StructuredDataStorage::clone(int srcID, int dstID) {
		// clone prop data
		if (!has(srcID) || has(dstID))
			return false;
			
		DTypePtr pd = getDataForObject(srcID);
		
		DTypePtr nd = new DType(*pd, mUseDataCache);
		
		// insert into map for the new object
		std::pair<DataMap::iterator, bool> res  = mDataMap.insert(std::make_pair(dstID, nd));
			
		return res.second;
	}

	// --------------------------------------------------------------------------
	bool StructuredDataStorage::destroy(int objID) {
		DataMap::iterator it = mDataMap.find(objID);
		
		if (it != mDataMap.end()) {
			mDataMap.erase(it);
			return true;
		}
		
		return false;
	}

	// --------------------------------------------------------------------------
	bool StructuredDataStorage::create(int objID) {
		return !(_create(objID).isNull());
	}
	
	// --------------------------------------------------------------------------
	DTypePtr StructuredDataStorage::_create(int objID) {
		DataMap::iterator it = mDataMap.find(objID);
		
		if (it == mDataMap.end()) {
			DTypePtr propd = new DType(mTypeDef, mUseDataCache);
			
			mDataMap.insert(std::make_pair(objID, propd));
			
			return propd;
		}
		
		return it->second;
	}

	// --------------------------------------------------------------------------	
	IntIteratorPtr StructuredDataStorage::getAllStoredObjects() {
		return new DataKeyIterator(mDataMap);
	}
	
	// --------------------------------------------------------------------------
	DTypePtr StructuredDataStorage::getDataForObject(int objID) {
		DataMap::iterator it = mDataMap.find(objID);
		
		if (it != mDataMap.end()) {
			return it->second;
		} else {
			return NULL;
		}
	}
	
	// --------------------------------------------------------------------------
	DataFieldDescIteratorPtr StructuredDataStorage::getFieldDescIterator(void) {
		// return our pre-prepared field desc iterator
		return mFieldDesc.getIterator();
	}
	
	// --------------------------------------------------------------------------
	// --------------- String Data Storage --------------------------------------
	// --------------------------------------------------------------------------
	StringDataStorage::StringDataStorage() {
	    mFieldDesc.enumerator = NULL;
	    mFieldDesc.name = "";
	    mFieldDesc.size = -1;
	    mFieldDesc.type = DVariant::DV_STRING;
	}

	// --------------------------------------------------------------------------
	bool StringDataStorage::create(int objID) {
		return _create(objID, "");
	}
	
	// --------------------------------------------------------------------------
	bool StringDataStorage::destroy(int objID) {
		StringDataMap::iterator it = mStringPropMap.find(objID);
		
		if (it != mStringPropMap.end()) {
			mStringPropMap.erase(it);
			
			// true, erase went ok
			return true;
		}
		
		return false;
	}
	
	// --------------------------------------------------------------------------
	bool StringDataStorage::has(int objID) {
		StringDataMap::iterator it = mStringPropMap.find(objID);
		
		return (it != mStringPropMap.end());
	}
	
	// --------------------------------------------------------------------------
	bool StringDataStorage::clone(int srcID, int dstID) {
		StringDataMap::iterator it = mStringPropMap.find(srcID);
		
		if (it != mStringPropMap.end()) {
			std::pair<StringDataMap::iterator, bool> res = mStringPropMap.insert(std::make_pair(dstID, it->second));
			
			return res.second;
		}
		
		return false;
	}
	
	// --------------------------------------------------------------------------
	bool StringDataStorage::getField(int objID, const std::string& field, DVariant& target) {
		assert(field=="");
		
		StringDataMap::iterator it = mStringPropMap.find(objID);
		
		if (it != mStringPropMap.end()) {
			target = it->second;
			
			return true;
		}
		
		return false;
	}
	
	// --------------------------------------------------------------------------
	bool StringDataStorage::setField(int objID, const std::string& field, const DVariant& value) {
		assert(field=="");
		
		StringDataMap::iterator it = mStringPropMap.find(objID);
		
		if (it != mStringPropMap.end()) {
			it->second = value.toString();
			
			return true;
		}
		
		return false;
	}
	
	// --------------------------------------------------------------------------
	bool StringDataStorage::writeToFile(FilePtr& file, int objID, bool sizeStored) {
		StringDataMap::iterator it = mStringPropMap.find(objID);
		
		if (it != mStringPropMap.end()) {
			const std::string& str = it->second;
			
			uint32_t size = str.size();
			uint32_t outer_size = size + sizeof(uint32_t);
			
			// Write the size (first size is the size of the string + 4 bytes of the internal size)
			if (sizeStored)
				file->writeElem(&outer_size, sizeof(uint32_t));
			
			file->writeElem(&size, sizeof(uint32_t));
			
			// write the data itself
			file->write(str.c_str(), size);
			
			return true;
		}
		
		return false;
	}
	
	// --------------------------------------------------------------------------
	bool StringDataStorage::readFromFile(FilePtr& file, int objID, bool sizeStored) {
		StringDataMap::iterator it = mStringPropMap.find(objID);
		
		if (it == mStringPropMap.end()) {
			uint32_t outer_size, size;
			
			if (sizeStored)
				file->readElem(&outer_size, sizeof(uint32_t));
			
			file->readElem(&size, sizeof(uint32_t));
			
			if (!sizeStored)
				outer_size = size - sizeof(uint32_t);
				
			assert(outer_size == (size + sizeof(uint32_t)));
			
			// prepare the string temp buffer
			char* str = new char[size + 1];
			
			str[size] = 0; // terminate to be sure
			
			file->read(str, size);
			
			std::string sobj(str);
			
			_create(objID, sobj);
			
			delete[] str;
			
			return true;
		} else {
			// skip the prop...
			uint32_t size;
			
			file->readElem(&size, sizeof(uint32_t));
			file->seek(size, File::FSEEK_CUR);
			
			LOG_ERROR("Data (str) already defined for object %d", objID);
		}
		
		return false;
	}

	// --------------------------------------------------------------------------
	bool StringDataStorage::_create(int objID, const std::string& text) {
		std::pair<StringDataMap::iterator, bool> res = mStringPropMap.insert(std::make_pair(objID, text));
		
		return res.second;
	}
	
	// --------------------------------------------------------------------------
	void StringDataStorage::clear() {
		mStringPropMap.clear();
	}
	
	// --------------------------------------------------------------------------
	bool StringDataStorage::isEmpty() {
		return mStringPropMap.empty();
	}
	
	// --------------------------------------------------------------------------
	IntIteratorPtr StringDataStorage::getAllStoredObjects() {
		return new StringDataMapKeyIterator(mStringPropMap);
	}

    // --------------------------------------------------------------------------
    DataFieldDescIteratorPtr StringDataStorage::getFieldDescIterator(void) {
        return new SingleFieldDescIterator(mFieldDesc);
    }

	// --------------------------------------------------------------------------
	// --------------- Bool Data Storage ----------------------------------------
	// --------------------------------------------------------------------------
	
}
