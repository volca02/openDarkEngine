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
	class PropertyService : public Service {
		public:
			PropertyService(ServiceManager *manager);
			virtual ~PropertyService();

			/** Creates a property group - a family of properties of the same type.
			* @see PropertyGroup::PropertyGroup
			*/
			PropertyGroupPtr createPropertyGroup(const std::string& name, const std::string& chunk_name, DTypeDefPtr type, uint ver_maj, uint ver_min, std::string inheritorName);

            /** Retrieves the property group given it's name, or NULL if not found
            * @param name The name of the property to retrieve the group for
            * @return PropertyGroupPtr of the PropertyGroup named name if found, isNull()==true otherwise */
            PropertyGroupPtr getPropertyGroup(const std::string& name);

		protected:
            /// service initialization
            virtual void bootstrapFinished();

            /// Database change message callback
            void onDBChange(const DatabaseChangeMsg& m);

            /** Load the properties from the database
			* @param db The database file group to use */
			void _load(FileGroupPtr db);

			/** Saves the properties according to the saveMask
			* @param db The database file group to save to
			* @param saveMask The save mask (1 - Archetypes, 2 - Instances, 3 - both) */
			void _save(FileGroupPtr db, uint saveMask);

			/** Clears out all the PropertyGroups (effectively wiping out all properties) */
			void _clear();


			/// maps property groups to their names
			typedef std::map< std::string, PropertyGroupPtr > PropertyGroupMap;

			/// maps the properties by their names
			PropertyGroupMap mPropertyGroupMap;

            /// Database callback
            DatabaseService::ListenerPtr mDbCallback;

            /// Database service
            DatabaseServicePtr mDatabaseService;
	};

	/// Shared pointer to Property service
	typedef shared_ptr<PropertyService> PropertyServicePtr;

	/// Factory for the PropertyService objects
	class PropertyServiceFactory : public ServiceFactory {
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
