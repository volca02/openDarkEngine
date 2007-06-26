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

 
#ifndef __LINKSERVICE_H
#define __LINKSERVICE_H

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "FileGroup.h"
#include "LinkCommon.h"
#include "Relation.h"
#include "SharedPtr.h"

namespace Opde {
	
	/** @brief Link service - service managing in-game object links
	*/
	class LinkService : public Service {
		public:
			LinkService(ServiceManager *manager);
			virtual ~LinkService();
			
			/** Load the links from the database */ 
			void load(FileGroup* db);
			
			/** Saves the links and link data according to the saveMask */
			void save(FileGroup* db, uint saveMask);
			
			/** Sets the Relation chunk version */
			void setChunkVersion(uint major, uint minor) {
				mRelVMaj = major;
				mRelVMin = minor;
			}
			
			/** Convert the relation name to a flavor */
			int nameToFlavor(const std::string& name);
			
			/** Creates a relation type (link kind)
			* @param id The ID the relation will have (>0)
			* @param name The relation name
			* @param type The type defining the data format for link data 
			* @param hidden The hidden relations (true) will not show up on public link list places */
			RelationPtr createRelation(const std::string& name, DTypeDefPtr type, bool hidden);
			
			/** Registers a listener for relation events (link addition/removal/chage)
			* @param relname The name of the relation (Link Kind) the listener wants to listen to
			* @param listener a pointer to a LinkChangeListenerPtr struct containing instance and method pointers
			* @note The same pointer to the listener struct has to be supplied to the unregisterLinkListener in order to suceed with unregistration
			*/
			void registerLinkListener(const std::string& relname, LinkChangeListenerPtr* listener);
			
			/** Unregisters a listener for relation events (link addition/removal/chage)
			* @param relname The name of the relation (Link Kind) the listener wants to listen to
			* @param listener a pointer to a LinkChangeListenerPtr struct containing instance and method pointers
			* @note The same pointer to the LinkChangeListenerPtr as the one used to registration has to be used 
			*/
			void unregisterLinkListener(const std::string& relname, LinkChangeListenerPtr* listener);
			
			/** Get relation given it's name
			* @param name The realtion's name
			* @note The relation will be .isNull() if it was not found
			*/
			RelationPtr getRelation(const std::string& name);
			
		protected:
			/** load links from a single database */
			void _load(FileGroup* db);
			
			/** Clears all the data and the relation mappings */
			void _clear();
			
			/** request a mapping Name->Flavor and reverse 
			* @param id The flavor value requested
			* @param name The name for that flavor (Relation name)
			* @param rel The relation instance to associate with that id
			* @return false if conflict happened, true if all went ok, and new mapping is inserted (or already was registered) 
			*/
			bool requestRelationFlavorMap(int id, const std::string& name, RelationPtr rel);
			
			typedef std::map<int, std::string> FlavorToName;
			typedef std::map<std::string, int> NameToFlavor;
			
			/// Name to Relation instance. The primary storage of Relation instances.
			typedef std::map<std::string, RelationPtr> RelationNameMap;
			
			/// ID to Relation instance. Secondary storage of Relation instances, mapped per request when loading
			typedef std::map<int, RelationPtr> RelationIDMap;
			
			FlavorToName mFlavorToName;
			NameToFlavor mNameToFlavor;
			RelationIDMap mRelationIDMap;
			RelationNameMap mRelationNameMap;
			
			/// Relations chunk versions
			uint mRelVMaj, mRelVMin;
	};
	
	/// Shared pointer to Link service
	typedef shared_ptr<LinkService> LinkServicePtr;
	
	/// Factory for the LinkService objects
	class LinkServiceFactory : public ServiceFactory {
		public:
			LinkServiceFactory();
			~LinkServiceFactory() {};
			
			/** Creates a LinkService instance */
			Service* createInstance(ServiceManager* manager);
			
			virtual const std::string& getName();
		
		private:
			static std::string mName;
	};
}
 
 
#endif
