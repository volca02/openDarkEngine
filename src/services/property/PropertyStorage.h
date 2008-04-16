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


#ifndef __PROPERTYSTORAGE_H
#define __PROPERTYSTORAGE_H

#include "PropertyCommon.h"
#include "DTypeDef.h"
#include "DVariant.h"
#include "File.h"
#include "Iterator.h"

namespace Opde {
	// Forward Declaration
	class PropertyGroup;

	/** @brief Storage for properties (Interface).
	* This storage can be overriden for certain properties to manage service data in effective/different manner for example. 
	* @note This class does no inheritance resolving itself. Only stores property data.
	*/
	class PropertyStorage {
		friend class PropertyGroup;
	
		protected:
			/** Creates a default-value property for object numbered objID
			* @param objID The object ID
			* @return true if creation wen't ok, false if something went wrong (object already has the property attached)
			*/
			virtual bool createProp(int objID) = 0;
			
			/** Destroys property on a defined object
			* @param objID the object to destroy the property for
			* @return true if property was destroyed, false if something went wrong (property was not assigned to the object)
			*/
			virtual bool destroyProp(int objID) = 0;
			
			/** Property ownership detector. Returns true if the given object ID holds a property data.
			*/
			virtual bool hasProp(int objID) = 0;
			
			/** Clones the property to another object ID (copies all values)
			* @note The target property has to not exit (this routine does not overwrite)
			* @param srcID the source object ID
			* @param dstID the destination ID
			* @return true if all went ok, false otherwise (srcID invalid, dstID invalid...)
			*/
			virtual bool cloneProp(int srcID, int dstID) = 0;
			
			/** Field value getter
			* @param objID The object id to get property value for
			* @param field The field to get value for
			* @param target The target variant to fill with value
			* @return true if value was set to target, false if the field was invalid
			*/
			virtual bool getPropField(int objID, const std::string& field, DVariant& target) = 0;
			
			/** Field value setter
			* @param objID The object id to set property value for
			* @param field The field to set value for
			* @param value The new value to use
			* @return true if value was set to target, false if the field was invalid
			*/
			virtual bool setPropField(int objID, const std::string& field, const DVariant& value) = 0;
			
			/** Serialization core routine
			* This routine is used to serialize the property data for given object ID into a File handle.
			* Normally, this routine should write data for the stored property in this format:
			* @code
			*	32-bit unnsigned size (N)
			*	N bytes of Property data for the given property
			* @endcode
			*/
			virtual bool writeToFile(FilePtr& file, int objID) = 0;
			
			/** Deserialization core routine
			* This routine is used to read property data for a given object id from a given File handle.
			* Normally, this format format is used:
			* @code
			*	32-bit unnsigned size (N)
			*	N bytes of Property data for the given property
			* @endcode
			* @note This routine automatically creates property data slot for the objID, and does no load over any previous data (so clear before load is encouraged)
			*/
			virtual bool readFromFile(FilePtr& file, int objID) = 0;
			
			/** Called to clear all property data */
			virtual void clear() = 0;
			
			/** Emptyness of property storage detector
			* @return true If the property storage does not hold any data, false if does */
			virtual bool isEmpty() = 0;
			
			/** Getter for all stored object ID's
			*/
			virtual IntIteratorPtr getAllStoredObjects() = 0;
	};
	
	/// Shared pointer to property storage
	typedef shared_ptr<PropertyStorage> PropertyStoragePtr;
	
	/** Structured property implementation of the propery storage. */
	class StructuredPropertyStorage : public PropertyStorage {
		public:
			StructuredPropertyStorage(const DTypeDefPtr& type, bool useDataCache);
			
		protected:
			/** @see PropertyStorage::createProp */
			virtual bool createProp(int objID);
			
			/** @see PropertyStorage::destroyProp */
			virtual bool destroyProp(int objID);
			
			/** @see PropertyStorage::hasProp */
			virtual bool hasProp(int objID);
			
			/** @see PropertyStorage::cloneProp */
			virtual bool cloneProp(int srcID, int dstID);
			
			/** @see PropertyStorage::getPropField */
			virtual bool getPropField(int objID, const std::string& field, DVariant& target);
			
			/** @see PropertyStorage::setPropField */
			virtual bool setPropField(int objID, const std::string& field, const DVariant& value);
			
			/** @see PropertyStorage::writeToFile */
			virtual bool writeToFile(FilePtr& file, int objID);
			
			/** @see PropertyStorage::readFromFile */
			virtual bool readFromFile(FilePtr& file, int objID);
			
			/** @see PropertyStorage::clear */
			virtual void clear();
			
			/** @see PropertyStorage::isEmpty */
			virtual bool isEmpty();
			
			/** @see PropertyStorage::getAllStoredObjects */
			virtual IntIteratorPtr getAllStoredObjects();
			
			/** core property creation routine. Returns a pointer to newly created or already existed property data for the objID */
			virtual PropertyDataPtr _createProp(int objID);
			
			/// Internal getter for properties
			PropertyDataPtr getPropForObject(int objID);
			
			/// Stores objectID -> Property
			typedef std::map< int, PropertyDataPtr > PropertyMap;

			/// Property store instance
			PropertyMap mPropertyMap;
			
			/// Type definition for the stored properties
			const DTypeDefPtr mTypeDef;
			
			/// Data cache usage indicator (field conversion speedup)
			bool mUseDataCache;
			
			typedef MapKeyIterator<PropertyMap, int> PropertyDataKeyIterator;
			
			class EmptyIntIterator : public IntIterator {
				public:
					EmptyIntIterator() : mZero(0) {};
					
					const int& next() {
						assert(false);
						
						return mZero;
					};
					
					bool end() const {
						return true;
					};
					
				protected:
					int mZero;
			};
	};
	
	
	/** Property storage targetted at storing variable length strings (One per property). */
	class StringPropertyStorage : public PropertyStorage {
		public:
			StringPropertyStorage();
			
		protected:
			/** @see PropertyStorage::createProp */
			virtual bool createProp(int objID);
			
			/** @see PropertyStorage::destroyProp */
			virtual bool destroyProp(int objID);
			
			/** @see PropertyStorage::hasProp */
			virtual bool hasProp(int objID);
			
			/** @see PropertyStorage::cloneProp */
			virtual bool cloneProp(int srcID, int dstID);
			
			/** @see PropertyStorage::getPropField */
			virtual bool getPropField(int objID, const std::string& field, DVariant& target);
			
			/** @see PropertyStorage::setPropField */
			virtual bool setPropField(int objID, const std::string& field, const DVariant& value);
			
			/** @see PropertyStorage::writeToFile */
			virtual bool writeToFile(FilePtr& file, int objID);
			
			/** @see PropertyStorage::readFromFile */
			virtual bool readFromFile(FilePtr& file, int objID);
			
			/** @see PropertyStorage::clear */
			virtual void clear();
			
			/** @see PropertyStorage::isEmpty */
			virtual bool isEmpty();
			
			/** @see PropertyStorage::getAllStoredObjects */
			virtual IntIteratorPtr getAllStoredObjects();
			
			/** Core property creation routine */
			virtual bool _createProp(int objID, const std::string& text);
			
			/// String property map
			typedef std::map<int, std::string> StringPropertyMap;
			
			/// Holder of property (string) values
			StringPropertyMap mStringPropMap;
			
			typedef MapKeyIterator<StringPropertyMap, int> StringPropertyMapKeyIterator;
	};
	
}

#endif // __PROPERTYSTORAGE_H
