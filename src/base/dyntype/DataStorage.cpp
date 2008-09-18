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
}
