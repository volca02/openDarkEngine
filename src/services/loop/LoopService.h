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


#ifndef __LOOPSERVICE_H
#define __LOOPSERVICE_H

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "SharedPtr.h"
#include "Callback.h"
#include "ConfigService.h"

namespace Opde {

	
	typedef struct {
	} LoopClient;


	/** @brief Loop Service - service which handles game loop (per-frame loop) */
	class LoopService : public Service {
		public:
			LoopService(ServiceManager *manager, const std::string& name);
			virtual ~LoopService();
			
		protected:
			bool init();
			
	};

	/// Shared pointer to game service
	typedef shared_ptr<LoopService> LoopServicePtr;

	/// Factory for the GameService objects
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
