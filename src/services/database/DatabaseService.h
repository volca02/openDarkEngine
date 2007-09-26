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


#ifndef __DATABASESERVICE_H
#define __DATABASESERVICE_H

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "FileGroup.h"
#include "SharedPtr.h"
#include "PrioritizedMessageSource.h"

namespace Opde {

	/// Database event types
	typedef enum {
	    /// Loading a new database
	    DBC_LOADING = 1,
	    /// Saving a database
	    DBC_SAVING,
	    /// Dropping a database
	    DBC_DROPPING
	} DatabaseChangeType;

	/// Database types
	typedef enum {
	    /// Game system database
	    DBT_GAMESYS = 1,
	    /// Mission file database
	    DBT_MISSION,
	    /// Savegame database
	    DBT_SAVEGAME,
	    /// A complete - full database (e.g. VBR, also means all for DB_DROPPING)
	    DBT_COMPLETE
	} DatabaseType;

	/// The database change message
	typedef struct DatabaseChangeMsg {
	    /// A change requested to happen
	    DatabaseChangeType change;
	    /// Type of the database distributed by this event
	    DatabaseType dbtype;
	    /// Type of the target database - valid options are DB_MISSION, DB_SAVEGAME and DB_COMPLETE
	    DatabaseType dbtarget;
	    /// The pointer to the database file to be used
	    FileGroupPtr db;
	};

	/** @brief Database service - service which handles dark database loading and saving
	* Typically, a service wanting to handle database events will register as a listener, and handle the events given from this service.
	* There are some rules:
	* @li Mission loading will drop with DB_COMPLETE
	* @li Savegame loading will have DB_SAVEGAME as dbtarget (so the services handling savegame releted chunks will not process the mission's chunks)
	*/
	class DatabaseService : public Service, public PrioritizedMessageSource<DatabaseChangeMsg> {
		public:
			DatabaseService(ServiceManager *manager);
			virtual ~DatabaseService();

			/// Loads a game database. Can be either savegame, or mission
			void load(const std::string& filename);

			/// Unload the game data. Release all the data that are connected to a game's mission in progress
			void unload();
		protected:
            virtual void init();

			/// Retrieve a readonly database file by it's name
			FileGroupPtr getDBFileNamed(const std::string& filename);

			/// Load and assign a mission database to the db (has to be a SaveGame), then loads the gamesys for the loaded miss file
			void _loadMissionDB(FileGroupPtr db);

			/// Load and assign a gamesys database to the db (has to be a mission or savegame)
			void _loadGameSysDB(FileGroupPtr db);

			FileGroupPtr mCurDB;
	};

	/// Shared pointer to game service
	typedef shared_ptr<DatabaseService> DatabaseServicePtr;


	/// Factory for the GameService objects
	class DatabaseServiceFactory : public ServiceFactory {
		public:
			DatabaseServiceFactory();
			~DatabaseServiceFactory() {};

			/** Creates a GameService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

		private:
			static std::string mName;
	};
}


#endif
