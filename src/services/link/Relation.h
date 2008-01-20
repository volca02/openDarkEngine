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


#ifndef __RELATION_H
#define __RELATION_H

#include <string>
#include "NonCopyable.h"
#include "DTypeDef.h"
#include "LinkCommon.h"
#include "FileGroup.h"
#include "MessageSource.h"

namespace Opde {
	/** @brief Relation. A store of a group of links of the same flavor.
	*/
	class Relation : public NonCopyable, public MessageSource<LinkChangeMsg> {
		public:
			Relation(const std::string& name, DTypeDefPtr type, bool isInverse, bool hidden = false);
			virtual ~Relation();

			/** Loads the relation data from the given FileGroup */
			void load(FileGroupPtr db);

			/** Saves the relation data to the fiven file group
			* @todo Save Mask implementation */
			void save(FileGroupPtr db, uint saveMask);

			/// @returns the Data Type describing the link data
			DTypeDefPtr getDataType() { return mType; };

			void setChunkVersions(uint lmajor, uint lminor, uint dmajor, uint dminor) {
				mLCVMaj = lmajor;
				mLCVMin = lminor;
				mDCVMaj = dmajor;
				mDCVMin = dminor;
			}

			/// Inverse relation getter. Will return a relation with the links going in opposite direction
			Relation* inverse();
			
			bool isInverse() { return mIsInverse; };
			
			/// Sets a inverse relation to this relation. Can only be done once.
			void setInverseRelation(Relation* rel);

			/** Clears out all the links, releses data, clears query caches */
			void clear();

			/** Sets the ID (flavor) of this Relation. Must be done prior to any operation with the relation instance */
			inline void setID(int id) { mID = id; };

			/** Returns the ID (flavor) of this relation. */
			inline int getID() { return mID; }

			/** Sets a fake size of the link data structure (some relation do not store a real size, but a fake one) */
			void setFakeSize(int size) { assert(size>0); mFakeSize = size; };

			/** @return the name of this relation */
			const std::string& getName() { return mName; };

			// ----------------- Link management methods --------------------
			/** Deletes a link specified by it's ID
			* @param id The ID of the link that should be deleted
			* @note Broadcasts the link removal message (LNK_REMOVED) */
			void remove(link_id_t id);

			/** Creates a new link, returning it's ID
			* @param from The source object ID of this link
			* @param to The destination obect ID of this link
			* @note This version of createLink does initialize the Link data with zero values, if the DTypeDefPtr.isNull() is not true. This is not always the requested bahavior. Use create(int,int,char*) to create a link with already preset data
			* @return the id of the newly created link (The concreteness of the link is autodetected given the from and to values (both < 0 - non concrete, otherwise concrete))
			* @note Broadcasts the link creation message
			* @todo Some relations could be limited to one link between obect pair(!). This could be
			*/
			link_id_t create(int from, int to);

			/** Creates a new link, returning it's ID
			* @see createLink(int, int)
			* @note this version does create the link including the data, so a time and plenty of broadcasts is saved.
			* @note to create and fill the data, use DTypeDef::create to obtain a valid buffer to fill, then use DTypeDef::set to fill the fields. Once ready, supply the final buffer to the createLink
			*/
			link_id_t create(int from, int to, DTypePtr data);

			/** getter for the DTypeDef this relation uses.
			* @note The returned object is better be tested by doing DTypeDefPtr::isNull() to determine if the object is usable. Relations with no data will just return null DTypeDefPtr */
			inline DTypeDefPtr getTypeDef() { return mType; };

			/** Sets the link data field
			* @param id The link id
			* @param field The name of the field the modification is requested on
			* @param DVariant The new value of the field
			* @todo If DTypeDef will implement indexing, create a version with index int as a first parameter
			*/
			void setLinkField(link_id_t id, const std::string& field, const DVariant& value);

			/** Gets the link data field value
			* @param id The link id
			* @param field The name of the field the modification is requested on
			*/
			DVariant getLinkField(link_id_t id, const std::string& field);

			/** Swaps the link data for new data. Deletes the old data
			For more massive data overwriting.
			* @param id The link id
			* @param data The new data pointer
			* @note deallocates the original data. All the references to the original data are then invalid and potentially harmful
			*/
			void setLinkData(link_id_t id, char* data);

			/** Returns link data for the given link ID
			* @param id The link id
			*/
			LinkDataPtr getLinkData(link_id_t id);

			// ----------------- Link query methods --------------------
			/** Gets all links that come from source to destination
			* @param src Source object ID
			* @param dst Destination object ID or 0 if any destination
			* @return LinkQueryResultPtr filled with the query result */
			LinkQueryResultPtr getAllLinks(int src, int dst) const;

			/** Gets single link ID that is coming from source to destination
			* @param src Source object ID, or 0 if any source
			* @param dst Destination object ID or 0 if any destination
			* @return LinkPtr link instance that fulfills the requirements
			* @note only one of the parameters can be zero
			* @throws BasicException if there was more than one link that could be returned
			*/
			LinkPtr getOneLink(int src, int dst) const;

            /** Gets single link given the link ID
			* @param id The link's ID
			* @return LinkPtr link instance that fulfills the requirements, or NULL
			*/
			LinkPtr getLink(link_id_t id) const;
			
			/** Removes all links that connected to a given object ID
			* @param id the object id to remove all links from
			*/
			void objectDestroyed(int id);
			
			/** Sets the relation to cache link data (caches fields so no direct to/from data will be used on loading) 
			* @param cache if true, writes will set a the value in a cache as well, and reads will search cache first
			*/
			void setCacheData(bool cache) { mUseDataCache = cache; };
			
			/** @return true if cache for data is used, false otherwise */
			bool getCacheData() { return mUseDataCache; };

		protected:
            class MultiTargetLinkQueryResult;

			/** Internal method for link insertion. Inserts the link to the map, notifies listeners and query databases
			* @param newlnk The link to be inserted
			* @note Always use this method to internally insert new links, if not in a situation when the standard sequence of link addition is needed (notification, query database refresh)
			* @note The link data have to be assigned prior to calling this method
			*/
			void _addLink(LinkPtr newlnk);

			/** Internal method for link removal handling. Notifies the listeners, refreshes query databases.
			* @param id The id of the link to be removed
			* @note Also removes the link data
			*/
			void _removeLink(link_id_t id);

			/** Returns the id that can be used to create a link (a free ID that is).
			* @param cidx Concreteness index of the requested link (0-15)
			* @note concreteness 0 is usually used for abstract links (both src and dst ID's end up < 0), concreteness 1 is used for links having at least one end in positive object ID space (non abstract objects)
			* @ret
			*/
			link_id_t getFreeLinkID(uint cidx);

			/** Assigns the link data to a link ID
			* @param id Link id to assign the data to
			* @param data The link data to be assigned
			*/
			void _assignLinkData(link_id_t id, LinkDataPtr data);

			/** Removes link data for a link ID
			* @param id the ID of the link for which data should be removed
			*/
			void _removeLinkData(link_id_t id);

			/** Allocates the link ID, meaning it wont be given as free now.
			* @param id The link that was allocated
			* @note This now only increments the maximal index for the concreteness of the link if it is bigger than that
			*/
			void allocateLinkID(link_id_t id);

			/** Unallocates the link ID, meaning it will be available again.
			* @param id The link that was allocated
			* @note This now only decrements the maximal index of the concreteness the id has, if it was the maximal id
			*/
			void unallocateLinkID(link_id_t id);
			
			/** Creates an inverse link for the given link. The links share the same data, but have src and dst object id's swapped
			* @param src The source link
			* @return LinkPtr of the new inverse link
			*/
			LinkPtr createInverseLink(LinkPtr src);
			
			/** internal object destruction handler. @see objectDestroyed */
			void _objectDestroyed(int id);

			/// Map of links. Indexed by whole link id, contains the link class (LinkPtr)
			typedef std::map< link_id_t, LinkPtr > LinkMap;

			/// Map of link data. Indexed by whole link id, contains the link data (LinkDataPtr)
			typedef std::map< link_id_t, LinkDataPtr > LinkDataMap;

			/// Map of all links that have an object ID in either target or source
			typedef std::multimap< int,  LinkPtr > ObjectIDToLinks;

			/// Map of all maps that share a certain object ID
			typedef std::map< int, ObjectIDToLinks > ObjectLinkMap;

			/// Map of links in source object ID, destination object id order
			ObjectLinkMap mSrcDstLinkMap;

			/// ID of this relation (Flavor)
			int mID;

			/// The maximal ID of the given concreteness level (only the index part)
			link_id_t mMaxID[16];

			/// Name of this relation
			std::string mName;

			/// Typedef pointer used by this relation (or null if no data are used)
			DTypeDefPtr mType;

			/// Hidden relations are those which should not be mentioned by editor as a normal links (metaproperty and such)
			bool mHidden;

			/// The map of ID->LinkPtr (Stores link info per link ID)
			LinkMap mLinkMap;

			/// The map of ID->LinkDataPtr (Stores link data per link ID)
			LinkDataMap mLinkDataMap;

			/// fake size. This size is written as the data size into the LD$ chunks
			uint32_t mFakeSize;
			
			/// The pointer to inverse relation
			Relation* mInverse;

			/// This relation is an inverse relation...
			bool mIsInverse;

			/// chunk versions. Both Link and LinkData
			uint mLCVMaj;
			uint mLCVMin;
			uint mDCVMaj;
			uint mDCVMin;
			
			/// If true, data caching will be used
			bool mUseDataCache;

	};


	/// Shared pointer on Relation
	typedef shared_ptr< Relation > RelationPtr;
}

#endif
