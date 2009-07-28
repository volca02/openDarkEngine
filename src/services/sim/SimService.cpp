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


#include "SimService.h"
#include "OpdeException.h"
#include "ServiceCommon.h"

using namespace std;

namespace Opde {
	/*----------------------------------------------------*/
	/*-------------------- Sim Listener ------------------*/
	/*----------------------------------------------------*/
	SimListener::SimListener() : mSimTime(0), mSimTimeFlow(1), mSimPaused(false), mSimRunning(false) {

	}

	//------------------------------------------------------
	SimListener::~SimListener() {
	}

	//------------------------------------------------------
	void SimListener::simStarted() {
		mSimTime = 0;
		mSimRunning = true;
	}

	//------------------------------------------------------
	void SimListener::simEnded() {
		mSimRunning = false;
	}


	//------------------------------------------------------
	void SimListener::simPaused() {
		mSimPaused = true;
	}

	//------------------------------------------------------
	void SimListener::simUnPaused() {
		mSimPaused = false;
	}

	//------------------------------------------------------
	void SimListener::simFlowChange(float newFlow) {
		mSimTimeFlow = newFlow;
	}

	//------------------------------------------------------
	void SimListener::simStep(float simTime, float delta) {
		mSimTime = simTime;
	}

	/*----------------------------------------------------*/
	/*-------------------- Sim Service -------------------*/
	/*----------------------------------------------------*/
	template<> const size_t ServiceImpl<SimService>::SID = __SERVICE_ID_SIM;
		
	SimService::SimService(ServiceManager *manager, const std::string& name) : ServiceImpl< SimService >(manager, name),
			mTimeCoeff(1),
			mSimTime(0),
			mPaused(false) {
	}

   	//------------------------------------------------------
	bool SimService::init() {
	    return true;
	}

	//------------------------------------------------------
	SimService::~SimService() {
	}

	//------------------------------------------------------
	void SimService::loopStep(float deltaTime) {
		SimListeners::iterator it;

		if (mStartStopReq.getIfReq(mSimRunning)) {
			if (mSimRunning)
				mSimTime = 0;

			it = mSimListeners.begin();
			while (it != mSimListeners.end()) {
				if (mSimRunning)
					(it++)->second->simStarted();
				else
					(it++)->second->simEnded();
			}
		}

		// has to happen here, otherwise half of the sim could get different scaling
		if (mPauseChangeReq.getIfReq(mPaused)) {
			it = mSimListeners.begin();
			while (it != mSimListeners.end()) {
				if (mPaused)
					(it++)->second->simPaused();
				else
					(it++)->second->simUnPaused();
			}
		}

		if (mFlowChangeReq.getIfReq(mTimeCoeff)) {
			it = mSimListeners.begin();
			while (it != mSimListeners.end()) {
				(it++)->second->simFlowChange(mTimeCoeff);
			}
		}

		if (!mPaused && mSimRunning) {
			float delta = mTimeCoeff * deltaTime;

			it = mSimListeners.begin();
			while (it != mSimListeners.end()) {
				(it++)->second->simStep(mSimTime, delta);
			}

			mSimTime += delta;
		}
	}

	//------------------------------------------------------
	void SimService::registerListener(SimListener* listener, size_t priority) {
		mSimListeners.insert(std::make_pair(priority, listener));

		// to be sure
		if (mSimRunning)
			listener->simStarted();
		else
			listener->simEnded();

		if (mPaused)
			listener->simPaused();
		else
			listener->simUnPaused();

		listener->simFlowChange(mTimeCoeff);
	}

	//------------------------------------------------------
	void SimService::unregisterListener(SimListener* listener) {
		SimListeners::iterator it;

		while (it != mSimListeners.end()) {
		    if (it->second == listener) {
		    	SimListeners::iterator rem = it++;
		    	mSimListeners.erase(rem);
			} else {
				it++;
	        }
		}
	}

	//------------------------------------------------------
	void SimService::pauseSim() {
		if (mPaused || !mSimRunning)
			return;

		mPauseChangeReq.set(true);
	}

	//------------------------------------------------------
	void SimService::unPauseSim() {
		if (!mPaused || !mSimRunning)
			return;

		mPauseChangeReq.set(false);
	}

	//------------------------------------------------------
	void SimService::startSim() {
		if (mSimRunning)
			return;

		mStartStopReq.set(true);
	}

	//------------------------------------------------------
	void SimService::endSim() {
		if (!mSimRunning)
			return;

		mStartStopReq.set(false);
	}

	//------------------------------------------------------
	void SimService::setFlowCoeff(float flowCoeff) {
		if (flowCoeff > 0)
			mFlowChangeReq.set(flowCoeff);
	}


	//-------------------------- Factory implementation
	std::string SimServiceFactory::mName = "SimService";

	SimServiceFactory::SimServiceFactory() : ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
	};

	const std::string& SimServiceFactory::getName() {
		return mName;
	}

	const uint SimServiceFactory::getMask() {
		return SERVICE_ENGINE;
	}
	
	const size_t SimServiceFactory::getSID() {
		return SimService::SID;
	}

	Service* SimServiceFactory::createInstance(ServiceManager* manager) {
		return new SimService(manager, mName);
	}

}
