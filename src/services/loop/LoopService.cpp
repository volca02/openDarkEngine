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
 *****************************************************************************/

#include "ServiceCommon.h"
#include "LoopService.h"
#include "OpdeException.h"
#include "logger.h"

using namespace std;

namespace Opde {
	/*-------------------------------------------------*/
    /*-------------------- LoopMode -------------------*/
    /*-------------------------------------------------*/
    
    LoopMode::LoopMode(const LoopModeDefinition& def, LoopService* owner) : mLoopModeDef(def), mOwner(owner) {
    }
    
    //------------------------------------------------------
    void LoopMode::loopModeStarted() {
    	LoopClientSet::iterator it = mLoopClients.begin();
    	
    	while (it != mLoopClients.end()) {
    		(*(it++))->loopModeStarted(mLoopModeDef);
    	}
    }

	//------------------------------------------------------
	void LoopMode::loopModeEnded() {
    	LoopClientSet::iterator it = mLoopClients.begin();
    	
    	while (it != mLoopClients.end()) {
    		(*(it++))->loopModeEnded(mLoopModeDef);
    	}

	}
			
	//------------------------------------------------------
	void LoopMode::loopStep(float deltaTime) {
    	LoopClientSet::iterator it = mLoopClients.begin();
    	
    	bool debugFrame = mDebugNextFrame;
    	mDebugNextFrame = false;
    	
    	int corder = 0;
    	
    	while (it != mLoopClients.end()) {
    		unsigned long stTime = mOwner->getCurrentTime();
    		
    		LoopClient* client = *(it++);
    		
    		client->loopStep(deltaTime);
    		
    		unsigned long nowTime = mOwner->getCurrentTime();
    		
    		if (debugFrame) {
    			LOG_DEBUG("LoopMode(%s) : Client %d (%s) : Used %ld ms", getLoopModeName().c_str(), corder, client->getLoopClientSpecification().name.c_str(), nowTime - stTime);
    		}
    		
    		corder++;
    	}

	}
    
    /*----------------------------------------------------*/
    /*-------------------- LoopService -------------------*/
    /*----------------------------------------------------*/
    LoopService::LoopService(ServiceManager *manager, const std::string& name) : Service(manager, name), 
			mTerminationRequested(false), mNewLoopModeID(0), mLastFrameTime(0), mNewModeRequested(false) {
				
		mActiveMode.setNull();
    }

    //------------------------------------------------------
    LoopService::~LoopService() {
    }

	//------------------------------------------------------
	bool LoopService::init() {
		// We use the Ogre::Root for timing, so it has to be defined.
		// I could use render service for that, but it could create a circular reference 
		// (not likely, loop clients are weak pointers in fact, but anyway)
		mRoot = Ogre::Root::getSingletonPtr();
		return true;
	}
	
	//------------------------------------------------------
	void LoopService::createLoopMode(const LoopModeDefinition& modeDef) {
		LOG_DEBUG("LoopService::createLoopMode: Creating new loop mode '%s'", modeDef.name.c_str());
		
		// First, look if the ID is not already reserved
		LoopModeMap::const_iterator it = mLoopModes.find(modeDef.id);
		
		if (it != mLoopModes.end()) {
			LOG_ERROR("LoopService::createLoopMode: Loop mode ID %llX already reserved by '%s'", it->second->getLoopModeName().c_str());
			return;
		}
		
		LoopModePtr mode = new LoopMode(modeDef, this);
		
		mLoopModes.insert(make_pair(modeDef.id, mode));
		
		// Loop mode created...
	}
	
	//------------------------------------------------------
	void LoopService::addLoopClient(LoopClient* client) {
		// iterate through the loop modes. If the mask complies, add the client
		LoopModeMap::const_iterator it = mLoopModes.begin();
		
		while( it != mLoopModes.end())  {
			if (it->second->getLoopMask() & client->getLoopMask()) {
				it->second->addLoopClient(client);
			}
			
			it++;
		}
	}
	
	//------------------------------------------------------
	void LoopService::removeLoopClient(LoopClient* client) {
		// iterate through the loop modes. If the mask complies, add the client
		LoopModeMap::const_iterator it = mLoopModes.begin();
		
		while( it != mLoopModes.end())  {
			// The worst thing that could happen is that there was no client in that loop
			// So no removal was done. 
			it->second->removeLoopClient(client);
			
			it++;
		}
	}
	
	//------------------------------------------------------
	void LoopService::requestLoopMode(LoopModeID newLoopMode) {
		mNewLoopModeID = newLoopMode;
		mNewModeRequested = true;
	}
	
	
	//------------------------------------------------------
	void LoopService::requestTermination() {
		LOG_DEBUG("LoopService: Termination was requested!");
		mTerminationRequested = true;
	}
	

	//------------------------------------------------------
	void LoopService::run() {
		float deltaTime = 0.0;
		
		if (mActiveMode.isNull()) {
			LOG_FATAL("LoopService::run: No loop mode set prior to run(). Terminating");
			return;
		}
		
		while (!mTerminationRequested) {
			bool debugFrame = mDebugOneFrame;
			mDebugOneFrame = false;
			
			if (debugFrame) {
				LOG_DEBUG("---- LoopService one frame timings ----");
				mActiveMode->debugOneFrame();
			}
			
			unsigned long lFrameStart = getCurrentTime();
			unsigned long deltaTime = lFrameStart - mLastFrameTime;
			
			
			mActiveMode->loopStep(deltaTime);
			
			if (mNewModeRequested) {
				mNewModeRequested = false;
				mActiveMode->loopModeEnded();
				
				
				// Search for the new loop mode. If not found, log fatal, terminate
				if (!setLoopMode(mNewLoopModeID)) {
					LOG_FATAL("LoopService::run: The new requested loop mode (%llX) is invalid. Terminating", mNewLoopModeID);
					mTerminationRequested = true;
				} else {
					mActiveMode->loopModeStarted();
				}
			}
			
			if (debugFrame) {
				LOG_DEBUG("---- One frame timings end. Total frame time: %d ms ----", getCurrentTime() - mLastFrameTime);
			}
			mLastFrameTime = lFrameStart;
		}
		
		if (!mActiveMode.isNull()) {
			mActiveMode->loopModeEnded();
		}
		
		// The end of the loop.
	}

	//------------------------------------------------------
	bool LoopService::setLoopMode(LoopModeID newModeID) {
		LoopModeMap::const_iterator it = mLoopModes.find(newModeID);
		
		if (it != mLoopModes.end()) {
			mActiveMode = it->second;
			return true;
		} else {
			return false;
		}
	}
	
	//------------------------------------------------------
	void LoopService::debugOneFrame() {
		mDebugOneFrame = true;
	}

    //-------------------------- Factory implementation
    std::string LoopServiceFactory::mName = "LoopService";

    LoopServiceFactory::LoopServiceFactory() : ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
    };

    const std::string& LoopServiceFactory::getName() {
		return mName;
    }

    Service* LoopServiceFactory::createInstance(ServiceManager* manager) {
	return new LoopService(manager, mName);
    }

}
