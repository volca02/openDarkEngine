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
 *      $Id$
 *
 *****************************************************************************/

#ifndef __CAMERASERVICE_H
#define __CAMERASERVICE_H

#include "config.h"

#include "OpdeService.h"
#include "ServiceCommon.h"
#include "sim/SimService.h"

namespace Ogre {
class Camera;
} // namespace Ogre

namespace Opde {

struct InputEventMsg;
class Property;

/** @brief camera service. Service that handles in-game camera
 */
class OPDELIB_EXPORT CameraService : public ServiceImpl<CameraService>,
                                     public SimListener {
public:
    CameraService(ServiceManager *manager, const std::string &name);
    virtual ~CameraService();

    /** Attaches camera to an object. Does not allow freelook
     * @return true on success, false on failure */
    bool staticAttach(int objID);

    /** Attaches camera to an object with freelook allowed
     * @return true on success, false on failure */
    bool dynamicAttach(int objID);

    /** Returns (conditionally) the camera to the player object.
     * Camera is only returned if it is currently attached to the object
     * specified by the parameter
     * @param curObjID The current object to which camera is attached
     * @return true on success, false on failure */
    bool cameraReturn(int objID);

    /** Returns (unconditionally) the camera to the player object. */
    void forceCameraReturn();

    // sim service listener:
    virtual void simPaused();
    virtual void simUnPaused();
    virtual void simStep(float simTime, float delta);

protected:
    bool init();
    void bootstrapFinished();
    void shutdown();

    // input listeners

    /// Input callback. Called upon mouse left/right (X axis) movement
    void onMTurn(const InputEventMsg &iem);

    /// Input callback. Called upon mouse up/down (Y axis) movement
    void onMLook(const InputEventMsg &iem);

    // schedules a camera rotation. The given values are not scaled by time
    void appendCameraRotation(float horizontal, float vertical);

    /** Updates the camera position and orientation based on the submodel
     * specified
     * @param objId the object id
     * @param submdl the sub-model number
     * @return true if the update was done, false if the parameters are invalid
     */
    bool updateCameraFromSubObject(int objId, size_t submdl);

    /** Handles the object attachment details - effects, hasrefs property
     * handling, player service handling.
     * @param objID the object to attach to
     * @param dynamic Controls the dynamicity aspect of the attachment. If true,
     * the camera can control the object's rotation */
    void handleAttachment(int objID, bool dynamic);

private:
    /// Input service ptr
    InputServicePtr mInputSrv;

    /// Render service ptr
    RenderServicePtr mRenderSrv;

    /// Simulation service ptr
    SimServicePtr mSimSrv;

    /// Object service ptr
    ObjectServicePtr mObjSrv;

    float mHorizontalRot;
    float mVerticalRot;

    /// paused flag (if paused, all rotation is discarded)
    bool mPaused;

    /// object we're attached to, or zero if none
    int mAttachmentObject;

    /// true means we allow the camera to rotate the object (no meaning if no
    /// attachment is done)
    bool mDynamicAttach;

    /// Physics service for camera updates
    PhysicsServicePtr mPhysicsService;

    /// Player service that holds the player object ID
    PlayerServicePtr mPlayerSrv;

    /// Property service handle (For hasrefs, etc)
    PropertyServicePtr mPropertySrv;

    /// Hasrefs property that is used to hide the object camera is currently
    /// attached to
    Property *mHasRefsProperty;

    /// Camera that we use
    Ogre::Camera *mCamera;
};

/// Shared pointer to Camera service
typedef shared_ptr<CameraService> CameraServicePtr;

/// Factory for the CameraService objects
class OPDELIB_EXPORT CameraServiceFactory : public ServiceFactory {
public:
    CameraServiceFactory();
    ~CameraServiceFactory(){};

    /** Creates a CameraService instance */
    Service *createInstance(ServiceManager *manager);

    virtual const std::string &getName();

    virtual const uint getMask();

    virtual const size_t getSID();

private:
    static std::string mName;
};
} // namespace Opde

#endif
