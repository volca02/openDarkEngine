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

#ifndef __PHYSICSSERVICE_H
#define __PHYSICSSERVICE_H

#include "config.h"

#include "FileGroup.h"
#include "OpdeService.h"
#include "OpdeServiceFactory.h"
#include "ServiceCommon.h"
#include "PhysModels.h"
#include "Quaternion.h"
#include "SharedPtr.h"
#include "Vector3.h"
#include "database/DatabaseCommon.h"

#include <ode/ode.h>

namespace Opde {

/** @brief Physics service - service handling physics (STUB)
 */
class PhysicsService : public ServiceImpl<PhysicsService>,
                       public DatabaseListener {
public:
    /** Constructor */
    PhysicsService(ServiceManager *manager, const std::string &name);

    /** Destructor */
    virtual ~PhysicsService();

    /** Returns true if the given object has physics
     * @param objID the object id
     * @return true if the object is physical
     */
    bool objHasPhysics(int objID);

    /** Returns the count of submodels of the specified object
     * @param objID the object id
     * @return count, or zero if the object does not have any physics
     */
    size_t getSubModelCount(int objID);

    /** Returns the position of given submodel if exists
     * @param objId the object id
     * @param submdl the submodel id
     * @return Position if the given submodel exists, otherwise Vector3::ZERO */
    const Vector3 &getSubModelPosition(int objId, size_t submdl);

    /** Returns the orientation of given submodel if exists
     * @param objId the object id
     * @param submdl the submodel id
     * @return Orientation if the given submodel exists, otherwise
     * Quaternion::IDENTITY */
    const Quaternion &getSubModelOrientation(int objId, size_t submdl);

    /** Sets the given submodel orientation
     * @param objId the object id
     * @param submdl the submodel id
     * @param rot the rotation to set
     * @return true if the value was set */
    void setSubModelOrientation(int objId, size_t submdl,
                                const Quaternion &rot);

protected:
    bool init();

    virtual void onDBLoad(const FileGroupPtr &db, uint32_t curmask);
    virtual void onDBSave(const FileGroupPtr &db, uint32_t tgtmask);
    virtual void onDBDrop(uint32_t dropmask);

    DatabaseServicePtr mDbService;

    /// Current version of the physics tag
    unsigned int mPhysVersion;

    // Dark World
    dWorldID dDarkWorldID;

    /// Collection of all the physical models
    PhysModels mPhysModels;
};
/// Factory for the PhysicsService objects
class PhysicsServiceFactory : public ServiceFactory {
public:
    PhysicsServiceFactory();
    ~PhysicsServiceFactory(){};

    /** Creates a PhysicsService instance */
    Service *createInstance(ServiceManager *manager) override;

    const std::string &getName() override;
    const uint getMask() override;
    const size_t getSID() override;

private:
    static const std::string mName;
};
} // namespace Opde

#endif
