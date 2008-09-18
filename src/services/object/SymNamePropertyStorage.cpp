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


#include "SymNamePropertyStorage.h"

namespace Opde {
	/*-----------------------------------------------------------------------*/
	/*-------------------- Symbolic name property storage -------------------*/
	/*-----------------------------------------------------------------------*/
	
	
	// --------------------------------------------------------------------------
	SymNamePropertyStorage::SymNamePropertyStorage() : StringDataStorage() {};
	
	
	// --------------------------------------------------------------------------
	SymNamePropertyStorage::~SymNamePropertyStorage() {};
	
	// --------------------------------------------------------------------------
	bool SymNamePropertyStorage::destroy(int objID) {
		// Find the object name before destroying
		DVariant res;

		if (getField(objID, "", res)) {
			bool eraseok = StringDataStorage::destroy(objID);
			
			assert(eraseok);
			
			// destroy our record as well
			ReverseNameMap::iterator it = mReverseMap.find(res.toString());
		
			if (it != mReverseMap.end()) {
				mReverseMap.erase(it);
				
				// true, erase went ok on both sides
				return true;
			}
		}
		
		return false;
	};
	
	
	// --------------------------------------------------------------------------
	bool SymNamePropertyStorage::setField(int objID, const std::string& field, const DVariant& value) {
		// if the parent's set field passes, update ourselves as well.
		// but first, look if we can proceed (name duplicity check)
		
		ReverseNameMap::iterator it = mReverseMap.find(value.toString());
		
		if (it != mReverseMap.end()) {
			if (it->second != objID)
				return false;
				
			return true;
		}
		
		if (!has(objID))
			return false;
		
		// get the previous name of the object, free that record
		DataMap::const_iterator sit = mDataMap.find(objID);
		
		assert(sit != mDataMap.end());
		
		// name not in use yet, we can assign it
		if (StringDataStorage::setField(objID, field, value)) {
			ReverseNameMap::iterator rit = mReverseMap.find(sit->second);
			mReverseMap.erase(rit);
			
			std::pair<ReverseNameMap::iterator, bool> res = mReverseMap.insert(std::make_pair(value.toString(), objID));
			
			assert(res.second);
			
			return true;
		}
		
		return false;
	}

	// --------------------------------------------------------------------------
	bool SymNamePropertyStorage::_create(int objID, const std::string& text) {
		std::pair<ReverseNameMap::iterator, bool> res = mReverseMap.insert(std::make_pair(text, objID));
		
		if (res.second) {
			if (StringDataStorage::_create(objID, text)) {
				return true;
			}
		}
		
		return false;
	}
	
	// --------------------------------------------------------------------------
	int SymNamePropertyStorage::objectNamed(const std::string& name) {
		ReverseNameMap::const_iterator it = mReverseMap.find(name);
		
		if (it != mReverseMap.end()) {
			return it->second;
		}
		
		return 0;
	}
	
	// --------------------------------------------------------------------------
	bool SymNamePropertyStorage::nameUsed(const std::string& name) {
		ReverseNameMap::iterator it = mReverseMap.find(name);
		
		return (it != mReverseMap.end());
	}
	
	// --------------------------------------------------------------------------
	void SymNamePropertyStorage::clear() {
		StringDataStorage::clear();
		mReverseMap.clear();
	}
};

