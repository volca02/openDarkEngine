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


#include "Room.h"
#include "FileCompat.h"
#include "RoomPortal.h"

namespace Opde {
	/*----------------------------------------------------*/
	/*------------------------ Room ----------------------*/
	/*----------------------------------------------------*/
	Room::Room(RoomService* owner) :
			mOwner(owner),
			mObjectID(0),
			mRoomID(0),
			mCenter(),
			mPortalCount(0),
			mPortals(),
			mPortalDistances(NULL) {
		// nothing
	}

	//------------------------------------------------------
	Room::~Room() {
		clear();
	}

	//------------------------------------------------------
	void Room::read(const FilePtr& sf) {
		clear();
		
		*sf >> mObjectID >> mRoomID >> mCenter;
		
		for (size_t i = 0; i < 6; ++i) {
			*sf >> mPlanes[i];
		}
		
		*sf >> mPortalCount;
		
		// read the room portals
		mPortals.grow(mPortalCount);
		for (size_t i = 0; i < mPortalCount; ++i) {
			mPortals[i] = new RoomPortal(mOwner);
			mPortals[i]->read(sf);
		}
		
		// the portal-to-portal distances
		delete[] mPortalDistances;
		mPortalDistances = new float[mPortalCount * mPortalCount];
		sf->readElem(mPortalDistances, sizeof(float), mPortalCount * mPortalCount);

		// the object ID lists
		// there are usually two lists of ID's in the room database
		// This could have worked based on some subscription system of id->location providers
		uint32_t num_lists;
		*sf >> num_lists;
		
        for (size_t li = 0; li < num_lists; ++li) {
            // read the list
        	IDSet ids = IDSet();
            
            uint32_t count;
            *sf >> count;
            
            for (size_t i = 0; i < count; ++i) {
                int32_t id;
                *sf >> id;
                
                ids.insert(id);
            }
            
			mIDLists.push_back(ids);
        }
	}
	
	//------------------------------------------------------
	void Room::write(const FilePtr& sf) {
		*sf << mObjectID << mRoomID << mCenter;
		
		for (size_t i = 0; i < 6; ++i) {
			*sf << mPlanes[i];
		}
		
		// write the portals
		*sf << mPortalCount;
		
		for (size_t i = 0; i < mPortalCount; ++i) {
			mPortals[i]->write(sf);
		}
		
		// the portal-to-portal distances
		sf->writeElem(mPortalDistances, sizeof(float), mPortalCount * mPortalCount);
		
		// the object ID lists
		uint32_t num_lists = mIDLists.size();
		*sf << num_lists;
		
        for (size_t li = 0; li < num_lists; ++li) {
            // read the list
            IDSet ids = mIDLists[li];
            
            uint32_t count = ids.size();
            *sf << count;
            
            IDSet::iterator i = ids.begin();
            
            for (; i != ids.end(); ++i) {
                int32_t id = *i;
                *sf << id;
            }
        }
	}
	
	//------------------------------------------------------
	void Room::clear() {
		mObjectID = 0;
		mRoomID = 0;
		mCenter = Ogre::Vector3::ZERO;
		
		for (size_t i = 0; i < mPortalCount; ++i)
			delete mPortals[i];
		
		mPortals.clear();
		mPortalCount = 0;
		delete[] mPortalDistances;
		mPortalDistances = NULL;
		
		mIDLists.clear();
	}
	
	
}

