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
 *	  $Id$
 *
 *****************************************************************************/

/**
 @file LoopService.h
 @brief A loop service. The loop service is a loop maintainer, has the abilities
 to set different modes, which contain different sets of clients
*/

#ifndef __LOOPSERVICE_H
#define __LOOPSERVICE_H

#include <vector>
#include <map>

#include "Callback.h"
#include "LoopCommon.h"
#include "OpdeService.h"
#include "OpdeServiceFactory.h"
#include "SharedPtr.h"
#include "compat.h"

namespace Ogre {
class Root;
} // namespace Ogre

namespace Opde {

class LoopService;

typedef std::multimap<LoopClientPriority, LoopClient *> LoopClientMap;

/** The loop mode definition. */
class LoopMode {
public:
    LoopMode(const LoopModeDefinition &def, LoopService *owner);

    /** Loop mode start event. Notifies all the loop clients that this loop mode
     * started.
     */
    void loopModeStarted();

    /** Loop mode ended event. Notifies all the loop client that the loop mode
     * ended.
     */
    void loopModeEnded();

    /** Loop step event. Notifies all the clients a step happened.
     */
    void loopStep(float deltaTime);

    /** Sets a flag to spit timing info for the next frame */
    void debugOneFrame() { mDebugNextFrame = true; };

    /// @returns the Loop mask of this mode
    LoopModeMask getLoopMask() const { return mLoopModeDef.mask; };

    const std::string &getLoopModeName() const { return mLoopModeDef.name; };

    /// Adds a loop client into this loop mode
    void addLoopClient(LoopClient *client) {
        mLoopClients.insert(std::make_pair(client->getPriority(), client));
    };

    /// Removes a loop client from this loop mode
    void removeLoopClient(LoopClient *client);

protected:
    LoopModeDefinition mLoopModeDef;
    LoopClientMap mLoopClients;
    LoopService *mOwner; // Used to get timing info if debug is on for the loop

    bool mDebugNextFrame;
};

typedef shared_ptr<LoopMode> LoopModePtr;

/** @brief Loop Service - service which handles game loop (per-frame loop) */
class LoopService : public ServiceImpl<LoopService> {
public:
    /** Constructor
     * @param manager The ServiceManager that created this service
     * @param name The name this service should have (For Debugging/Logging)
     */
    LoopService(ServiceManager *manager, const std::string &name);
    virtual ~LoopService();

    /** Creates a loop mode specified by the definition
     * @param modeDef The loop mode definition. The mode should have a unique ID
     * (Is checked)
     * @note If the mode has an loop ID of a loop mode which has already been
     * created, error is logged and loop is not created
     */
    void createLoopMode(const LoopModeDefinition &modeDef);

    /** Adds a loop client. The client is automatically added to all loops with
     * the complying masks (mode mask & client mask != 0)
     * @param client The loop client going to be inserted
     * @note The client has to have a unique ID. The loop mode definition of the
     * client is gathered with LoopClient::getLoopClientSpecification
     * @note shared_ptr is not used here because of some problems expected with
     * reference counts
     */
    void addLoopClient(LoopClient *client);

    /** Removes a loop client from all the loop modes.
     * @param client The loop client to be removed
     * @note shared_ptr is not used here because of some problems expected with
     * reference counts
     */
    void removeLoopClient(LoopClient *client);

    /** Requests a transition to a new loop mode
     * @param newLoopMode The new loop mode to be used. If the loop mode is
     * invalid, the loop service's run() method will terminate
     * @return true if the loop mode was found and request was queued, false
     * otherwise
     */
    bool requestLoopMode(LoopModeID newLoopMode);

    /** Requests a transition to a new loop mode
     * @param newLoopMode The new loop mode to be used. If the loop mode is
     * invalid, the loop service's run() method will terminate
     * @return true if the loop mode was found and request was queued, false
     * otherwise
     */
    bool requestLoopMode(const std::string &name);

    /// Requests the termination of the loop (program exit)
    void requestTermination();

    /// Returns true if the termination was requested and program should end
    bool isTerminationRequested() { return mTerminationRequested; }

    /// The main run code. Run by the main.cpp typically. Runs the application
    /// loops
    void run();

    /// Does a single pass through the loop mode
    void step();

    /// @return The current absolute time in ms
    unsigned long getCurrentTime();

    /// Logs DEBUG level info of one frame timings
    void debugOneFrame();

    /// returns time in miliseconds the last frame took
    unsigned long getLastFrameTime();

protected:
    /// Service intialization
    bool init();

    /** sets a new loop mode
     */
    void setLoopMode(const LoopModePtr &newMode);

    /// Termination was requested
    bool mTerminationRequested;

    /// The new requested loop mode (if the mNewModeRequested is true)
    LoopModePtr mNewLoopMode;

    /// True if something requested a change in the loop mode
    bool mNewModeRequested;

    /// The active loop mode
    LoopModePtr mActiveMode;

    typedef std::map<LoopModeID, LoopModePtr> LoopModeIDMap;
    typedef std::map<std::string, LoopModePtr> LoopModeNameMap;

    typedef std::vector<LoopClient *> LoopClientList;

    /// Loop mode map - per ID
    LoopModeIDMap mLoopModes;
    /// Loop mode map - per Name
    LoopModeNameMap mLoopNamedModes;

    /// List of registered loop clients
    LoopClientList mLoopClients;

    /// Ogre::Root for timing purposes
    Ogre::Root *mRoot;

    /// Time of the last frame (absolute)
    unsigned long mLastFrameTime;

    /// Length of the last frame (in miliseconds)
    unsigned long mLastFrameLength;

    bool mDebugOneFrame;
};

/// Factory for the LoopService objects
class LoopServiceFactory : public ServiceFactory {
public:
    LoopServiceFactory();
    ~LoopServiceFactory(){};

    /** Creates a LoopService instance */
    Service *createInstance(ServiceManager *manager);

    const std::string &getName() override;
    const uint getMask() override;
    const size_t getSID() override;

private:
    static const std::string mName;
};
} // namespace Opde

#endif
