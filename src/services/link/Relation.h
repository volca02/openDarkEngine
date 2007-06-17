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
 
#ifndef __RELATION_H
#define __RELATION_H

#include <string>
#include "NonCopyable.h"
#include "DTypeDef.h"
#include "LinkCommon.h"
#include "FileGroup.h"

namespace Opde {
	/** @brief Relation. A store of a group of links of the same flavour.
	*/
	class Relation : public NonCopyable {
		public:
			Relation(const std::string& name, DTypeDefPtr type, bool hidden = false);
			virtual ~Relation();
			
			/** Loads the relation data from the given FileGroup */
			void load(FileGroup* db);
			
			/** Clears out all the links, releses data, clears query caches */
			void clear();

			/** Sets the ID of this Relation. Must be done prior to any operation with the relation instance */			
			inline void setID(int id) { mID = id; };
			
			/** Returns the ID of this relation. */
			inline int getID() { return mID; }


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
			link_id_t create(int from, int to, char* data);
			
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
			
			// ----------------- Link query methods --------------------
			
			
			
			// ----------------- Listener releted methods --------------------			

			/** Registers a relation change listener.
			* @param listener A pointer to LinkChangeListenerPtr
			* @note The same pointer has to be supplied to the unregisterListener in order to succeed with unregistration  
			*/
			void registerListener(LinkChangeListenerPtr* listener);
			
			/** Unregisters a relation change listener.
			* @param listener A pointer to LinkChangeListenerPtr
			* @note The pointer has to be the same as the one supplied to the registerListener (not a big deal, just supply a pointer to a member variable)
			*/
			void unregisterListener(LinkChangeListenerPtr* listener);
			
		protected:
			/// Send a message to all listeners of relation change
			void broadcastLinkMessage(const LinkChangeMsg& msg);

			/** Internal method for link insertion. Inserts the link to the map, notifies listeners and query databases
			* @param newlnk The link to be inserted
			* @note Always use this method to internally insert new links, if not in a situation when the standard sequence of link addition is needed (notification, query database refresh)
			*/
			void _addLink(LinkPtr newlnk);
			
			/** Internal method for link removal handling. Notifies the listeners, refreshes query databases.
			* @param id The id of the link to be removed  
			*/
			void _removeLink(link_id_t id);
			
			/** Returns the id that can be used to create a link (a free ID that is).
			* @param cidx Concreteness index of the requested link (0-15)
			* @note concreteness 0 is usually used for abstract links (both src and dst ID's end up < 0), concreteness 1 is used for links having at least one end in positive object ID space (non abstract objects)
			* @ret
			*/
			link_id_t getFreeLinkID(uint cidx);
			
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
			
			/// Map of links. Indexed by whole link id, contains the link data
			typedef std::map< link_id_t, LinkPtr > LinkMap;
			
			/// A set of listeners
			typedef std::set< LinkChangeListenerPtr* > LinkListeners;
			
			/// ID of this relation (Flavour)
			int mID;
			
			/// The maximal ID of the given concreteness level (only the index part)
			link_id_t mMaxID[16];
			
			/// Name of this relation
			std::string mName;
			
			/// Typedef pointer used by this relation (or null if no data are used)
			DTypeDefPtr mType;
			
			/// Hidden relations are those which should not be mentioned by editor as a normal links (metaproperty and such)
			bool mHidden;
			
			/// The map of ID->linkptr
			LinkMap mLinkMap;
			
			/// Listeners for the link changes
			LinkListeners mLinkListeners;
	};
	
	
	/// Shared pointer on Relation
	typedef shared_ptr< Relation > RelationPtr;
}
 
#endif
