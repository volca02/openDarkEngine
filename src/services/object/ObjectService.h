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
 *
 *	$Id$
 *
 *****************************************************************************/


#ifndef __OBJECTSERVICE_H
#define __OBJECTSERVICE_H

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "DatabaseService.h"
#include "MessageSource.h"

#include "LinkService.h"
#include "InheritService.h"
#include "PropertyService.h"

#include <stack>

namespace Opde {
	typedef enum {
		// Object was created (Including all links and properties)
		OBJ_CREATED = 1,
		// Object was destroyed (Including all links and properties)
		OBJ_DESTROYED,
		// All objects were destroyed
		OBJ_SYSTEM_CLEARED
		
	} ObjectServiceMessageType;
	
	/// Message from object service - object was created/destroyed, etc
	struct ObjectServiceMsg {
		// Type of the message that happened
		ObjectServiceMessageType type;
		// Object id that the message is informing about (not valid for OBJ_SYSTEM_CLEARED)
		int id;
	};

	/** @brief Object service - service managing in-game objects
	*/
	class ObjectService : public Service, public MessageSource<ObjectServiceMsg> {
		public:
			ObjectService(ServiceManager *manager, const std::string& name);

			virtual ~ObjectService();
			
			/// Creates a new concrete object, inheriting from archetype archetype, and returns it's id
			int createObject(int archetype);
			
			/// Begins creating a new object, but does not broadcast yet. 
			/// Good to use when wanting to prevent broadcasting of the creation before the properties/links are set
			int beginCreateObject(int archetype);
			
			/// finalises the creation if the given object (only broadcasts that object was created)
			int endCreateObject(int objID);

			/// Returns true if the object exists, false otherwise
			bool exists(int objID);
			
		protected:
			bool init();
			void bootstrapFinished();

			/** Database change callback */
			void onDBChange(const DatabaseChangeMsg& m);

			/** load links from a single database */
			void _load(FileGroupPtr db, uint clearMask);

			/** Saves the links and link data according to the saveMask */
			void _save(FileGroupPtr db, uint saveMask);

			/** Clears all the data and the relation mappings */
			void _clear(uint clearMask);
			
			/// Begins the creation of an object. Prepares properties and links as needed
			void _beginCreateObject(int objID, int archetypeID);
			
			/// Ends the creation of an object. Broadcasts the change
			void _endCreateObject(int objID);
			
			/// Destroys object, frees accompanying links and properties, broadcasts the change
			void _destroyObject(int objID);
			
			/// Returns a new free archetype ID. Modifies the mMinID/mMaxID as appropriate, removes the ID from the free id's
			int getFreeID(bool archetype);
			
			/// Frees object ID for later use
			void freeID(int objID);
			
			/// Resets the min and max id's to some sane values (uses config service values obj_min and obj_max if found)
			void resetMinMaxID();

			/** A database of used ID's */
			typedef std::set<int> ObjectAllocation;
			
			/// A stack of id's
			typedef std::stack<int> ObjectIDStack;
			
			/// Object to name map
			typedef std::map<int, std::string> ObjectToName;
			
			/// Name to object map
			typedef std::map<std::string, int> NameToObject;
			
			/// A set of allocated objects
			ObjectAllocation mAllocatedObjects;
			
			/// Database callback
			DatabaseService::ListenerPtr mDbCallback;

			/// Database service
			DatabaseServicePtr mDatabaseService;
			
			// Maximal and minimal used ID's (as read from config service, or from DB's ObjVec)
			/// Minimal ID used for game objects (lower limit, flexible)
			int mMinID, 
			/// Maximal ID used for game objects (upper limit, flexible)
				mMaxID;
			
			/// Chunk versions. TODO: Config Service values
			uint mObjVecVerMaj, mObjVecVerMin;
			
			/// A stack of free object to be used for creation - archetype id's
			ObjectIDStack mFreeArchetypeIDs;
			
			/// A stack of free object to be used for creation - concrete id's
			ObjectIDStack mFreeConcreteIDs;
			
			/// A shared ptr to inherit service
			InheritServicePtr mInheritService;
			
			/// A shared ptr to link service
			LinkServicePtr mLinkService;
			
			/// A shared ptr to property service
			PropertyServicePtr mPropertyService;
	};

	/// Shared pointer to the service
	typedef shared_ptr<ObjectService> ObjectServicePtr;

	/// Factory for the ObjectService objects
	class ObjectServiceFactory : public ServiceFactory {
		public:
			ObjectServiceFactory();
			~ObjectServiceFactory() {};

			/** Creates a LinkService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

			virtual const uint getMask();

		private:
			static std::string mName;
	};
}


#endif
