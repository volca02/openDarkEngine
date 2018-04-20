/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
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
 *
 *****************************************************************************/

#ifndef __OBJECTCOMMON_H
#define __OBJECTCOMMON_H

namespace Opde {

/// Object system broadcasted message types
typedef enum {
    /// Object is starting to be created (only basic initialization happened).
    /// Used to initialize depending services
    OBJ_CREATE_STARTED = 1,
    /// Object was created (Including all links and properties)
    OBJ_CREATED,
    /// Object was destroyed (Including all links and properties)
    OBJ_DESTROYED,
    /// All objects were destroyed
    OBJ_SYSTEM_CLEARED,
    /// New min/max range for object ID's was supplied
    OBJ_ID_RANGE_CHANGED

} ObjectServiceMessageType;

/// Message from object service - object was created/destroyed, etc
struct ObjectServiceMsg {
    /// Type of the message that happened
    ObjectServiceMessageType type;
    /// Object id that the message is informing about (not valid for
    /// OBJ_SYSTEM_CLEARED). Minimal object id for OBJ_ID_RANGE_CHANGED
    int objectID;
    /// The Maximal id to hold. Only for OBJ_ID_RANGE_CHANGED event, otherwise
    /// undefined
    int maxObjID;
};

} // namespace Opde

#endif /* __OBJECTCOMMON_H */
