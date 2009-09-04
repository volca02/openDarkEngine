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


#ifndef __ROOMCOMMON_H
#define __ROOMCOMMON_H

namespace Opde {
	// Forward declaration
	class RoomService;
	class Room;
	class RoomPortal;

	/* Room service structures - as specified by Telliamed */
	/*struct DarkDBChunkROOM_DB
	{
        	uint32_t  version;        // 0x01
	        uint32_t  num_rooms;
		
		void read(const FilePtr& f) {
			f->read(&version, sizeof(uint32_t));
			f->read(&num_rooms, sizeof(uint32_t));
		}
		
		void write(const FilePtr& f) {
			f->write(&version, sizeof(uint32_t));
			f->write(&num_rooms, sizeof(uint32_t));
		}
	};*/
	
/*	struct DarkDBRoom
	{
        	sint32_t  id;             // object ID
	        sint16_t  room;           // room number
	        DVector3  center;         // should not be in solid space or overlapping another room
	        DPlane    plane[6];
	        uint32_t  num_portals;
        	//DarkDBRoomPortal portals[]
	        // A matrix of portal-to-portal distances, calculated as the linear distance
	        // between each pair of portal centers.
	        //float distances[] // num_portals**2
	        //uint32        num_lists // The number of count+arrays that follow. Usually 2.
	        //DarkDBIDList lists[]
	        // There are usually two lists.
	        // The first is the IDs of all objects that are in this room.
	        // The second is the object IDs of creatures in this room.
	        // (The IDs should be in both lists, unless you know of an AI that isn't an object.)
		
		void read(const FilePtr& f) {
			f->read(&id, sizeof(uint32_t));
			f->read(&room, sizeof(sint16_t));
			center.read(f);
			
			for (size_t i = 0; i < 6; ++i)
				plane[i].read(f);
			
			f->read(&num_portals, sizeof(uint32_t));
		}
		
		void write(const FilePtr& f) {
			f->write(&id, sizeof(uint32_t));
			f->write(&room, sizeof(sint16_t));
			center.write(f);
			
			for (size_t i = 0; i < 6; ++i)
				plane[i].write(f);
			
			f->write(&num_portals, sizeof(uint32_t));
		}
	};
 
	struct DarkDBRoomPortal
	{
        	sint32_t  id;             // portal ID
	        uint32_t  index;          // The index of this portal in the room's portal list
	        DPlane    plane;
	        uint32_t  num_edges;
	        
	        Plane*    edge; // all planes follow at this point (num_edges)
		
	        sint32_t  dest_room;      // room number this portal goes to
	        sint32_t  src_room;       // this room number
        	DVector3  center;         // center point of the portal. (should not be in solid space)
	        sint32_t  dest_portal;    // portal ID on the other side of this portal
		
		DarkDBRoomPortal() {
			id = -1;
			index = 0;
			num_edges = 0;
			edge = NULL;
			dest_room = -1;
			src_room = -1;
			dest_portal = -1;
		}
		
		~DarkDBRoomPortal() {
			delete[] edge;
		}
		
		void read(const FilePtr& f) {
			f->read(&id, sizeof(sint32_t));
			f->read(&index, sizeof(uint32_t));
			plane.read(f);
			
			f->read(&num_edges, sizeof(uint32_t));

			delete[] edge; // to be sure
			edge = new DPlane[num_edges];
			for (size_t i = 0; i < num_edges; ++i)
				edge[i].read(f);
			
			// tail
			f->read(&dest_room, sizeof(sint32_t));
			f->read(&src_room, sizeof(sint32_t));
			
			center.read(f);
			
			f->read(&dest_portal, sizeof(sint32_t));
		}
		
		void write(const FilePtr& f) {
			f->write(&id, sizeof(sint32_t));
			f->write(&index, sizeof(uint32_t));
			plane.write(f);
			
			f->write(&num_edges, sizeof(uint32_t));

			assert(edge);
			for (size_t i = 0; i < num_edges; ++i)
				edge[i].write(f);
			
			// tail
			f->write(&dest_room, sizeof(sint32_t));
			f->write(&src_room, sizeof(sint32_t));
			
			center.write(f);
			
			f->write(&dest_portal, sizeof(sint32_t));
		}
	};
	*/
}


#endif
