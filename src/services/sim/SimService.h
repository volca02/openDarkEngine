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

#include "config.h"

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "LoopService.h"
#include "SharedPtr.h"

namespace Opde {
	/** Abstract Simulation listener - a class that does something related to simulation extends this
	*/
	class OPDELIB_EXPORT SimListener {
		public:
			virtual void simStarted();
			
			virtual void simEnded();
			
			virtual void simStep(float simTime);
			
			/// Priority of the listener
			virtual size_t getPriority();
	};

	/** @brief Sim Service - a simulation timer base. Implements Sim time (simulation time). This time can have different flow rate than normal time.
	  *  All simulation related listeners register here, not via loop service, which only handles game loop (and does not have the capability to pause/stretch time).
	*/
	class OPDELIB_EXPORT SimService : public Service, LoopClient {
		public:
			SimService(ServiceManager *manager, const std::string& name);
			virtual ~SimService();

			// --- Registration/Unregistration of clients
			void registerListener(SimListener* listener);
			
			void unregisterListener(SimListener* listener);
			
			// --- Loop Service Client related ---
			void loopStep(float deltaTime);

		protected:
			/// Service initialization
			bool init();
			
			/// Time flow coefficient - 1.0 means the sim time is 1:1 with real time. 0.5 means sim time flows 2 times slower than normal time
			float mTimeCoeff;
			
			/// Sim time. Set to zero on simulation start
			float mSimTime;
			
			/// Simulation listeners - multimap, as we need prioritised behavior 
			typedef std::multimap<size_t, SimListener*> SimListeners;
			
			SimListeners mSimListeners;
	};

	/// Shared pointer to Sim service
	typedef shared_ptr<SimService> SimServicePtr;


	/// Factory for the SimService objects
	class OPDELIB_EXPORT SimServiceFactory : public ServiceFactory {
		public:
			SimServiceFactory();
			~SimServiceFactory() {};

			/** Creates a SimService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

			virtual const uint getMask(); 
		private:
			static std::string mName;
	};
}


#endif
