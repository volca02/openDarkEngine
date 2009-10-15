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
/** @file Room.h
 * @brief A single room in room database.
 */

#ifndef __ROOM_H
#define __ROOM_H

#include "config.h"
#include "integers.h"
#include "SharedPtr.h"
#include "RoomCommon.h"
#include "File.h"
#include "Array.h"

#include <OgreVector3.h>
#include <OgrePlane.h>


namespace Opde {
	/** @brief A single Room. Rooms are space bounded elements that are used
	* for sound propagation, path finding and as a script message sources.
	*/
	class OPDELIB_EXPORT Room {
		public:
			Room(RoomService* owner);
			~Room();
			
			void read(const FilePtr& sf);
			void write(const FilePtr& sf);
			
			int32_t getObjectID() const { return mObjectID; };
			int16_t getRoomID() const { return mRoomID; };
			
			bool isInside(const Ogre::Vector3& point);

		private:
			/// clears the room into an empty state, drops all allocations
			void clear();
			
			/// Owner service
			RoomService* mOwner;
			
			/// Object ID
			int32_t mObjectID;
			/// Room number
			int16_t mRoomID; 
			/// Center point of the room. Should not be in solid space or overlapping another room
			Ogre::Vector3 mCenter;
			/// Bounding box as described by 6 enclosing planes
			Ogre::Plane mPlanes[6];
			/// Portal count
			uint32_t mPortalCount;
			/// Portal list
			SimpleArray<RoomPortal*> mPortals;
			/// Portal to portal distances (a 2d array, single index here for simplicity of allocations)
			float *mPortalDistances;
			
			typedef std::set<int> IDSet;
			typedef std::vector<IDSet> IDLists;
			
			/// lists of ID's
			IDLists mIDLists;
	};

	/// Shared pointer to a room instance
	typedef shared_ptr<Room> RoomPtr;
}


#endif
