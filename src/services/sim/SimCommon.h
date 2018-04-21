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

#ifndef __SIMCOMMON_H
#define __SIMCOMMON_H

namespace Opde {

/** Abstract Simulation listener - a class that does something related to
 * simulation extends this
 */
class SimListener {
public:
    SimListener();

    virtual ~SimListener();

    /** Called when the simulation is started by sim service. Sets sim time to
     * zero. sets mSimRunning to true. */
    virtual void simStarted();

    /** Called when the simulation is ended by sim service. Sets mSimRunning to
     * false. */
    virtual void simEnded();

    /** Called when the simulation is paused. Sets mPaused */
    virtual void simPaused();

    /** Called when the simulation is un-paused. Unsets mPaused */
    virtual void simUnPaused();

    /** Called every time time flow change happens. */
    virtual void simFlowChange(float newFlow);

    /** simulation time step happened
     * @param simTime the new sim time
     * @param delta the time increment that happened
     */
    virtual void simStep(float simTime, float delta);

protected:
    float mSimTime;
    float mSimTimeFlow;
    bool mSimPaused;
    bool mSimRunning;
};

} // namespace Opde

#endif /* __SIMCOMMON_H */
