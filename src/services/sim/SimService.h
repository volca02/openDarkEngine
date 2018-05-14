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

#ifndef __SIMSERVICE_H
#define __SIMSERVICE_H

#include <map>

#include "OpdeService.h"
#include "OpdeServiceFactory.h"
#include "ServiceCommon.h"
#include "SharedPtr.h"
#include "SimCommon.h"
#include "ValueChangeRequest.h"
#include "loop/LoopCommon.h"

namespace Opde {

/** @brief Sim Service - a simulation timer base. Implements Sim time
 * (simulation time). This time can have different flow rate than normal time.
 *  All simulation related listeners register here, not via loop service, which
 * only handles game loop (and does not have the capability to pause/stretch
 * time).
 */
class SimService : public ServiceImpl<SimService>, LoopClient {
public:
    SimService(ServiceManager *manager, const std::string &name);
    virtual ~SimService();

    // --- Registration/Unregistration of clients

    /** registers a loop listener
     * @param listener The listener instance pointer to register
     * @param priority The loop priority (order)
     */
    void registerListener(SimListener *listener, size_t priority);

    /// unregisters a loop listener
    void unregisterListener(SimListener *listener);

    // --- Loop Service Client related ---

    /// called from loop service
    void loopStep(float deltaTime);

    /** Pauses sim. Only possible if sim time is running and paused. Informs all
     * sim listeners.
     * @note This method is called automatically if loop listener this class
     * implements is informed the next loop mode won't include this listener
     */
    void pauseSim();

    /** un-pauses sim. Only possible if sim-time is running and paused. Informs
     * all sim listeners.
     * @note This method is called automatically if loop listener this class
     * implments is informed the next loop mode will include this listener.
     */
    void unPauseSim();

    /// Getter for sim time state (paused or running). Value for stopped sim
    /// time is unspecified.
    inline bool isSimPaused() { return mPaused; };

    /** Starts sim-time. Will reset the sim-time value to 0. Informs all sim
     * listeners. Does not touch the sim time flow coeff.
     */
    void startSim();

    /** Ends the sim-time. Possibly destructive to the sim related data. Informs
     * all sim listeners.
     */
    void endSim();

    /// Getter for sim-time running state (can be true although pause is in
    /// progress).
    inline bool isSimRunning() { return mSimRunning; };

    /// Detects if sim-time is effectively running - not stopped and not paused
    inline bool isSimEffectivelyRunning() { return mSimRunning && (!mPaused); };

    /// Sets the sim time flow ration (speed of flow of the sim in comparison
    /// with the real time)
    void setFlowCoeff(float flowCoeff);

    inline float getFlowCoeff() { return mTimeCoeff; };

protected:
    /// Service initialization
    bool init();

    /// Time flow coefficient - 1.0 means the sim time is 1:1 with real time.
    /// 0.5 means sim time flows 2 times slower than normal time
    float mTimeCoeff;

    /// Request for new flow value
    ValueChangeRequest<float> mFlowChangeReq;

    /// Request for Pause/UnPause
    ValueChangeRequest<bool> mPauseChangeReq;

    // Request for Start/Stop
    ValueChangeRequest<bool> mStartStopReq;

    /// Sim time. Set to zero on simulation start
    float mSimTime;

    /// Sim-Time pause in progress
    bool mPaused;

    /// Sim-Time is running (only true between startSim and endSim calls)
    bool mSimRunning;

    /// Simulation listeners - multimap, as we need prioritised behavior
    typedef std::multimap<size_t, SimListener *> SimListeners;

    SimListeners mSimListeners;
};

/// Shared pointer to Sim service
typedef shared_ptr<SimService> SimServicePtr;

/// Factory for the SimService objects
class SimServiceFactory : public ServiceFactory {
public:
    SimServiceFactory();
    ~SimServiceFactory(){};

    /** Creates a SimService instance */
    Service *createInstance(ServiceManager *manager) override;

    const std::string &getName() override;
    const uint getMask() override;
    const size_t getSID() override;

private:
    static const std::string mName;
};
} // namespace Opde

#endif
