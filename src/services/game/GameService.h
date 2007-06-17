/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2007 openDarkEngine team
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 * http://www.gnu.org/copyleft/lesser.txt.
 *****************************************************************************/
 
#ifndef __GAMESERVICE_H
#define __GAMESERVICE_H

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "FileGroup.h"
#include "SharedPtr.h"

namespace Opde {
	
	/** @brief Game service - service defining game states, and mission loading/saving
	*/
	class GameService : public Service {
		public:
			GameService(ServiceManager *manager);
			virtual ~GameService();
			
			/// Loads a game database. Can be either savegame, or mission
			void load(const std::string& filename);
			
			/// Unload the game data. Release all the data that are connected to a game's mission in progress
			void unload();
		protected:
			
			/// Retrieve a readonly database file by it's name
			DarkFileGroup* getDBFileNamed(const std::string& filename);
			
			/// Load and assign a mission database to the db (has to be a SaveGame), then loads the gamesys for the loaded miss file
			void _loadMissionDB(DarkFileGroup* db);
			
			/// Load and assign a gamesys database to the db (has to be a mission or savegame)
			void _loadGameSysDB(DarkFileGroup* db);
			
			DarkFileGroup* mCurDB;
	};
	
	/// Shared pointer to game service
	typedef shared_ptr<GameService> GameServicePtr;
	
	
	/// Factory for the GameService objects
	class GameServiceFactory : public ServiceFactory {
		public:
			GameServiceFactory();
			~GameServiceFactory() {};
			
			/** Creates a GameService instance */
			Service* createInstance(ServiceManager* manager);
			
			virtual const std::string& getName();
		
		private:
			static std::string mName;
	};
}
 
 
#endif
