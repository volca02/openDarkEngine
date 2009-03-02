/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
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
 *	  $Id$
 *
 *****************************************************************************/


#ifndef __LINKCOMMON_H
#define __LINKCOMMON_H

#include "compat.h"
#include "integers.h"
#include "SharedPtr.h"
#include "DTypeDef.h"
#include "Iterator.h"

namespace Opde {
	/// Link ID type. 32bit number at least...
	typedef unsigned int link_id_t;

	struct LinkStruct { // sLink-like, but this one contains id as well. Size: Fixed to 14 bytes
		uint32_t id;
		int32_t src;
		int32_t dest;
		uint16_t flavor;
	};

	/** A link container. Contains source, destination, ID, flavor and link data
	*/
	class Link {
		friend class Relation;

		protected:
			link_id_t mID;
			int mSrc;
			int mDst;
			uint mFlavor;

		public:
			Link(uint ID, int src, int dst, uint flavor)
				: mID(ID),
				mSrc(src),
				mDst(dst),
				mFlavor(flavor) {	};

			Link(const LinkStruct& ls) :
				mID(ls.id),
				mSrc(ls.src),
				mDst(ls.dest),
				mFlavor(ls.flavor) {
			}

			/// Copy constructor
			Link(Link& b) :
				mID(b.mID),
				mSrc(b.mSrc),
				mDst(b.mDst),
				mFlavor(b.mFlavor) { };

			inline link_id_t id() { return mID; };
			inline int src() { return mSrc; };
			inline int dst() { return mDst; };
			inline int flavor() { return mFlavor; };
	};

	/// Shared pointer to Link
	typedef shared_ptr< Link > LinkPtr;

	/// Supportive Link comparison operator for sets and maps
	inline bool operator<(const LinkPtr& a, const LinkPtr& b) {
		return a->id() < b->id();
	}

	/// Class representing a link query result.
	typedef ConstIterator< LinkPtr > LinkQueryResult;

	/// Shared pointer instance to link query result
	typedef shared_ptr< LinkQueryResult > LinkQueryResultPtr;


	/// Just an empty result of a query
    class EmptyLinkQueryResult : public LinkQueryResult {
	    public:
            EmptyLinkQueryResult() : LinkQueryResult(), mNullPtr(NULL) {};

            virtual const LinkPtr& next() { return mNullPtr; };

            virtual bool end() const {
                return true;
            };
		
		protected:
			LinkPtr mNullPtr;
	};

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
	struct LinkChangeMsg {
		LinkChangeMsg() : link(NULL) {};
		LinkChangeMsg(const LinkPtr& lnk) : link(lnk) {};
		
		/// A change that happened
		LinkChangeType change;
		/// An ID of link that was added/removed/modified
		link_id_t linkID;
		/// The link itself. Do not modify!
		const LinkPtr link;
	};
	
/// Creates a link ID from flavor, concreteness and index
#define LINK_MAKE_ID(flavor, concrete, index) (((flavor<<20) | (concrete << 16)) | (index))
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
