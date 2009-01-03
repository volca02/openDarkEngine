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


#ifndef __PHYSICSSERVICE_H
#define __PHYSICSSERVICE_H

#include "config.h"

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "DatabaseService.h"
#include "FileGroup.h"
#include "SharedPtr.h"

#include "ode/ode.h"

namespace Opde {

	/** @brief Physics service - service defining game states (Temporary code. Will be filled with a high level state management - screens)
	*/
	class OPDELIB_EXPORT PhysicsService : public Service {
		public:
			PhysicsService(ServiceManager *manager, const std::string& name);
			virtual ~PhysicsService();

		protected:
			bool init();

			DatabaseServicePtr mDbService;

			//Dark World
			dWorldID dDarkWorldID;
	};

	/// Shared pointer to Physics service
	typedef shared_ptr<PhysicsService> PhysicsServicePtr;


	/// Factory for the PhysicsService objects
	class OPDELIB_EXPORT PhysicsServiceFactory : public ServiceFactory {
		public:
			PhysicsServiceFactory();
			~PhysicsServiceFactory() {};

			/** Creates a PhysicsService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

			virtual const uint getMask(); 
		private:
			static std::string mName;
	};
}


#endif
