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

			/** Load the properties from the database
			* @param db The database file group to use */
			void load(FileGroup* db);

			/** Saves the properties according to the saveMask
			* @param db The database file group to save to
			* @param saveMask The save mask (1 - Archetypes, 2 - Instances, 3 - both) */
			void save(FileGroup* db, uint saveMask);

			/** Clears out all the PropertyGroups (effectively wiping out all properties) */
			void clear();

		protected:
			/// maps property groups to their names
			typedef std::map< std::string, PropertyGroupPtr > PropertyGroupMap;

			/// maps the properties by their names
			PropertyGroupMap mPropertyGroupMap;
	};

	/// Shared pointer to Link service
	typedef shared_ptr<PropertyService> PropertyServicePtr;

	/// Factory for the LinkService objects
	class PropertyServiceFactory : public ServiceFactory {
		public:
			PropertyServiceFactory();
			~PropertyServiceFactory() {};

			/** Creates a LinkService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

		private:
			static std::string mName;
	};
}


#endif
