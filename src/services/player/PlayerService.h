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
 *
 *		$Id$
 *
 *****************************************************************************/

/**
 @file PlayerService.h
 @brief Player service implementation. Handles player's input and movement
*/

#ifndef __PLAYERSERVICE_H
#define __PLAYERSERVICE_H

#include "config.h"

#include "OpdeService.h"
#include "OpdeServiceFactory.h"
#include "ServiceCommon.h"
#include "SharedPtr.h"
#include "link/LinkCommon.h"
#include "input/InputCommon.h"
#include "sim/SimCommon.h"

namespace Opde {

/** @brief Player service. Service that handles the player object (and input)
 */
class OPDELIB_EXPORT PlayerService : public ServiceImpl<PlayerService>,
                                     public SimListener {
public:
    PlayerService(ServiceManager *manager, const std::string &name);

    virtual ~PlayerService();

    int getPlayerObject();

    void handleCameraAttachment(int objID);

    virtual void simStep(float simTime, float delta);

    size_t getPlayerHeadSubModel(void) const;

    /** Returns the state of the creepOn modifier (slower movement speed) */
    bool getCreepOn(void) const { return mCreepOn; };

protected:
    bool init();
    void bootstrapFinished();
    void shutdown();

    void onInputForward(const InputEventMsg &msg);
    void onInputSidestep(const InputEventMsg &msg);
    void onInputCreepOn(const InputEventMsg &msg);

private:
    /// Object service ptr
    ObjectServicePtr mObjSrv;
    InputServicePtr mInputSrv;
    LinkServicePtr mLinkSrv;
    int mPlayerObjID;
    RelationPtr mPlayerFactoryRelation;
    PhysicsServicePtr mPhysSrv;
    SimServicePtr mSimSrv;
    float mForwardMovement;
    float mSideMovement;
    bool mCreepOn;
};

/// Shared pointer to Player service
typedef shared_ptr<PlayerService> PlayerServicePtr;

/// Factory for the PlayerService objects
class OPDELIB_EXPORT PlayerServiceFactory : public ServiceFactory {
public:
    PlayerServiceFactory();
    ~PlayerServiceFactory(){};

    /** Creates a PlayerService instance */
    Service *createInstance(ServiceManager *manager);

    const std::string &getName() override;
    const uint getMask() override;
    const size_t getSID() override;

private:
    static const std::string mName;
};
} // namespace Opde

#endif
