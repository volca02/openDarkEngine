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
#include "FileGroup.h"
#include "OpdeException.h"
#include "OpdeServiceManager.h"
#include "Room.h"
#include "RoomPortal.h"
#include "ServiceCommon.h"
#include "database/DatabaseService.h"
#include "logger.h"

namespace Opde {

/*----------------------------------------------------*/
/*-------------------- RoomService -------------------*/
/*----------------------------------------------------*/
template <> const size_t ServiceImpl<RoomService>::SID = __SERVICE_ID_ROOM;

RoomService::RoomService(ServiceManager *manager, const std::string &name)
    : ServiceImpl<Opde::RoomService>(manager, name), mRoomsOk(false) {}

//------------------------------------------------------
RoomService::~RoomService() { clear(); }

//------------------------------------------------------
Room *RoomService::getRoomByID(int32_t id) { return mRoomsByID[id]; }

//------------------------------------------------------
Room *RoomService::findObjRoom(size_t idset, int objID, const Vector3 &pos) {
    Room *r = roomFromPoint(pos);

    if (r) {
        // re-assign the object
        _detachObjRoom(idset, objID, NULL);
        _attachObjRoom(idset, objID, r);
    }

    return r;
}

//------------------------------------------------------
Room *RoomService::roomFromPoint(const Vector3 &pos) {
    for (const std::unique_ptr<Room> &r : mRooms) {
        if (r->isInside(pos))
            return r.get();
    }

    return NULL;
}

//------------------------------------------------------
void RoomService::updateObjRoom(size_t idset, int objID, const Vector3 &pos) {
    // suppose we have a room?
    Room *r = getCurrentObjRoom(idset, objID);

    if (!r) {
        r = findObjRoom(idset, objID, pos);

        // only broadcast the object entered the room
        // TODO: Broadcast the obj. entered
        return;
    }

    // still in room?
    if (r->isInside(pos))
        return;

    // nope. propagate to other via portals?
    RoomPortal *rp = r->getPortalForPoint(pos);

    if (rp) {
        Room *newRoom = rp->getFarRoom();

        // Also not in the dest room?
        if (!newRoom->isInside(pos)) {
            newRoom = findObjRoom(idset, objID, pos);
            // TODO: Broadcast the transition
            return;
        }

        _detachObjRoom(idset, objID, r);
        _attachObjRoom(idset, objID, newRoom);
    }

    // TODO: Else the object is lost in transition, maybe we should detach from
    // current room?
}

//------------------------------------------------------
bool RoomService::init() { return true; }

//------------------------------------------------------
void RoomService::bootstrapFinished() {
    mDbService = GET_SERVICE(DatabaseService);
    mDbService->registerListener(this, DBP_ROOM);
}

//------------------------------------------------------
void RoomService::shutdown() {
    mDbService->unregisterListener(this);
    mDbService.reset();
}

//------------------------------------------------------
void RoomService::clear() {
    mRoomsByID.clear();
    mRooms.clear();
    mRoomsOk = false;
}

//------------------------------------------------------
void RoomService::onDBLoad(const FileGroupPtr &db, uint32_t curmask) {
    LOG_INFO("RoomService::onDBLoad called.");

    if (!(curmask & DBM_OBJTREE_CONCRETE)) // May not be it but seems to fit
        return;

    // to be sure
    clear();

    if (!db->hasFile("ROOM_DB")) {
        LOG_FATAL("RoomService: Database '%d' should contain ROOM_DB by mask, "
                  "but doesn't",
                  db->getName().c_str());
        return;
    }

    // load rooms Ok flag, then room count
    uint32_t roomsOk;
    uint32_t count;

    FilePtr rdb = db->getFile("ROOM_DB");

    *rdb >> roomsOk;

    if (!roomsOk) {
        LOG_ERROR("RoomService: Database '%d' had RoomOK false",
                  db->getName().c_str());
        mRoomsOk = false;
        return;
    }

    mRoomsOk = true;

    *rdb >> count;

    LOG_INFO("RoomService: ROOM_DB contains %u rooms", count);

    // construct array of rooms as needed
    mRooms.resize(count);

    // Two phase load... first we construct them
    for (size_t rn = 0; rn < count; ++rn)
        mRooms[rn].reset(new Room(this));

    // then we load - the two phase construction enables us to link rooms and
    // room portals together directly...
    for (size_t rn = 0; rn < count; ++rn) {
        LOG_DEBUG("RoomService: Loading room %d", rn);
        std::unique_ptr<Room> &r = mRooms[rn];
        assert(r);
        r->read(rdb);
        mRoomsByID[r->getRoomID()] = r.get(); // weak ptrs here
    }
}

//------------------------------------------------------
void RoomService::onDBSave(const FileGroupPtr &db, uint32_t tgtmask) {
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

//------------------------------------------------------
void RoomService::_attachObjRoom(size_t idset, int objID, Room *room) {
    assert(room);
    LOG_VERBOSE("RoomService: IDset %u Object %d attaching (new room %d)",
                idset, objID, (int)room->getRoomID());
    room->attachObj(idset, objID);
    setCurrentObjRoom(idset, objID, room);
    // TODO: listener notification
}

//------------------------------------------------------
void RoomService::_detachObjRoom(size_t idset, int objID, Room *current) {
    if (!current)
        current = getCurrentObjRoom(idset, objID);

    if (current) {
        LOG_VERBOSE("RoomService: IDset %u Object %d detaching (old room %d)",
                    idset, objID, (int)current->getRoomID());

        current->detachObj(idset, objID);

        if (mIDSets.size() > idset) {
            mIDSets[idset].erase(objID);
        }
    }
    // TODO: listener notification
}

//------------------------------------------------------
Room *RoomService::getCurrentObjRoom(size_t idset, int objID) const {
    // get the idset in question
    if (mIDSets.size() > idset) {
        RoomsByID::const_iterator it = mIDSets[idset].find(objID);

        if (it != mIDSets[idset].end())
            return it->second;
    }

    return NULL;
}

//------------------------------------------------------
void RoomService::setCurrentObjRoom(size_t idset, int objID, Room *room) {
    // ensure the idset exists
    if (mIDSets.size() <= idset) {
        mIDSets.resize(idset + 1);
    }

    mIDSets[idset][objID] = room;
}

//-------------------------- Factory implementation
const std::string RoomServiceFactory::mName = "RoomService";

RoomServiceFactory::RoomServiceFactory() : ServiceFactory() {}

const std::string &RoomServiceFactory::getName() { return mName; }

const uint RoomServiceFactory::getMask() { return SERVICE_ENGINE; }

const size_t RoomServiceFactory::getSID() { return RoomService::SID; }

Service *RoomServiceFactory::createInstance(ServiceManager *manager) {
    return new RoomService(manager, mName);
}

} // namespace Opde
