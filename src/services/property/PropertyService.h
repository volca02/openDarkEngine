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


#ifndef __PROPERTYSERVICE_H
#define __PROPERTYSERVICE_H

#include "config.h"

#include "PropertyCommon.h"
#include "PropertyGroup.h"
#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "FileGroup.h"
#include "SharedPtr.h"
#include "MessageSource.h"

namespace Opde {
	/** @brief Property service - service managing in-game object properties
	*/
	class OPDELIB_EXPORT PropertyService : public Service {
		public:
			PropertyService(ServiceManager *manager, const std::string& name);
			virtual ~PropertyService();
			
			/** Creates a standard (data-holding only) property group using the specified property storage. This method should only be used for data processing applications.
			* @param name The name of the property group
			* @param chunkName The name of the chunk the property is stored in
			* @param inheritorName The name of the iheritor to use for the property (the published name of the inheritor factory)
			* @param storage The property storage to be used for the property data. Caller is responsible for the storage destruction, unless takeover is set to true
			* @param takeover If true, the storage's ownership will be taken over, meaning the storage will be destroyed upon destruction of the property group (or when construction fails)
			* @see PropertyGroup::PropertyGroup
			*/
			PropertyGroup* createPropertyGroup(const std::string& name, const std::string& chunkName, std::string inheritorName, const DataStoragePtr& storage);

			/** Registers a custom property group to the property service. These properties are not destroyed at the end of the lifetime of this service.
			*/
			void registerPropertyGroup(PropertyGroup* group);
			
			/** Unregisters a custom property group from the property service.
			*/
			void unregisterPropertyGroup(PropertyGroup* group);
			
			/** Retrieves the property group given it's name, or NULL if not found
			* @param name The name of the property to retrieve the group for
			* @return PropertyGroup* of the PropertyGroup named name if found, isNull()==true otherwise */
			PropertyGroup* getPropertyGroup(const std::string& name);
			
			/** Determines if the given object has a property mapped (either itself, or by inheritance through MetaProperty link)
			* @param obj_id The object id to query
			* @param propName The name of the property to query for
			* @return true if the object has the specified property, false if not
			*/
			bool has(int obj_id, const std::string& propName);
			
			/** Determines if the given object is a direct owner of a given property
			* @param obj_id The object id to query
			* @param propName The name of the property to query for
			* @return true if the object owns the specified property, false if not
			*/
			bool owns(int obj_id, const std::string& propName);
			
			/** Property setter. Sets a value of a property field
			* @param obj_id The object id
			* @param propName The name of the property group
			* @param propField The field path to set
			* @param value The new value
			*/
			bool set(int obj_id, const std::string& propName, const std::string& propField, const DVariant& value);
			
			/** Property getter. Gets a value of a property field
			* @param obj_id The object id
			* @param propName The name of the property group
			* @param propField The field path to get
			*/
			bool get(int obj_id, const std::string& propName, const std::string& propField, DVariant& target);

			/** A shortcut to PropertyGroup::getFieldDescIterator
			* @param propName The name of the property
			* @return the iterator over property field descriptions, or NULL if invalid name was specified
			* @see PropertyGroup::getFieldDescIterator
			*/
			DataFieldDescIteratorPtr getFieldDescIterator(const std::string& propName);

			/** A notification that object was destroyed (removes all properties of the obj. ID)
			* @param id The object id that was removed
			* @note Do NOT call this directly unless you know what it does
			*/
			void objectDestroyed(int id);

			/** Load the properties from the database
			* @param db The database file group to use */
			void load(const FileGroupPtr& db);

			/** Saves the properties according to the saveMask
			* @param db The database file group to save to
			* @param objMask the BitArray of objects to be written */
			void save(const FileGroupPtr& db, const BitArray& objMask);

			/** Clears out all the PropertyGroups (effectively wiping out all properties) */
			void clear();
			
			/** @return a property name iterator usable to iterate over all property types */
			StringIteratorPtr getAllPropertyNames();

			/// maps property groups to their names
			typedef std::map< std::string, PropertyGroup* > PropertyGroupMap;
			
			typedef std::list< PropertyGroup* > PropertyList;

		protected:
			/// service initialization
			bool init();

			/// service initialization
			void bootstrapFinished();

			/// service deinitialization
			void shutdown();

			/// maps the properties by their names
			PropertyGroupMap mPropertyGroupMap;
			
			/// List of properties that will be freed upon service termination
			PropertyList mOwnedProperties;

			/// Database service
			DatabaseServicePtr mDatabaseService;
	};

	/// Shared pointer to Property service
	typedef shared_ptr<PropertyService> PropertyServicePtr;

	/// Factory for the PropertyService objects
	class OPDELIB_EXPORT PropertyServiceFactory : public ServiceFactory {
		public:
			PropertyServiceFactory();
			~PropertyServiceFactory() {};

			/** Creates a PropertyService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

			virtual const uint getMask();

		private:
			static std::string mName;
	};
}


#endif
