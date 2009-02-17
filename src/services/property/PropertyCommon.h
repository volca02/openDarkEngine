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
 *	  $Id$
 *
 *****************************************************************************/

 
#ifndef __PROPERTYCOMMON_H
#define __PROPERTYCOMMON_H

#include "compat.h"
#include "DTypeDef.h"
#include "File.h"

namespace Opde {
	/** @brief Property data holder
	* @deprecated We now have common data storage for both link and data, and extra layer of abstraction using DataStorage class
	*/
	class PropertyData : public DType {
		friend class PropertyGroup;
		
		public:
			/** Constructor. Constructs new data buffer, filled with zeros */
			PropertyData(int id, const DTypeDefPtr& type, bool useCache = false) : DType(type, useCache), mID(id) {};
			
			/** Constructor - loads data from FilePtr 
			* @param id The object ID
			* @param type The DTypeDef pointer to use (type definition)
			* @param file The File pointer (FilePtr) to load data from
			* @param _size the size of the data to be loaded
			* */
			PropertyData(int id, const DTypeDefPtr& type, FilePtr file, int _size, bool useCache = false) : DType(type, file, _size, useCache), mID(id) { };
			
			/** Constructor - takes object id and data instance 
			* @param id The object ID
			* @param data The Data instance to copy the data from */
			PropertyData(int id, const DTypePtr& data, bool useCache = false) : DType(*data, useCache), mID(id) { };
			
			/** PropertyData cloning constructor
			* @param id The object ID
			* @param data The Data instance to copy the data from */
			PropertyData(int id, const PropertyData& data, bool useCache = false) : DType(data, useCache), mID(id) { };
			
			/** Destructor */
			~PropertyData() { };
		
			inline int id() { return mID; };
		protected:
			/// object holder ID
			int mID;
	};
	
	/// Shared pointer to property data
	typedef shared_ptr<PropertyData> PropertyDataPtr;
	
	/// Supportive PropertyData comparison operator for sets and maps
	inline bool operator<(const PropertyDataPtr& a, const PropertyDataPtr& b) {
		return a->id() < b->id();
	}
	
	/// Property change types
	enum PropertyChangeType {
		/// Link was added (Sent after the addition)
		PROP_ADDED = 1,
		/// Link was removed (Sent before the removal)
		PROP_REMOVED, 
		/// Link has changed data (Sent after the change)
		PROP_CHANGED,
		/// All the links were removed from the relation (Sent before the cleanout)
		PROP_GROUP_CLEARED
	};
	
	/// Property chage message
	struct PropertyChangeMsg {
		/// A change that happened
		PropertyChangeType change;
		
		/// An ID of the object that changed
		int objectID;
	};
}
 
#endif
