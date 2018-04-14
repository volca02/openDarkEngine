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

#ifndef __ROOMSERVICE_H
#define __ROOMSERVICE_H

#include "FileGroup.h"
#include "OpdeService.h"
#include "OpdeServiceManager.h"
#include "RoomCommon.h"
#include "SharedPtr.h"
#include "Vector3.h"
#include "config.h"
#include "database/DatabaseService.h"

namespace Opde {
/** @brief Room service - service providing a Room database.
 * Room database is a high level system that classifies object positions into
 * 'rooms'.
 * @note This service listens to object positions
 * @note This service is a source of script messages
 */
class OPDELIB_EXPORT RoomService : public ServiceImpl<RoomService>,
                                   DatabaseListener {
public:
    /// Constructor
    RoomService(ServiceManager *manager, const std::string &name);

    /// Destructor
    virtual ~RoomService();

    Room *getRoomByID(int32_t id);

    /** Reassigns the object's room if a room is sucessfully found
     * @param idset The id set to use (typically 0-objects, 1-AI)
     * @param objID The id of the object to track down
     * @param pos The position of the object
     * @return Room pointer if room was found, NULL otherwise
     */
    Room *findObjRoom(size_t idset, int objID, const Vector3 &pos);

    /** Finds a room which encloses the specified point
     * @param pos The position to locate room for
     * @return Room pointer if the position is enclosed in a room, NULL
     * otherwise
     */
    Room *roomFromPoint(const Vector3 &pos);

    /** Updates the object's room (preferably incrementally without doing room
     * search)
     * @param idset The id set to use (typically 0-objects, 1-AI)
     * @param objID The id of the object to track down
     * @param pos The position of the object
     */
    void updateObjRoom(size_t idset, int objID, const Vector3 &pos);

    /** Attaches the specified object to a room instance
     * @param idset the object id set
     * @param objID the object to attach
     * @param room the room to attach the object to
     * */
    void _attachObjRoom(size_t idset, int objID, Room *room);

    /** Detaches the object from a room (be it attached to some)
     * @param idset the object id set
     * @param objID the object id to track down and detach
     * @param current the current room, or NULL if it should be searched for
     */
    void _detachObjRoom(size_t idset, int objID, Room *current = NULL);

    /** Returns the current room the object is attached to
     * @param idset the object id set
     * @param objID the object id
     * @return Room pointer or NULL if the object is not attached
     * */
    Room *getCurrentObjRoom(size_t idset, int objID) const;

protected:
    void setCurrentObjRoom(size_t idset, int objID, Room *room);

    // service related
    bool init();
    void bootstrapFinished();
    void shutdown();

    void clear();

    /** Database load callback
     * @see DatabaseListener::onDBLoad */
    void onDBLoad(const FileGroupPtr &db, uint32_t curmask);

    /** Database save callback
     * @see DatabaseListener::onDBSave */
    void onDBSave(const FileGroupPtr &db, uint32_t tgtmask);

    /** Database drop callback
     * @see DatabaseListener::onDBDrop */
    void onDBDrop(uint32_t dropmask);

private:
    typedef std::map<int32_t, Room *> RoomsByID;
    typedef std::vector<RoomsByID> ObjectIDSets;

    /// Database service
    DatabaseServicePtr mDbService;

    /// Array of all rooms
    SimpleArray<Room *> mRooms;

    /// Map of rooms by their ID
    RoomsByID mRoomsByID;

    /// Indicates the room database is loaded/ok
    bool mRoomsOk;

    /// Sets of object id's
    ObjectIDSets mIDSets;
};

/// Shared pointer to Room service
typedef shared_ptr<RoomService> RoomServicePtr;

/// Factory for the RoomService objects
class OPDELIB_EXPORT RoomServiceFactory : public ServiceFactory {
public:
    RoomServiceFactory();
    ~RoomServiceFactory(){};

    /** Creates a RoomService instance */
    Service *createInstance(ServiceManager *manager);

    virtual const std::string &getName();

    virtual const uint getMask();

    virtual const size_t getSID();

private:
    static std::string mName;
};
} // namespace Opde

#endif
