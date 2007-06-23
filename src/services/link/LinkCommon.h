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
 
#ifndef __LINK_H
#define __LINK_H

#include "compat.h"
#include "integers.h"
#include "SharedPtr.h"

namespace Opde {
	/// Link ID type. 32bit number at least...
	typedef unsigned int link_id_t;

	/// Link change types
	typedef enum {
		/// Link was added (Sent after the addition)
		LNK_ADDED = 1,
		/// Link was removed (Sent before the removal)
		LNK_REMOVED, 
		/// Link has changed data (Sent after the change)
		LNK_CHANGED,
		/// All the links were removed from the relation (Sent before the cleanout)
		LNK_RELATION_CLEARED
	} LinkChangeType;
	
	/// Link chage message
	typedef struct LinkChangeMsg {
		/// A change that happened
		LinkChangeType change;
		/// An ID of link that was added/removed/modified
		link_id_t linkID;
		/// TODO: What about adding a linkPtr here?
	};
	
	/** Relation listener abstract class
	* This class is to be inherited by classes wanting to listen to Relation messages */	
	class LinkChangeListener {};
	
	/// Callback method declaration (thanks betajaen for this idea)
	typedef void (LinkChangeListener::*LinkChangeMethodPtr)(const LinkChangeMsg& msg);

	/// Listener pair. LinkChangeListener 
	typedef struct LinkChangeListenerPtr {
		LinkChangeListener* listener;
		LinkChangeMethodPtr method;
	};
	
	/** A link container. Contains source, destination, ID, flavour and link data
	*/
	class Link {
		friend class Relation;
		
		protected:
			link_id_t mID;
			int mSrc;
			int mDst;
			uint mFlavor;
			
			// DTypeDefPtr would be nice here, but I guess it would be too much waste to do
		public:
			Link(uint ID, int src, int dst, uint flavor) 
				: mID(ID), 
				mSrc(src), 
				mDst(dst), 
				mFlavor(flavor) {	};
			
			inline link_id_t id() { return mID; };
	};
	
	/** Link data container. Holds link ID and it's data as a char array.
	*/
	class LinkData {
		friend class Relation;
		
		protected:
			link_id_t mID;
			char* mData;
			size_t mSize;
		
		public:
			/// Constructor - Allocates the mData as a char[size] array
			LinkData(link_id_t id, size_t size) : mID(id), mData(NULL), mSize(size) { mData = new char[size]; };
			
			/// Constructor - Reuses existing data
			LinkData(link_id_t id, char *data, size_t size) : mID(id), mData(data), mSize(size) { };
			
			/// Destructor - Deletes the data
			~LinkData() { delete[] mData; };
	};
	
	typedef shared_ptr< Link > LinkPtr;
	typedef shared_ptr< LinkData > LinkDataPtr;
	
	/// Supportive Link comparison operator for sets and maps
	inline bool operator<(const LinkPtr& a, const LinkPtr& b) {
		return a->id() < b->id();
	}
	
	/** Class representing a link query result.
	*/
	class LinkQueryResult {
			friend class Relation;
			
		protected:
			typedef std::set< LinkPtr > LinkQueryResultSet;
			
		public:	
			typedef LinkQueryResultSet::iterator iterator;
			
		protected:
			LinkQueryResultSet mResultSet;
			
			/** Internal method - pushes a link into the result vector. */
			inline void insert(LinkPtr res) {
				mResultSet.insert(res);
			}
			
			/** Internal method - pushes a link into the result vector. */
			inline void insert(iterator begin, iterator end) {
				mResultSet.insert(begin, end);
			}
			
		public:
			LinkQueryResult() : mResultSet() {};
			~LinkQueryResult() { mResultSet.clear(); };
			
			
			/// Returns the begin iterator of the query result
			inline iterator begin() { return mResultSet.begin(); };
			
			/// Returns the end iterator of the query result
			inline iterator end() { return mResultSet.end(); };
			
			size_t size() { return mResultSet.size(); };
	};
	
	/// Shared pointer instance to link query result
	typedef shared_ptr< LinkQueryResult > LinkQueryResultPtr;
	
/// Creates a link ID from flavour, concreteness and index
#define LINK_MAKE_ID(flavor, concrete, index) (flavor<<20 | concrete << 16 | index)
/// Extracts Flavor ID from the link ID
#define LINK_ID_FLAVOR(id) (id >> 20)
/// Extracts Concreteness from the link ID
#define LINK_ID_CONCRETE(id) (id >>16 & 0x0F)
/// Extracts the index of the link from link ID	
#define LINK_ID_INDEX(id) (id & 0x0FFFF)

// --------- Concreteness of the links -----------------
/// Archetype links (<0) -> (<0)	
#define LINKC_ARCHETYPES	(1 << 0)
/// Links to concrete objects
#define LINKC_CONCRETE	(1 << 1)
/// All link concreteness types	
#define LINKC_ALL	(0x0F)
	
}
 
#endif
