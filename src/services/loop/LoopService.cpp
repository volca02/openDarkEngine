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
    
    LoopMode::LoopMode(const LoopModeDefinition& def, LoopService* owner) : mLoopModeDef(def), mOwner(owner), mDebugNextFrame(false) {
    }
    
    //------------------------------------------------------
    void LoopMode::loopModeStarted() {
    	LoopClientMap::iterator it = mLoopClients.begin();
    	
    	while (it != mLoopClients.end()) {
    		it->second->loopModeStarted(mLoopModeDef);

		++it;
    	}
    }

	//------------------------------------------------------
	void LoopMode::loopModeEnded() {
    	LoopClientMap::iterator it = mLoopClients.begin();
    	
    	while (it != mLoopClients.end()) {
    		it->second->loopModeEnded(mLoopModeDef);

		++it;
    	}

	}
			
    //------------------------------------------------------
    void LoopMode::loopStep(float deltaTime) {
    	LoopClientMap::iterator it = mLoopClients.begin();
    	
    	bool debugFrame = mDebugNextFrame;
    	mDebugNextFrame = false;
    	
    	int corder = 0;
    	
    	while (it != mLoopClients.end()) {
    		unsigned long stTime = mOwner->getCurrentTime();
    		
    		LoopClient* client = it->second;
		
			it++;
    		
    		client->loopStep(deltaTime);
    		
    		unsigned long nowTime = mOwner->getCurrentTime();
    		
    		if (debugFrame) {
    			LOG_DEBUG("LoopMode(%s) : Client %d (%s) : Used %ld ms", getLoopModeName().c_str(), corder, client->getLoopClientSpecification().name.c_str(), nowTime - stTime);
    		}
    		
    		corder++;
    	}

    }
    
    //------------------------------------------------------
    void LoopMode::removeLoopClient(LoopClient* client) {
	LoopClientMap::iterator it = mLoopClients.find(client->getPriority());
	
	while (it != mLoopClients.end()) {
	    if (it->second == client) {
		LoopClientMap::iterator rem = it++;
		
		mLoopClients.erase(rem);
	    } else {
		it++;
	    }
	}
    }

    /*---------------------------------------------------*/
    /*-------------------- LoopClient -------------------*/
    /*---------------------------------------------------*/
    LoopClient::~LoopClient() {
    }
    
	void LoopClient::loopModeStarted(const LoopModeDefinition& loopMode) {
	}
	
	void LoopClient::loopModeEnded(const LoopModeDefinition& loopMode) {
	}
    
    /*----------------------------------------------------*/
    /*-------------------- LoopService -------------------*/
    /*----------------------------------------------------*/
    LoopService::LoopService(ServiceManager *manager, const std::string& name) : Service(manager, name), 
			mTerminationRequested(false), mNewLoopMode(NULL), mLastFrameTime(0), mNewModeRequested(false),
			mDebugOneFrame(false) {
				
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
		LoopModeIDMap::const_iterator it = mLoopModes.find(modeDef.id);
		
		if (it != mLoopModes.end()) {
			LOG_ERROR("LoopService::createLoopMode: Loop mode ID %llX already reserved by '%s'", modeDef.id, it->second->getLoopModeName().c_str());
			return;
		}
		
		LoopModeNameMap::const_iterator it1 = mLoopNamedModes.find(modeDef.name);
		
		if (it1 != mLoopNamedModes.end()) {
			LOG_ERROR("LoopService::createLoopMode: Loop mode name %s already reserved by '%s'", modeDef.name.c_str(), it->second->getLoopModeName().c_str());
			return;
		}
		
		LoopModePtr mode = new LoopMode(modeDef, this);
		
		mLoopModes[modeDef.id] = mode;
		mLoopNamedModes[modeDef.name] = mode;
		
		// Loop mode created. Loop over already inserted clients and register them as well.
		LoopClientList::const_iterator cit = mLoopClients.begin();
		for (; cit != mLoopClients.end(); ++cit) {
		    mode->addLoopClient(*cit);
		}
	}
	
	//------------------------------------------------------
	void LoopService::addLoopClient(LoopClient* client) {
		// iterate through the loop modes. If the mask complies, add the client
		LoopModeIDMap::const_iterator it = mLoopModes.begin();
		
		while( it != mLoopModes.end())  {
			if (it->second->getLoopMask() & client->getLoopMask()) {
				it->second->addLoopClient(client);
			}
			
			it++;
		}
		
		mLoopClients.push_back(client);
	}
	
	//------------------------------------------------------
	void LoopService::removeLoopClient(LoopClient* client) {
		// iterate through the loop modes. If the mask complies, add the client
		LoopModeIDMap::const_iterator it = mLoopModes.begin();
		
		while( it != mLoopModes.end())  {
			// The worst thing that could happen is that there was no client in that loop
			// So no removal was done. 
			it->second->removeLoopClient(client);
			
			it++;
		}
	}

	
	//------------------------------------------------------
	bool LoopService::requestLoopMode(LoopModeID newLoopMode) {
		LoopModeIDMap::const_iterator it = mLoopModes.find(newLoopMode);
		
		if (it != mLoopModes.end()) {
			mNewLoopMode = it->second;
			mNewModeRequested = true;
			return true;
		} else {
			return false;
		}
	}

	//------------------------------------------------------
	bool LoopService::requestLoopMode(const std::string& name) {
		LoopModeNameMap::const_iterator it = mLoopNamedModes.find(name);
		
		if (it != mLoopNamedModes.end()) {
			mNewLoopMode = it->second;
			mNewModeRequested = true;
			return true;
		} else {
			return false;
		}
	}

	//------------------------------------------------------
	void LoopService::requestTermination() {
		LOG_DEBUG("LoopService: Termination was requested!");
		mTerminationRequested = true;
	}
	

	//------------------------------------------------------
	void LoopService::run() {
		float deltaTime = 0.0;
		
		if (mNewModeRequested) { // See if there is a loop mode pending...
			mNewModeRequested = false;
			setLoopMode(mNewLoopMode);
			mActiveMode->loopModeStarted();
		}
		
		if (mActiveMode.isNull()) {
			LOG_FATAL("LoopService::run: No loop mode set prior to run(). Terminating");
			return;
		}
		
		while (!mTerminationRequested) {
			bool debugFrame = mDebugOneFrame;
			mDebugOneFrame = false;
			
			if (debugFrame) {
				LOG_FATAL("---- LoopService one frame timings ----");
				mActiveMode->debugOneFrame();
			}
			
			unsigned long lFrameStart = getCurrentTime();
			unsigned long deltaTime = lFrameStart - mLastFrameTime;
			
			
			mActiveMode->loopStep(deltaTime);
			
			if (mNewModeRequested) {
				mNewModeRequested = false;
				mActiveMode->loopModeEnded();
				
				// Search for the new loop mode. If not found, log fatal, terminate
				if (!mNewLoopMode.isNull()) {
					setLoopMode(mNewLoopMode);
				} else {
					LOG_FATAL("LoopService::run: The new requested loop mode is invalid (NULL mode pointer encountered). Terminating");
					mTerminationRequested = true;
				}
			
				// Signalize the start of the loop
				mActiveMode->loopModeStarted();
			}
			
			if (debugFrame) {
				LOG_FATAL("---- One frame timings end. Total frame time: %d ms ----", getCurrentTime() - mLastFrameTime);
			}
			mLastFrameTime = lFrameStart;
		}
		
		if (!mActiveMode.isNull()) {
			mActiveMode->loopModeEnded();
		}
		
		// The end of the loop.
	}

	//------------------------------------------------------
	void LoopService::setLoopMode(const LoopModePtr& newMode) {
		mActiveMode = mNewLoopMode;
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

	const uint LoopServiceFactory::getMask() {
		return SERVICE_ENGINE;
	}
	
    Service* LoopServiceFactory::createInstance(ServiceManager* manager) {
	return new LoopService(manager, mName);
    }

}
