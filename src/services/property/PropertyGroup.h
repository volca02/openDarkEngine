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


#ifndef __PROPERTYGROUP_H
#define __PROPERTYGROUP_H

#include "config.h"

#include "PropertyCommon.h"
#include "NonCopyable.h"
#include "DTypeDef.h"
#include "FileGroup.h"
#include "OpdeException.h"
#include "logger.h"
#include "InheritService.h"
#include "PropertyStorage.h"
#include "BitArray.h"

namespace Opde {
	// forward decl.
	class PropertyService;
	
	/** @brief Property group - a group of properties of the same kind (name, type).
	* Property group holds all the properties of the same kind for all the objects.
	*/
	class OPDELIB_EXPORT PropertyGroup : public NonCopyable, public MessageSource<PropertyChangeMsg> {
		public:
			/** PropertyGroup Constructor
			* @param name The property name
			* @param chunk_name The name of the chunk (without the P$)
			* @param storage The property storage created to hold the data
			* @param deleteStorageOnDestroy delete the property storage on destroy of this prop. group?
			* @param ver_maj The major version of the chunk that stores this property
			* @param ver_min The minor version of the chunk that stores this property
			*/
			PropertyGroup(PropertyService* owner, const std::string& name, const std::string& chunk_name, const DataStoragePtr& storage, std::string inheritorName);

			/** Setter for the property chunk version */
			inline void setChunkVersions(uint verMaj, uint verMin) {
				mVerMaj = verMaj;
				mVerMin = verMin;
			};
			
			/** gets the major version of the chunk */
			inline uint getChunkVersionMajor(void) { return mVerMaj; };
			
			/** gets the minor version of the chunk */
			inline uint getChunkVersionMinor(void) { return mVerMin; };

			/** Destructor */
			virtual ~PropertyGroup();
			
			/// Sets this property group's flag mBuiltIn to true, meaning this PropertyGroup was created by code as a core property
			inline void setBuiltin() { mBuiltin = true; };
			
			/// Gets the builtin flag
			inline bool getBuiltin() { return mBuiltin; };
			
			/** Property storage setter
			* @param propStorage The new storage for properties
			* @warning This should not be used when the property group holds some data
			*/
			virtual void setPropertyStorage(const DataStoragePtr& newStorage);
			

			/// Name getter. Returns the name of property this group manages
			const std::string& getName() { return mName; };

			/** Determines whether an object with id obj_id stores or inherits property of this type
			* @param obj_id The object id of the object
			* @return true if the object has the given property
			 */
			bool has(int obj_id) const {
				// Get the effective id for the object
				int eoid = _getEffectiveObject(obj_id);

				if (eoid == 0)
					return false;

				// This code could be removed, if we knew the nonzero eoid will always mean prop exists
				return owns(eoid);
			}

			/** Determines whether an object with id obj_id stores property of this type
			* @param obj_id The object id of the object
			* @return true if the object has the given property
			* @note Will return false if the object only inherits the property, but does not own it (use has for that)
			*/
			bool owns(int obj_id) const {
				return mPropertyStorage->has(obj_id);
			}

			/** Loads properties from a file group
			* @param db The database to load from */
			void load(const FileGroupPtr& db);

			/** Saves properties to a file group
			* @param db The database to save to
			* @param objMask The BitArray of objects to be saved
			*/
			void save(const FileGroupPtr& db, const BitArray& objMask);

			/** Clears the whole property group.
			* Clears out all the properties, and broadcasts PROP_GROUP_CLEARED
			*/
			void clear();

			/** Creates a property for given object ID, using the default values for the property fields
			* @param obj_id The id of the object to create the property for
			* @return true if the property was created, false if the object ID already holds the given property
			* @note Broadcasts a PROP_ADDED on success
			* @note Notifies inheritor about the change */
			bool createProperty(int obj_id);

			/** Creates a property for given object ID
			* @param obj_id The id of the object to create the property for
			* @return true if the property was removed, false if the object ID didn't hold the property
			* @note Broadcasts a PROP_REMOVED on success
			* @note Notifies inheritor about the change */
			bool removeProperty(int obj_id);

			/** Creates a new property, or replaces all the values on the current, by cloning a given property on object ID
			* @param obj_id Target object ID (id to create)
			* @param src_id The id of the object to clone property from */
			bool cloneProperty(int obj_id, int src_id);

			// ----------------- Data manipulation related methods --------------------
			/** Direct data setter.
			* @param id object id
			* @param field the field name
			* @param value New value for the field
			* @return true if the change was sucessful
			* @see owns
			* @note Will log error when object id does not own the property to be changed */
			bool set(int id, const std::string& field, const DVariant& value);

			/** Direct data getter
			* @param id object id
			* @param field the field name
			* @param target The target value holder to be filled from the field indicated, or untouched if field invalid
			* @see owns
			* @return false if field name was invalid, true if value was set in target
			*/
			bool get(int id, const std::string& field, DVariant& target);
			
			/** Notification that an object was destroyed. @see PropertyService::objectDestroyed */
			void objectDestroyed(int id);

			/** @return A reference to const DataFieldDesc iterator, usable for data description, automatic gui composition, etc.
			* Internally, this is just a wrapper around getFieldDescIterator call to PropertyStorage.
			* @todo It should be decided if it is guaranteed to have this iterator in a storable quality - if it could be used to load/save data.
			*/
			DataFieldDescIteratorPtr getFieldDescIterator(void);

		protected:
			/** Does the internal handling related to the creation of a property for object
			* @param objID The object id to which a property was added
			*/
			void _addProperty(int objID);

            /// The listener to the inheritance messages
            void onInheritChange(const InheritValueChangeMsg& msg);

			/** Returns an ID of the object which is responsible for the current property values
			* As the properties can be inherited using both archetype links and MetaProps,
			* there must be a way to know what object currently represents the property values
			* @param obj_id The object id to query
			* @return object ID of the effective property holder, or zero (0) if not found */
			int _getEffectiveObject(int obj_id)  const {
				return mInheritor->getEffectiveID(obj_id);
			}

			/// The name of the property
			std::string mName;

			/// The name of the chunk data (reduced of the P$)
			std::string mChunkName;

			/// chunk version - major
			uint mVerMaj;
			/// chunk version - minor
			uint mVerMin;

            /// Inheritor used to determine property inheritance
			InheritorPtr mInheritor;

			/// Inheritor value changes listener
			Inheritor::ListenerID mInheritorListenerID;
			
			/// Property storage used to store data for the property
			DataStoragePtr mPropertyStorage;
			
			/// Owner service
			PropertyService* mOwner;
			
			/// Builtin flag - property groups created in code as builtin have true here
			bool mBuiltin;
	};

	/// Shared pointer to property group
	// typedef shared_ptr<PropertyGroup> PropertyGroupPtr;

}

#endif
