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


#include "RoomService.h"
#include "Room.h"
#include "OpdeException.h"
#include "ServiceCommon.h"
#include "logger.h"

using namespace std;

namespace Opde {

	/*----------------------------------------------------*/
	/*-------------------- RoomService -------------------*/
	/*----------------------------------------------------*/
	template<> const size_t ServiceImpl<RoomService>::SID = __SERVICE_ID_ROOM;
	
	RoomService::RoomService(ServiceManager *manager, const std::string& name) : ServiceImpl< Opde::RoomService >(manager, name), mRoomsOk(false) {
	}

	//------------------------------------------------------
	RoomService::~RoomService() {
		clear();
	}

	//------------------------------------------------------
	Room* RoomService::getRoomByID(int32_t id) {
		return mRoomsByID[id];
	}
	
	//------------------------------------------------------
	bool RoomService::init() {
		return true;
	}
	
	//------------------------------------------------------
	void RoomService::bootstrapFinished() {
		mDbService = GET_SERVICE(DatabaseService);
		mDbService->registerListener(this, DBP_ROOM);
	}
	
	//------------------------------------------------------
	void RoomService::shutdown() {
		mDbService->unregisterListener(this);
		mDbService.setNull();
	}
	
	//------------------------------------------------------
	void RoomService::clear() {
		mRoomsByID.clear();
		
		for (size_t rn = 0; rn < mRooms.size(); ++rn) {
			delete mRooms[rn];
		}
		
		mRooms.clear();
		mRoomsOk = false;
	}
	
	//------------------------------------------------------
	void RoomService::onDBLoad(const FileGroupPtr& db, uint32_t curmask) {
		LOG_INFO("RoomService::onDBLoad called.");
		
		if (!(curmask & DBM_OBJTREE_CONCRETE)) // May not be it but seems to fit
			return;
		
		// to be sure
		clear();
		
		if (!db->hasFile("ROOM_DB")) {
		    LOG_FATAL("RoomService: Database '%d' should contain ROOM_DB by mask, but doesn't", db->getName().c_str());
		    return;
		}
		
		// load rooms Ok flag, then room count
		uint32_t roomsOk;
		uint32_t count;
		
		FilePtr rdb = db->getFile("ROOM_DB");
		
		*rdb >> roomsOk;
		
		if (!roomsOk) {
			LOG_ERROR("RoomService: Database '%d' had RoomOK false", db->getName().c_str());
			mRoomsOk = false;
			return;
		}
		
		mRoomsOk = true;
		
		*rdb >> count;
		
		LOG_INFO("RoomService: ROOM_DB contains %u rooms", count);
		
		// construct array of rooms as needed
		mRooms.grow(count);

		// Two phase load... first we construct them
		for (size_t rn = 0; rn < count; ++rn) 
			mRooms[rn] = new Room(this);
		
		// then we load - the two phase construction enables us to link rooms and room portals together directly...
		for (size_t rn = 0; rn < count; ++rn) {
			LOG_DEBUG("RoomService: Loading room %d", rn);
			Room* r = mRooms[rn];
			assert(r != NULL);
			r->read(rdb);
			mRoomsByID[r->getRoomID()] = r;
		}
	}
	
	//------------------------------------------------------
	void RoomService::onDBSave(const FileGroupPtr& db, uint32_t tgtmask) {
		LOG_INFO("RoomService::onDBSave called.");
		
		if (!(tgtmask & DBM_MIS_DATA))
			return;
		
		uint32_t roomsOk = mRoomsOk ? 1 : 0;
		uint32_t count = mRooms.size();
		
		if (!mRoomsOk)
			count = 0;
			
		FilePtr rdb = db->getFile("ROOM_DB");
		*rdb << roomsOk << count;
		
		if (!mRoomsOk)
			return;
		
		for (size_t rn = 0; rn < count; ++rn) {
			mRooms[rn]->write(rdb);
		}
	}
	
	//------------------------------------------------------
	void RoomService::onDBDrop(uint32_t dropmask) {
		LOG_INFO("RoomService::onDBDrop called.");
		
		if (!(dropmask & DBM_MIS_DATA))
			return;
		
		clear();
	}

	//-------------------------- Factory implementation
	std::string RoomServiceFactory::mName = "RoomService";

	RoomServiceFactory::RoomServiceFactory() : ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
	};

	const std::string& RoomServiceFactory::getName() {
		return mName;
	}

	const uint RoomServiceFactory::getMask() {
		return SERVICE_ENGINE;
	}

	const size_t RoomServiceFactory::getSID() {
		return RoomService::SID;
	}

	Service* RoomServiceFactory::createInstance(ServiceManager* manager) {
		return new RoomService(manager, mName);
	}

}
