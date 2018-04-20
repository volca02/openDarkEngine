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
 *****************************************************************************/

#ifndef __LOOPCOMMON_H
#define __LOOPCOMMON_H

#include <string>

#include "integers.h"

namespace Opde {

// Forward decl.
class LoopService;
class LoopMode;

/// @typedef Loop client id specifier
typedef uint64_t LoopClientID;

/// The ID of the loop client that is invalid (The loop client did not fill the
/// definition struct)
#define LOOPCLIENT_ID_INVALID 0

/// @typedef Loop mode id specifier
typedef uint64_t LoopModeID;

/// @typedef The loop mode mask
typedef uint64_t LoopModeMask;

/// @typedef The priority of the client (inverse)
typedef int64_t LoopClientPriority;

/// Mask that permits all the clients to run
/// ULL at the end of the value means unsigned long long
#define LOOPMODE_MASK_ALL_CLIENTS 0xFFFFFFFFFFFFFFFFULL

/// The definition of a loop client
struct LoopClientDefinition {
    /// Unique ID of the loop client
    LoopClientID id;
    /// Loop mode mask (which loop modes this loop client is member of)
    LoopModeMask mask;
    /// A descriptive name of the loop client name (for debugging)
    std::string name;
    /// The clients priority (not necesarilly unique) - lower number -> client
    /// called sooner in the loop
    LoopClientPriority priority;
};

/// Loop mode definition struct
struct LoopModeDefinition {
    /// unique id of this loop mode
    LoopModeID id;
    /// The mask of the loop mode (Should be 2^n for normal loop modes, but is
    /// not checked)
    LoopModeMask mask;
    /// A descriptive name of the loop mode name (for debugging)
    std::string name;
};

/// Loop Client. Abstract class. Receives the loop messages
class LoopClient {
protected:
    friend class LoopService;
    friend class LoopMode;

    /// default destructor
    virtual ~LoopClient();

    /** Loop mode started event. Called on the start of the frame, if a loop
     * mode changed and this client is a listener of the new loop mode
     * @param loopMode The new active loop mode
     */
    virtual void loopModeStarted(const LoopModeDefinition &loopMode);

    /** Loop mode ended event. Called on the end of the frame, if a loop mode
     * changed and this client is a listener of the old loop mode
     * @param loopMode The old loop mode
     */
    virtual void loopModeEnded(const LoopModeDefinition &loopMode);

    /** Loop step event. Called every frame by the active loop mode, if this
     * mode is the member of the new mode
     * @param deltaTime The time (in miliseconds) that passed since the start of
     * the last frame to the start of this frame
     */
    virtual void loopStep(float deltaTime) = 0;

    /// @returns the Loop mask of this client
    LoopModeMask getLoopMask() { return mLoopClientDef.mask; };

    /// @returns the clients priority
    LoopClientPriority getPriority() { return mLoopClientDef.priority; };

    /** The loop client definition getter.
     * @return The loop client definition struct reference.
     */
    virtual const LoopClientDefinition &getLoopClientSpecification() const {
        return mLoopClientDef;
    };

    /// Loop client mode definition
    LoopClientDefinition mLoopClientDef;
};

} // namespace Opde


#endif /* __LOOPCOMMON_H */
