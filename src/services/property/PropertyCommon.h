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

#ifndef __PROPERTYCOMMON_H
#define __PROPERTYCOMMON_H

#include "compat.h"

namespace Opde {

/// Property change types
enum PropertyChangeType {
    /// Link was added (Sent after the addition)
    PROP_ADDED = 1,
    /// Link was removed (Sent before the removal)
    PROP_REMOVED,
    /// Link has changed data (Sent after the change)
    PROP_CHANGED,
    /// All the links were removed from the relation (Sent before the cleanout)
    PROP_CLEARED
};

/// Property chage message
struct PropertyChangeMsg {
    /// A change that happened
    PropertyChangeType change;

    /// An ID of the object that changed
    int objectID;
};
} // namespace Opde

#endif
