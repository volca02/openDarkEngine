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


#include "PhysicsService.h"
#include "OpdeException.h"
#include "ServiceCommon.h"
#include "logger.h"
#include "OpdeServiceManager.h"
#include "database/DatabaseService.h"

#include "PhysModel.h"
#include "PhysModels.h"

using namespace std;

namespace Opde {

/*----------------------------------------------------*/
/*-------------------- Physics Service ---------------*/
/*----------------------------------------------------*/
template <>
const size_t ServiceImpl<PhysicsService>::SID = __SERVICE_ID_PHYSICS;

PhysicsService::PhysicsService(ServiceManager *manager, const std::string &name)
    : ServiceImpl<Opde::PhysicsService>(manager, name), mPhysModels(this)
{
}

//------------------------------------------------------
bool PhysicsService::init()
{
    mDbService = GET_SERVICE(DatabaseService);

    if (!mDbService)
        return false;

    // create world
    /*
    dInitODE2(0);
    dInitODE();
    dDarkWorldID = dWorldCreate();
    */

    return true;
}

//------------------------------------------------------
void PhysicsService::onDBLoad(const FileGroupPtr &db, uint32_t curmask)
{
    // no phys data? skip loading
    if (!(curmask & DBM_MIS_DATA))
        return;

    //
    FilePtr ph = db->getFile("PHYS_SYSTEM");

    *ph >> mPhysVersion;

    // load all the models
    if (mPhysVersion >= 5) {
        mPhysModels.read(ph, mPhysVersion);
    }

    /* TODO:
    // load all the contacts
    if (mPhysVersion >= 27) {
            mPhysContacts.load(ph);
    }
    */

    // TODO: Subscribe service (ver. >=17)?

    STUB_WARN();
};

//------------------------------------------------------
void PhysicsService::onDBSave(const FileGroupPtr &db, uint32_t tgtmask)
{
    if (!(tgtmask & DBM_MIS_DATA))
        return;

    FilePtr ph = db->getFile("PHYS_SYSTEM");

    *ph >> mPhysVersion;

    // load all the models
    if (mPhysVersion >= 5) {
        mPhysModels.write(ph, mPhysVersion);
    }

    /* TODO:
    // load all the contacts
    if (mPhysVersion >= 27) {
            mPhysContacts.save(ph);
    }
    */

    STUB_WARN();
};

//------------------------------------------------------
void PhysicsService::onDBDrop(uint32_t dropmask)
{
    if (!(dropmask & DBM_MIS_DATA))
        return;

    // do we fit into the drop mask?
    mPhysModels.clear();
    /* TODO: mPhysContacts.clear();
     */

    STUB_WARN();
};

//------------------------------------------------------
PhysicsService::~PhysicsService()
{
    /*
    dWorldDestroy(dDarkWorldID);
    dCloseODE();
    */
}

//------------------------------------------------------
bool PhysicsService::objHasPhysics(int objID)
{
    // TODO: Stub. Code
    STUB_WARN();

    // if we have model, we have physics

    return false;
}

//------------------------------------------------------
size_t PhysicsService::getSubModelCount(int objID)
{
    // TODO: Stub. Code
    STUB_WARN();
    return 0;
}

//------------------------------------------------------
const Vector3 &PhysicsService::getSubModelPosition(int objId, size_t submdl)
{
    // TODO: Stub. Code
    STUB_WARN();
    return Vector3::ZERO;
}

//------------------------------------------------------
const Quaternion &PhysicsService::getSubModelOrientation(int objId,
                                                         size_t submdl)
{
    // TODO: Stub. Code
    STUB_WARN();
    return Quaternion::IDENTITY;
}

//------------------------------------------------------
void PhysicsService::setSubModelOrientation(int objId, size_t submdl,
                                            const Quaternion &rot)
{
    // TODO: Stub. Code
    STUB_WARN();
}

//-------------------------- Factory implementation
const std::string PhysicsServiceFactory::mName = "PhysicsService";

PhysicsServiceFactory::PhysicsServiceFactory() : ServiceFactory(){};

const std::string &PhysicsServiceFactory::getName() { return mName; }

const uint PhysicsServiceFactory::getMask() { return SERVICE_ENGINE; }

const size_t PhysicsServiceFactory::getSID() { return PhysicsService::SID; }

Service *PhysicsServiceFactory::createInstance(ServiceManager *manager)
{
    return new PhysicsService(manager, mName);
}

} // namespace Opde
