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
		/// Link was added
		LNK_ADDED = 1,
		/// Link was removed
		LNK_REMOVED, 
		/// Link has changed data
		LNK_CHANGED
		/* If considered a good idea to stream the clears through the callbacks, implement this one too:
		/// All the links were removed from the relation
		LNK_RELATION_CLEARED
		*/
	} LinkChangeType;
	
	/// Link chage message
	typedef struct LinkChangeMsg {
		/// A change that happened
		LinkChangeType change;
		/// An ID of link that was added/removed/modified
		link_id_t linkID;
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
			uint mFlavour;
		
			char* mData;
			
			// DTypeDefPtr would be nice here, but I guess it would be too much waste to do
		public:
			Link(uint ID, int src, int dst, uint flavour, char* data = NULL) 
				: mID(ID), 
				mSrc(src), 
				mDst(dst), 
				mFlavour(flavour), 
				mData(NULL) {	};
			
			~Link() {
				delete mData;
			};
	};
	
	typedef shared_ptr< Link > LinkPtr;
	
	// Create a link ID from flavour, concreteness and index
#define LINK_MAKE_ID(flavor, concrete, index) (flavor<<20 | concrete << 16 | index)
#define LINK_ID_FLAVOR(id) (id >> 20)
#define LINK_ID_CONCRETE(id) (id >>16 & 0x0F)
#define LINK_ID_INDEX(id) (id & 0x0FFFF)
}
 
#endif
