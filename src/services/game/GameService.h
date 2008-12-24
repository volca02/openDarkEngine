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


#ifndef __GAMESERVICE_H
#define __GAMESERVICE_H

#include "config.h"

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "DatabaseService.h"
#include "FileGroup.h"
#include "SharedPtr.h"

namespace Opde {

	/** @brief Game service - service defining game states (Temporary code. Will be filled with a high level state management - screens)
	*/
	class OPDELIB_EXPORT GameService : public Service {
		public:
			GameService(ServiceManager *manager, const std::string& name);
			virtual ~GameService();

			/// Loads a game database using the database service
			void load(const std::string& filename);

		protected:
			bool init();

			DatabaseServicePtr mDbService;
	};

	/// Shared pointer to game service
	typedef shared_ptr<GameService> GameServicePtr;


	/// Factory for the GameService objects
	class OPDELIB_EXPORT GameServiceFactory : public ServiceFactory {
		public:
			GameServiceFactory();
			~GameServiceFactory() {};

			/** Creates a GameService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

			virtual const uint getMask(); 
		private:
			static std::string mName;
	};
}


#endif
