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

#include "CameraService.h"
#include "OpdeServiceManager.h"
#include "ServiceCommon.h"
#include "SharedPtr.h"
#include "input/InputService.h"
#include "logger.h"
#include "physics/PhysicsService.h"
#include "player/PlayerService.h"
#include "property/PropertyService.h"
#include "render/RenderService.h"

using namespace std;
using namespace Ogre;

namespace Opde {

/*----------------------------------------------------*/
/*-------------------- CameraService -----------------*/
/*----------------------------------------------------*/
template <> const size_t ServiceImpl<CameraService>::SID = __SERVICE_ID_CAMERA;

CameraService::CameraService(ServiceManager *manager, const std::string &name)
    : ServiceImpl<CameraService>(manager, name), mHorizontalRot(0),
      mVerticalRot(0), mPaused(false), mAttachmentObject(0),
      mDynamicAttach(false) {}

//------------------------------------------------------
CameraService::~CameraService() {}

//------------------------------------------------------
bool CameraService::staticAttach(int objID) {
    if (mObjSrv->exists(objID)) {
        // TODO: Hide player's limbs and weapon (Via playerservice?)
        handleAttachment(objID, false);
        return true;
    }

    return false;
}

//------------------------------------------------------
bool CameraService::dynamicAttach(int objID) {
    if (mObjSrv->exists(objID)) {
        // TODO: Hide player's limbs and weapon (Via playerservice?)
        handleAttachment(objID, true);
        return true;
    }

    return false;
}

//------------------------------------------------------
bool CameraService::cameraReturn(int objID) {
    if (mAttachmentObject == objID) {
        forceCameraReturn();
        return true;
    }

    return false;
}

//------------------------------------------------------
void CameraService::forceCameraReturn() {
    // TODO: Reveal the player's limbs and weapon again? Conditional?
    handleAttachment(mPlayerSrv->getPlayerObject(), true);
}

//------------------------------------------------------
void CameraService::simPaused() {
    // this means we will not apply any rotation
    mPaused = true;
}

//------------------------------------------------------
void CameraService::simUnPaused() { mPaused = false; }

//------------------------------------------------------
void CameraService::simStep(float simTime, float delta) {
    if (mPaused)
        return;

    // TODO: Handle rotation controlled/not controlled physics flags on the
    // target object!

    // scale
    Vector2 rotation(mHorizontalRot, mVerticalRot);
    rotation /= delta;

    // apply to the object in question
    if (mAttachmentObject != mPlayerSrv->getPlayerObject()) {
        // load the rotation
        Quaternion rot = mObjSrv->orientation(mAttachmentObject);
        // laod the position
        Vector3 pos = mObjSrv->position(mAttachmentObject);

        if (mDynamicAttach) { // we should rotate the object...
            // TODO: Code - apply the rotation on the obj.
            // rot.
        }
    } else {
        //
        // rotate the player object
        // TODO: rotate the camera
        // this is not as easy as it looks. We should take physics into
        // consideration...

        // the player should have some sub-objects present - we only rotate
        // submodel 0 e.g.:

        // first rotate the head sub-object accordingly to the accumulated
        // values
        /* TODO:
           Ogre::Quaternion rot =
           mPhysicsService->getSubModelRotation(mPlayerObject, 0, rot);
           // TODO: multiply by delta
           rot.yaw(rotation.x);
           rot.pitch(rotation.y);

           mPhysicsService->setSubModelRotation(mPlayerObject, 0, rot);
           updateCameraFromSubObject(mPlayerObject, 0);
        */
    }

    // reset the rotation indicators
    mHorizontalRot = 0;
    mVerticalRot = 0;
}

//------------------------------------------------------
bool CameraService::init() { return true; }

//------------------------------------------------------
void CameraService::bootstrapFinished() {
    mInputSrv = GET_SERVICE(InputService);
    mRenderSrv = GET_SERVICE(RenderService);
    mSimSrv = GET_SERVICE(SimService);
    mObjSrv = GET_SERVICE(ObjectService);
    mPlayerSrv = GET_SERVICE(PlayerService);
    mPropertySrv = GET_SERVICE(PropertyService);

    /* TODO: Input commands:
       - turnleftfast
       - turnrightfast
       - lookup
       - lookdown
       - lookcenter
       - joyxaxis
    */

    InputService::ListenerPtr mturnListener(
        new ClassCallback<InputEventMsg, CameraService>(
            this, &CameraService::onMTurn));
    InputService::ListenerPtr mlookListener(
        new ClassCallback<InputEventMsg, CameraService>(
            this, &CameraService::onMLook));

    mInputSrv->registerCommandTrap("mturn", mturnListener);
    mInputSrv->registerCommandTrap("mlook", mlookListener);

    // sim listener for the camera rotation
    mSimSrv->registerListener(this, SIM_PRIORITY_INPUT);

    mCamera = mRenderSrv->getDefaultCamera();
    // needed for propper orientation...
    mCamera->pitch(Degree(90));
    mCamera->setFixedYawAxis(true, Vector3::UNIT_Z);
    // TODO: Need to check this value...
    mCamera->setFOVy(Degree(60));
    // TODO: what the heck is the variable user_camera_offset and how does it
    // interfere with this code?

    mHasRefsProperty = mPropertySrv->getProperty("HasRefs");
}

//------------------------------------------------------
void CameraService::shutdown() {
    mInputSrv->unregisterCommandTrap("mturn");
    mInputSrv->unregisterCommandTrap("mlook");
    mSimSrv->unregisterListener(this);

    mInputSrv.reset();
    mRenderSrv.reset();
    mSimSrv.reset();
    mObjSrv.reset();
    mPropertySrv.reset();
}

//------------------------------------------------------
void CameraService::onMTurn(const InputEventMsg &iem) {
    // check for the event type
    if (iem.event != IET_MOUSE_MOVE)
        return;

    // calculate the rotation based on some variables
    // we are interested in:
    //  * mouse_sensitivity
    //  * mouse_invert
    const DVariant &msens = mInputSrv->getVariable("mouse_sensitivity");
    const DVariant &minvert = mInputSrv->getVariable("mouse_invert");

    // schedule a turn on the object in question
    float rot = iem.params.toFloat() * msens.toFloat();
    if (minvert.toBool())
        rot = -rot;

    appendCameraRotation(rot, 0.0f);
}

//------------------------------------------------------
void CameraService::onMLook(const InputEventMsg &iem) {
    // check for the event type
    if (iem.event != IET_MOUSE_MOVE)
        return;

    // is freelook enabled?
    DVariant mfreelook = mInputSrv->getVariable("freelook");

    if (mfreelook.toInt() == 0)
        return;

    // freelook enabled, rotate the view
    const DVariant &msens = mInputSrv->getVariable("mouse_sensitivity");
    const DVariant &minvert = mInputSrv->getVariable("mouse_invert");

    float rot = iem.params.toFloat() * msens.toFloat();
    if (minvert.toBool())
        rot = -rot;

    appendCameraRotation(0.0f, rot);
}

//------------------------------------------------------
void CameraService::appendCameraRotation(float horizontal, float vertical) {
    // will be applied upon sim service callback
    mHorizontalRot += horizontal;
    mVerticalRot += vertical;
}

//------------------------------------------------------
bool CameraService::updateCameraFromSubObject(int objId, size_t submdl) {
    if (!mPhysicsService->objHasPhysics(objId))
        return false;

    // get the position and orientation of the subobj
    size_t smc = mPhysicsService->getSubModelCount(submdl);

    if (smc <= submdl)
        return false;

    const Vector3 &pos = mPhysicsService->getSubModelPosition(objId, submdl);
    const Quaternion &rot =
        mPhysicsService->getSubModelOrientation(objId, submdl);

    mCamera->setPosition(pos);
    mCamera->setOrientation(rot);

    return true;
}

//------------------------------------------------------
void CameraService::handleAttachment(int objID, bool dynamic) {
    // re-enable hasrefs on the prev. object
    if (mAttachmentObject)
        mHasRefsProperty->set(mAttachmentObject, "", 1);

    mAttachmentObject = objID;
    mDynamicAttach = dynamic;

    if (mAttachmentObject)
        mHasRefsProperty->set(mAttachmentObject, "", 0);

    mPlayerSrv->handleCameraAttachment(mAttachmentObject);
}

//-------------------------- Factory implementation
std::string CameraServiceFactory::mName = "CameraService";

CameraServiceFactory::CameraServiceFactory() : ServiceFactory(){};

const std::string &CameraServiceFactory::getName() { return mName; }

const uint CameraServiceFactory::getMask() { return SERVICE_RENDERER; }

const size_t CameraServiceFactory::getSID() { return CameraService::SID; }

Service *CameraServiceFactory::createInstance(ServiceManager *manager) {
    return new CameraService(manager, mName);
}

} // namespace Opde
