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

/**
 @file LoopService.h
 @brief A loop service. The loop service is a loop maintainer, has the abilities to set different modes, which contain different sets of clients
*/

#ifndef __LOOPSERVICE_H
#define __LOOPSERVICE_H

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "SharedPtr.h"
#include "Callback.h"
#include "ConfigService.h"
#include "compat.h"
#include "config.h"

#include <OgreRoot.h>
#include <OgreTimer.h>

namespace Opde {

	// Forward decl.
	class LoopService;
	class LoopMode;

	/// @typedef Loop client id specifier
	typedef uint64_t LoopClientID;
	
	/// The ID of the loop client that is invalid (The loop client did not fill the definition struct)
	#define LOOPCLIENT_ID_INVALID 0
	
	/// @typedef Loop mode id specifier
	typedef uint64_t LoopModeID;
	
	/// @typedef The loop mode mask
	typedef uint64_t LoopModeMask;
	
	/// @typedef The priority of the client (inverse)
	typedef int64_t LoopClientPriority;
	
	/// Mask that permits all the clients to run
	#define LOOPMODE_MASK_ALL_CLIENTS 0xFFFFFFFFFFFFFFFF
	
	/// The definition of a loop client
	typedef struct {
		/// Unique ID of the loop client
		LoopClientID id;
		/// Loop mode mask (which loop modes this loop client is member of)
		LoopModeMask mask;
		/// A descriptive name of the loop client name (for debugging)
		std::string name;
		/// The clients priority (not necesarilly unique) - lower number -> client called sooner in the loop
		LoopClientPriority priority;
	} LoopClientDefinition;

	/// Loop mode definition struct
	typedef struct {
		/// unique id of this loop mode
		LoopModeID id;
		/// The mask of the loop mode (Should be 2^n for normal loop modes, but is not checked)
		LoopModeMask mask;
		/// A descriptive name of the loop mode name (for debugging)
		std::string name;
	} LoopModeDefinition;

	
	/// Loop Client. Abstract class. Receives the loop messages
	class LoopClient {
		protected:
			friend class LoopService;
			friend class LoopMode;
			
			/** Loop mode started event. Called on the start of the frame, if a loop mode changed and 
			* this client is a listener of the new loop mode
			* @param loopMode The new active loop mode
			*/
			virtual void loopModeStarted(const LoopModeDefinition& loopMode);
			
			/** Loop mode ended event. Called on the end of the frame, if a loop mode changed and 
			* this client is a listener of the old loop mode
			* @param loopMode The old loop mode
			*/
			virtual void loopModeEnded(const LoopModeDefinition& loopMode);
			
			/** Loop step event. Called every frame by the active loop mode, if this mode is the member of the new mode
			* @param deltaTime The time that passed since the start of the last frame to the start of this frame
			*/
			virtual void loopStep(float deltaTime) = 0;
			
			/// @returns the Loop mask of this client
			LoopModeMask getLoopMask() { return mLoopClientDef.mask; };
			
			/// @returns the clients priority
			LoopClientPriority getPriority() { return mLoopClientDef.priority; };
			
			/** The loop client definition getter.
			* @return The loop client definition struct reference.
			*/
			virtual const LoopClientDefinition& getLoopClientSpecification() const { return mLoopClientDef; };
			
			/// Loop client mode definition
			LoopClientDefinition mLoopClientDef;
	};

	typedef std::multimap< LoopClientPriority, LoopClient* > LoopClientMap;

	/** The loop mode definition. */
	class LoopMode {
		public:
			LoopMode(const LoopModeDefinition& def, LoopService* owner);
		
			/** Loop mode start event. Notifies all the loop clients that this loop mode started.
			*/
			void loopModeStarted();
			
			/** Loop mode ended event. Notifies all the loop client that the loop mode ended.
			*/
			void loopModeEnded();
			
			/** Loop step event. Notifies all the clients a step happened.
			*/
			void loopStep(float deltaTime);
			
			/** Sets a flag to spit timing info for the next frame */
			void debugOneFrame() { mDebugNextFrame = true; };
			
			/// @returns the Loop mask of this mode
			LoopModeMask getLoopMask()  const { return mLoopModeDef.mask; };
			
			const std::string& getLoopModeName() const { return mLoopModeDef.name; };
			
			/// Adds a loop client into this loop mode
			void addLoopClient(LoopClient* client) { mLoopClients.insert( std::make_pair(client->getPriority(), client)); };
			
			/// Removes a loop client from this loop mode
			void removeLoopClient(LoopClient* client);

		protected:
			LoopModeDefinition mLoopModeDef;
			LoopClientMap mLoopClients;
			LoopService* mOwner; // Used to get timing info if debug is on for the loop
			
			bool mDebugNextFrame;
	};
	
	typedef shared_ptr< LoopMode > LoopModePtr;
	
	/** @brief Loop Service - service which handles game loop (per-frame loop) */
	class LoopService : public Service {
		public:
			/** Constructor
			* @param manager The ServiceManager that created this service
			* @param name The name this service should have (For Debugging/Logging)
			*/
			LoopService(ServiceManager *manager, const std::string& name);
			virtual ~LoopService();
			
			/** Creates a loop mode specified by the definition
			* @param modeDef The loop mode definition. The mode should have a unique ID (Is checked)
			* @note If the mode has an loop ID of a loop mode which has already been created, error is logged and loop is not created
			*/
			void createLoopMode(const LoopModeDefinition& modeDef);
			
			/** Adds a loop client. The client is automatically added to all loops with the complying masks (mode mask & client mask != 0)
			* @param client The loop client going to be inserted
			* @note The client has to have a unique ID. The loop mode definition of the client is gathered with LoopClient::getLoopClientSpecification 
			* @note shared_ptr is not used here because of some problems expected with reference counts 
			*/
			void addLoopClient(LoopClient* client);
			
			/** Removes a loop client from all the loop modes.
			* @param client The loop client to be removed
			* @note shared_ptr is not used here because of some problems expected with reference counts 
			*/
			void removeLoopClient(LoopClient* client);
			
			/** Requests a transition to a new loop mode
			* @param newLoopMode The new loop mode to be used. If the loop mode is invalid, the loop service's run() method will terminate
			* @return true if the loop mode was found and request was queued, false otherwise
			*/
			bool requestLoopMode(LoopModeID newLoopMode);
			
			/** Requests a transition to a new loop mode
			* @param newLoopMode The new loop mode to be used. If the loop mode is invalid, the loop service's run() method will terminate
			* @return true if the loop mode was found and request was queued, false otherwise
			*/
			bool requestLoopMode(const std::string& name);
			
			/// Requests the termination of the loop (program exit)
			void requestTermination();
			
			/// The main run code. Run by the main.cpp typically. Runs the application loops
			void run();

			/// @return The current absolute time in ms
			unsigned long getCurrentTime() { return mRoot->getTimer()->getMilliseconds(); };
			
			/// Logs DEBUG level info of one frame timings
			void debugOneFrame();

		protected:
			/// Service intialization
			bool init();
			
			/** sets a new loop mode
			*/
			void setLoopMode(LoopModePtr newMode);
			
			/// Termination was requested
			bool mTerminationRequested;
			
			/// The new requested loop mode (if the mNewModeRequested is true)
			LoopModePtr mNewLoopMode;
			
			/// True if something requested a change in the loop mode
			bool mNewModeRequested;
			
			/// The active loop mode
			LoopModePtr mActiveMode;
			
			typedef std::map<LoopModeID, LoopModePtr > LoopModeIDMap;
			typedef std::map<std::string, LoopModePtr > LoopModeNameMap;
			
			typedef std::list< LoopClient* > LoopClientList;
			
			/// Loop mode map - per ID
			LoopModeIDMap mLoopModes;
			/// Loop mode map - per Name
			LoopModeNameMap mLoopNamedModes;
			
			/// List of registered loop clients
			LoopClientList mLoopClients;
			
			/// Ogre::Root for timing purposes
			Ogre::Root* mRoot;
			
			/// Time of the last frame (absolute)
			unsigned long mLastFrameTime;
			
			bool mDebugOneFrame;
	};
	

	/// Shared pointer to game service
	typedef shared_ptr<LoopService> LoopServicePtr;


	/// Factory for the LoopService objects
	class LoopServiceFactory : public ServiceFactory {
		public:
			LoopServiceFactory();
			~LoopServiceFactory() {};

			/** Creates a GameService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

		private:
			static std::string mName;
	};
}


#endif
