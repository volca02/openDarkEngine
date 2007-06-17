/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2007 openDarkEngine team
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 * http://www.gnu.org/copyleft/lesser.txt.
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
			
			/** Load the links from the database and it's parents */ 
			void load(DarkFileGroup* db);
			
			/** Convert the relation name to a flavour */
			int nameToFlavour(const std::string& name);
			
			/** Creates a relation type (link kind)
			* @param id The ID the relation will have (>0)
			* @param name The relation name
			* @param type The type defining the data format for link data 
			* @param hidden The hidden relations (true) will not show up on public link list places */
			void createRelation(const std::string& name, DTypeDefPtr type, bool hidden);
			
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
			void _load(DarkFileGroup* db);
			
			/** Clears all the data and the relation mappings */
			void _clear();
			
			/** request a mapping Name->Flavour and reverse 
			* @param id The flavour value requested
			* @param name The name for that flavour (Relation name)
			* @param rel The relation instance to associate with that id
			* @return false if conflict happened, true if all went ok, and new mapping is inserted (or already was registered) 
			*/
			bool requestRelationFlavourMap(int id, const std::string& name, RelationPtr rel);
			
			typedef std::map<int, std::string> FlavourToName;
			typedef std::map<std::string, int> NameToFlavour;
			
			/// Name to Relation instance. The primary storage of Relation instances.
			typedef std::map<std::string, RelationPtr> RelationNameMap;
			
			/// ID to Relation instance. Secondary storage of Relation instances, mapped per request when loading
			typedef std::map<int, RelationPtr> RelationIDMap;
			
			FlavourToName mFlavourToName;
			NameToFlavour mNameToFlavour;
			RelationIDMap mRelationIDMap;
			RelationNameMap mRelationNameMap;
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
