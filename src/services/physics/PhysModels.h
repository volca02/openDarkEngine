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

#ifndef __PHYSMODELS_H
#define __PHYSMODELS_H

#include "File.h"

namespace Opde {
class PhysicsService;
class PhysModel;

/** @brief Collection of physics models. Contains two lists - Stationary and
 * Moving. Used to contain all the physics models in the mission.
 */
class OPDELIB_EXPORT PhysModels {
public:
    PhysModels(PhysicsService *owner);

    /** Get physics model by object ID, if present.
     * @param objid Object ID
     * @return PhysModel* pointer if exists, NULL otherwise */
    PhysModel *get(int objid) const;

    /** Read all the object phys models from specified tag */
    void read(const FilePtr &tag, unsigned int physVersion);

    /** Write all the object phys models to a specified tag */
    void write(const FilePtr &tag, unsigned int physVersion);

    /** clear the collection, remove all the objects */
    void clear(void);

    /** Adds the model to the list of stationary models */
    void addToStationary(PhysModel *mdl);

    /** @return true if the given object is stationary */
    bool isStationary(int objid);

private:
    typedef std::map<int, PhysModel *> IDModelMap;
    typedef std::set<int> IDSet;

    IDModelMap mModels;
    IDSet mStationaryObjects;

    /// Owning service of this object
    PhysicsService *mOwner;
};
} // namespace Opde

#endif