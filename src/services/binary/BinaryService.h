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


#ifndef __BINARYSERVICE_H
#define __BINARYSERVICE_H

#include "ConsoleCommandListener.h"
#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "DTypeDef.h"
#include "SharedPtr.h"

#define BINARY_GROUP_SEPARATOR '/'
namespace Opde {

	/** @brief Binary service - a catalogue of binary structure definitions.
	*
	* This service is responsible for storing the definitions of the dynamic binary templates.
	*/
	class BinaryService : public Service, public ConsoleCommandListener {
		public:
			/** Initializes the Service */
			BinaryService(ServiceManager* manager, const std::string& name);

			/** Destructs the WorldRepService instance, and unallocates the data, if any. */
			virtual ~BinaryService();

			/** Add a type definition to a certain group (or global, if group=="")
			* @param def The type definition to insert (the used name is taken from it)
			* @note Do release() the def if you do not intend to further use it once registered here
			*/
			void addType(const std::string& group, DTypeDefPtr def);

			/** Add a enumeration definition to a certain group (or global, if group=="") */
			void addEnum(const std::string& group, const std::string& name, DEnumPtr enm);

			/** Add a type definition to the global definitions
			* @param def The type definition to insert (the used name is taken from it)
			* @see addType(const std::string&,DTypeDef)
			*/
			inline void addType(DTypeDefPtr def) { addType("", def); };

			/** Add a type definition to the global definitions */
			inline void addEnum(const std::string& name, DEnumPtr enm) { addEnum("", name, enm); };

			/** get the type definition of a type.
			* @param name The type name, in form "Name" or "Group/Name"
			* @note You have to release the typedef once you stop using it
			* @throw BasicException if path contains multiple separators */
			DTypeDefPtr getType(const std::string& name) const;

			/** get the type definition of a type.
			* @param name The type name
			* @note You have to release the typedef once you stop using it */
			DTypeDefPtr getType(const std::string& group, const std::string& name) const;

			/** get the type definition of a type.
			* @param name The enum name, in form "Name" or "Group/Name"
			* @note You have to release the typedef once you stop using it */
			DEnumPtr getEnum(const std::string& name) const;

			/** get the type definition of a type.
			* @param name The enum name
			* @note You have to release the typedef once you stop using it */
			DEnumPtr getEnum(const std::string& group, const std::string& name) const;

			/** clear the definitions, leaving groups empty */
			void clear();

			/** clear all the definitions and groups */
			void clearAll();

			virtual void commandExecuted(std::string command, std::string parameters);

		protected:
            bool init();

			/** Split the path to groups - name
			* @note if not possible(no path separator), the first is "" and second the source string
			* @throw BasicException if more than one separator is present */
			std::pair<std::string, std::string> splitPath(const std::string& path) const;

			typedef std::map<std::string, DTypeDefPtr> TypeMap;
			typedef std::map<std::string, TypeMap> TypeGroups;

			typedef std::map<std::string, DEnumPtr> EnumMap;
			typedef std::map<std::string, EnumMap> EnumGroups;

			TypeGroups mTypeGroups;
			EnumGroups mEnumGroups;
	};

	/// Shared pointer to binary service
	typedef shared_ptr<BinaryService> BinaryServicePtr;

	/// Factory for the WorldRep service
	class BinaryServiceFactory : public ServiceFactory {
		public:
			BinaryServiceFactory();
			~BinaryServiceFactory() {};

			/** Creates a BinaryService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

		private:
			static std::string mName;
	};
}


#endif
