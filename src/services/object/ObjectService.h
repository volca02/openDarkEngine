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

#include "config.h"

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "DatabaseService.h"
#include "MessageSource.h"

#include "LinkService.h"
#include "InheritService.h"
#include "PropertyService.h"

#include "SymNamePropertyStorage.h"
#include "PositionPropertyStorage.h"

#include "BitArray.h"

#include <OgreSceneManager.h>

#include <stack>

namespace Opde {
	/// Object system broadcasted message types
	typedef enum {
		/// Object is starting to be created (only basic initialization happened). Used to initialize depending services
		OBJ_CREATE_STARTED = 1,
		/// Object was created (Including all links and properties)
		OBJ_CREATED,
		/// Object was destroyed (Including all links and properties)
		OBJ_DESTROYED,
		/// All objects were destroyed
		OBJ_SYSTEM_CLEARED,
		/// New min/max range for object ID's was supplied
		OBJ_ID_RANGE_CHANGED

	} ObjectServiceMessageType;

	/// Message from object service - object was created/destroyed, etc
	struct ObjectServiceMsg {
		/// Type of the message that happened
		ObjectServiceMessageType type;
		/// Object id that the message is informing about (not valid for OBJ_SYSTEM_CLEARED). Minimal object id for OBJ_ID_RANGE_CHANGED
		int objectID;
		/// The Maximal id to hold. Only for OBJ_ID_RANGE_CHANGED event, otherwise undefined
		int maxObjID;
	};

	/** @brief Object service - service managing in-game objects. Holder of the object's scene nodes
	* @todo Archetype object creation
	* @todo DonorType property management (together with Inherit service)
	* @todo Object queries - getAllConcrete, getAllArchetype, getMetapropertiesOf, etc. ObjectQuery class introduced
	* @todo FindClosestObjectNamed (Based on archetype name!), RenderedThisFrame (based on entity listener. This one should be implemented in RenderService)
	* @todo Transient objects - are those needed? If so, implement (This one will be implemented on demand, as I currently can't see a reason to do those)
	* @todo inheritsFrom - good to have query. Should only search archetypes, not metaprops
	* @todo OBJECT_CREATE_STARTED message type for those services needing a preparation for object creation
		(for example render service will create a SceneNode at that time, so it can update it's position and orientation when loading Position properties)
	*/
	class OPDELIB_EXPORT ObjectService : public ServiceImpl<ObjectService>, public MessageSource<ObjectServiceMsg> {
		public:
			ObjectService(ServiceManager *manager, const std::string& name);

			virtual ~ObjectService();

			/** Creates a new concrete object, inheriting from archetype archetype, and returns it's id
			@param archetype The archetype object ID
			@return int the new object's id
			*/
			int create(int archetype);

			/// Begins creating a new object, but does not broadcast yet.
			/// Good to use when wanting to prevent broadcasting of the creation before the properties/links are set
			int beginCreate(int archetype);

			/// finalises the creation if the given object (only broadcasts that object was created)
			void endCreate(int objID);

			/// Returns true if the object exists, false otherwise
			bool exists(int objID);

			/// @returns Object's position or 0,0,0 if the object is abstract or has no position yet (while loading f.e.)
			Vector3 position(int objID);

			/// @returns Object's orientation or Quaternion::IDENTITY if the object has no orientation
			Quaternion orientation(int objID);

			/** Gets the object's symbolic name
			@param objID the object id to get the name of
			@return the name of the object, or empty string if object id is invalid
			*/
			std::string getName(int objID);

			/** Sets the object's symbolic name
			@param objID the object id to set the name for
			@param name The new name of the object
			*/
			void setName(int objID, const std::string& name);

			/** Finds an object ID with the given symbolic name
			@param The id of the object, or zero if it was not found
			@return id of the object, or zero if object was not found
			*/
			int named(const std::string& name);

			/** Teleports an object to new position and orientation
			* @param pos the new position
			* @param ori the new orientation
			* @param relative If true, the position and orientation are taken as differences rather than absolute positions
			*/
			void teleport(int id, const Vector3& pos, const Quaternion& ori, bool relative);

			/** Assigns a metaproperty to an object
			* @param id The object id to assign the metaproperty to
			* @param mpName the name of the metaproperty to assign
			* @return 1 if sucessful, 0 on error */
			int addMetaProperty(int id, const std::string& mpName);

			/** Removes a metaproperty from an object
			* @param id The object id to assign the metaproperty to
			* @param mpName the name of the metaproperty to assign
			* @return 1 if sucessful, 0 on error */
			int removeMetaProperty(int id, const std::string& mpName);

			/** Determines if the given object has a metaproperty assigned
			* @param id the id of the object
			* @param mpName the name of the metaproperty to look for
			* @return true if the object inherits from the given metaproperty, false otherwise
			*/
			bool hasMetaProperty(int id, const std::string& mpName);

			/** Grows the whole object system to allow the storage of the given range of object ID's
			* The id range has to be greater than the old one (no object id removal allowed)
			*/
			void grow(int minID, int maxID);

		protected:
			bool init();

			void bootstrapFinished();

			void shutdown();

			/** Database change callback */
			void onDBChange(const DatabaseChangeMsg& m);

			/** load objects from a single database */
			void _load(const FileGroupPtr& db, uint clearMask);

			/** Saves the objects according to the saveMask */
			void _save(const FileGroupPtr& db, uint saveMask);

			/** Clears all the data (whole object system) */
			void _clear(uint clearMask);

			/// Begins the creation of an object. Prepares properties and links as needed
			void _beginCreateObject(int objID, int archetypeID);

			/// Ends the creation of an object. Broadcasts the change
			void _endCreateObject(int objID);

			/// Destroys object, frees accompanying links and properties, broadcasts the change
			void _destroyObject(int objID);

			/// Prepares for object. Does necessary steps to fully accept the object. informs all the listeners that object creation starts (f.e. creates SceneNode for concrete objects)
			void _prepareForObject(int objID);

			/// Returns a new free archetype ID. Modifies the mMinID/mMaxID as appropriate, removes the ID from the free id's
			int getFreeID(bool archetype);

			/// Frees object ID for later use
			void freeID(int objID);

			/// Resets the min and max id's to some sane values (uses config service values obj_min and obj_max if found)
			void resetMinMaxID();

			/// Creates built-in resources - DonorType property, SymbolicName property
			void createBuiltinResources();

			/// Converts the properties orientation to quaternion
			static Ogre::Quaternion toOrientation(PropertyDataPtr& posdata);

			/// A stack of id's
			typedef std::stack<int> ObjectIDStack;

			/// A set of allocated objects
			BitArray mAllocatedObjects;

			/// Position property (to set/get position/orientation of objects)
			PropertyGroup* mPropPosition;

			/// Map's symbolic name to object ID

			/// Database callback
			DatabaseService::ListenerPtr mDbCallback;

			/// Database service
			DatabaseServicePtr mDatabaseService;

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

			/// Symbolic name property pointer
			PropertyGroup* mPropSymName;
			/// Donor type property pointer
			PropertyGroup* mPropDonorType;

			/// Scene manager pointer
			Ogre::SceneManager* mSceneMgr;

			/// Our specialized property storage for symbolic names
			SymNamePropertyStoragePtr mSymNameStorage;

			/// Specialized storage for position struct
			PositionPropertyStoragePtr mPositionStorage;
	};

	/// Shared pointer to the Object service
	typedef shared_ptr<ObjectService> ObjectServicePtr;

	/// Factory for the ObjectService objects
	class OPDELIB_EXPORT ObjectServiceFactory : public ServiceFactory {
		public:
			ObjectServiceFactory();
			~ObjectServiceFactory() {};

			/** Creates a ObjectService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

			virtual const uint getMask();
			
			virtual const size_t getSID();

		private:
			static std::string mName;
	};
}


#endif
