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


#include "SimService.h"
#include "OpdeException.h"
#include "ServiceCommon.h"

using namespace std;

namespace Opde {
	/*----------------------------------------------------*/
	/*-------------------- Sim Listener ------------------*/
	/*----------------------------------------------------*/
	SimListener::SimListener() : mSimTime(0), mPaused(false), mSimRunning(false) {

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
		mPaused = true;
	}

	//------------------------------------------------------
	void SimListener::simUnPaused() {
		mPaused = false;
	}

	//------------------------------------------------------
	void SimListener::simStep(float simTime, float delta) {
		mSimTime = simTime;
	}

	/*----------------------------------------------------*/
	/*-------------------- Sim Service -------------------*/
	/*----------------------------------------------------*/
	SimService::SimService(ServiceManager *manager, const std::string& name) : Service(manager, name), mTimeCoeff(1), mSimTime(0), mPaused(false) {
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
		if (!mPaused && mSimRunning) {
			float delta = mTimeCoeff * deltaTime;

			SimListeners::iterator it;

			while (it != mSimListeners.end()) {
				(it++)->second->simStep(mSimTime, delta);
			}

			mSimTime += delta;
		}
	}

	//------------------------------------------------------
	void SimService::registerListener(SimListener* listener) {
		mSimListeners.insert(std::make_pair(listener->getPriority(), listener));
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

		SimListeners::iterator it;

		while (it != mSimListeners.end()) {
			(it++)->second->simPaused();
		}

		mPaused = true;
	}

	//------------------------------------------------------
	void SimService::unPauseSim() {
		if (!mPaused || !mSimRunning)
			return;

		SimListeners::iterator it;

		while (it != mSimListeners.end()) {
			(it++)->second->simUnPaused();
		}

		mPaused = false;
	}

	//------------------------------------------------------
	void SimService::startSim() {
		if (mSimRunning)
			return;

		mSimTime = 0;

		SimListeners::iterator it;

		while (it != mSimListeners.end()) {
			(it++)->second->simStarted();
		}
	}

	//------------------------------------------------------
	void SimService::endSim() {
		if (!mSimRunning)
			return;

		SimListeners::iterator it;

		while (it != mSimListeners.end()) {
			(it++)->second->simEnded();
		}
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

	Service* SimServiceFactory::createInstance(ServiceManager* manager) {
		return new SimService(manager, mName);
	}

}
