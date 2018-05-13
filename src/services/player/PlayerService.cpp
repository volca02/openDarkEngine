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

#include <functional>

#include "OpdeServiceManager.h"
#include "PlayerService.h"
#include "input/InputService.h"
#include "link/LinkService.h"
#include "link/Relation.h"
#include "logger.h"
#include "object/ObjectService.h"
#include "physics/PhysicsService.h"
#include "sim/SimService.h"

// TODO: The right value
#define PLAYER_HEAD_SUBMODEL 0

namespace Opde {

/*----------------------------------------------------*/
/*-------------------- PlayerService -----------------*/
/*----------------------------------------------------*/
template <> const size_t ServiceImpl<PlayerService>::SID = __SERVICE_ID_PLAYER;

PlayerService::PlayerService(ServiceManager *manager, const std::string &name)
    : ServiceImpl<PlayerService>(manager, name),
      mStartingPointObjID(0),
      mForwardMovement(0.0f),
      mSideMovement(0.0f),
      mCreepOn(false)
{
}

//------------------------------------------------------
PlayerService::~PlayerService() {}

//------------------------------------------------------
int PlayerService::getPlayerObject() { return mPlayerObjID; }

//------------------------------------------------------
void PlayerService::handleCameraAttachment(int objID) {
    // TODO: if this is not player object, disable movement, etc...
    STUB_WARN();
}

//------------------------------------------------------
bool PlayerService::init() { return true; }

//------------------------------------------------------
void PlayerService::bootstrapFinished() {
    mObjSrv = GET_SERVICE(ObjectService);   // what for?
    mInputSrv = GET_SERVICE(InputService);  // for input handling
    mLinkSrv = GET_SERVICE(LinkService);    // for playerfactory relation
    mPhysSrv = GET_SERVICE(PhysicsService); // for axis/velocity controls
    mSimSrv = GET_SERVICE(SimService);      // for input to physics updates

    mPlayerFactoryRelation =
        mLinkSrv->createRelation("PlayerFactory", DataStoragePtr(NULL), false);

    // we have to listen to link events on player factory relation to get star
    // object id
    Relation::ListenerPtr metaPropCallback(
        new ClassCallback<LinkChangeMsg, PlayerService>(
            this, &PlayerService::onLinkPlayerFactoryMsg));
    mPlayerFactoryListenerID =
        mPlayerFactoryRelation->registerListener(metaPropCallback);

    // Forward/Backward keypress
    mInputSrv->registerCommandTrap("forward", [&](const InputEventMsg &msg) {
        mForwardMovement = msg.params.toFloat();
    });

    mInputSrv->registerCommandTrap("sidestep", [&](const InputEventMsg &msg) {
        mSideMovement = msg.params.toFloat();
    });

    mInputSrv->registerCommandTrap("creepon", [&](const InputEventMsg &msg) {
        mCreepOn = msg.params.toBool();
    });

    // TODO: zlook, turn (maybe into the camera service...)

    // Aliases to other commands
    mInputSrv->registerCommandAlias("walkslow", "forward 1.0");
    mInputSrv->registerCommandAlias("walk", "forward 2.0");
    mInputSrv->registerCommandAlias("walkfast", "forward 4.0");

    mInputSrv->registerCommandAlias("backslow", "forward -1.0");
    mInputSrv->registerCommandAlias("back", "forward -2.0");
    mInputSrv->registerCommandAlias("backfast", "forward -4.0");

    mInputSrv->registerCommandAlias("moveleft", "sidestep -1.0");
    mInputSrv->registerCommandAlias("moveleftfast", "sidestep -2.0");

    mInputSrv->registerCommandAlias("moveright", "sidestep 1.0");
    mInputSrv->registerCommandAlias("moverightfast", "sidestep 2.0");

    mInputSrv->registerCommandAlias("turnleft", "turn -1.0");
    mInputSrv->registerCommandAlias("turnleftfast", "turn -2.0");

    mInputSrv->registerCommandAlias("turnright", "turn 1.0");
    mInputSrv->registerCommandAlias("turnrightfast", "turn 2.0");

    mInputSrv->registerCommandAlias("look_up", "zlook 1.0");
    mInputSrv->registerCommandAlias("look_down", "zlook -1.0");

    /* Input service commands handled here (* implemented, - not implemented
     yet):
     * forward
     * sidestep
     - zlook
     - leanleft
     - leanright
     - leanforward
     - crouch
     - jumpblock
     - use_weapon
     - use_item
     - block
     - drop_item
     - cycle_item (int N)
     - clear_weapon
     - inv_select (item name)
     - joyforward
     - joyaxis
     - joysidestep
     - joyturn
     - rudderturn
     - creepon (Modifier)
     - slideon (Modifier)
    */

    // TODO: Physics service calls (axis and velocity controls for mPlayerObjID)
}

//------------------------------------------------------
void PlayerService::shutdown() {
    if (mInputSrv) {
        mInputSrv->unregisterCommandTrap("forward");
        mInputSrv->unregisterCommandTrap("sidestep");
        mInputSrv->unregisterCommandTrap("creepon");
    }

    mObjSrv.reset();
    mInputSrv.reset();
    mLinkSrv.reset();
    mPhysSrv.reset();
    mSimSrv.reset();
}

//------------------------------------------------------
void PlayerService::simStep(float simTime, float delta) {
    // TODO: Only control movement when on ground!
    // TODO: limit values to a sane maximum

    // Process the movement
    // Do we have creepon pushed?
    /*
    float forward = mForwardMovement * (mCreepOn ? 0.5 : 1);
    float sidestep = mSideMovement * (mCreepOn ? 0.5 : 1);
    */

    // Control the objects velocity via phys axis controls
    /*
      if (forward != 0) {
      mPhysSrv->startAxisVelocityControl(mPlayerObjID, PhysicsService::AXIS_X,
      forward * delta); } else { mPhysSrv->stopAxisVelocityControl(mPlayerObjID,
      PhysicsService::AXIS_X);
      }

      if (sidestep != 0) {
      mPhysSrv->startAxisVelocityControl(mPlayerObjID, PhysicsService::AXIS_Y,
      sidestep * delta); } else {
      mPhysSrv->stopAxisVelocityControl(mPlayerObjID, PhysicsService::AXIS_Y);
      }
    */

    // take the current heading vector, apply the displacement
    // This is the current view direction...
    // const Quaternion& ori = mPhysSrv->getSubModelOrientation(mPlayerObjID,
    // PLAYER_HEAD_SUBMDL);

    // mPhysSrv->set
}

//------------------------------------------------------
void PlayerService::onLinkPlayerFactoryMsg(const LinkChangeMsg &msg) {
    if (msg.change == LNK_ADDED) {
        // get the Link ref.
        const Link *l = mPlayerFactoryRelation->getLink(msg.linkID);
        mStartingPointObjID = l->src();
        LOG_INFO("PlayerService: Found startingPoint %d", mStartingPointObjID);
    }
}

//------------------------------------------------------
size_t PlayerService::getPlayerHeadSubModel(void) const {
    return PLAYER_HEAD_SUBMODEL;
}

//-------------------------- Factory implementation
const std::string PlayerServiceFactory::mName = "PlayerService";

PlayerServiceFactory::PlayerServiceFactory() : ServiceFactory(){};

const std::string &PlayerServiceFactory::getName() { return mName; }

const uint PlayerServiceFactory::getMask() { return SERVICE_ENGINE; }

const size_t PlayerServiceFactory::getSID() { return PlayerService::SID; }

Service *PlayerServiceFactory::createInstance(ServiceManager *manager) {
    return new PlayerService(manager, mName);
}

} // namespace Opde
