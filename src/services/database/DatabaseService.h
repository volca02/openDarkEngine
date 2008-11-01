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


#ifndef __DATABASESERVICE_H
#define __DATABASESERVICE_H

#include "config.h"

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
	struct DatabaseChangeMsg {
		/// A change requested to happen
		DatabaseChangeType change;
		/// Type of the database distributed by this event
		DatabaseType dbtype;
		/// Type of the target database - valid options are DB_MISSION, DB_SAVEGAME and DB_COMPLETE
		DatabaseType dbtarget;
		/// The pointer to the database file to be used
		FileGroupPtr db;
	};
	
	
	/// Progress report of database loading. This is what the Progress Listener get's every now and then to update the display
	struct DatabaseProgressMsg {
		/// Completion status - 0.0-1.0
		float completed; // Only the coarse steps are included here. The fine steps are not
		/// Total coarse step count
		int totalCoarse; 
		/// Current count of the coarse steps
		int currentCoarse;
		/// Overall count of the fine steps
		int overallFine; // Increased on every DatabaseService::fineStep (not cleared). Means the overall step count that happened
		
		/// Recalculates the completed
		void recalc() {
			if (totalCoarse > 0) {
				completed = static_cast<float>(currentCoarse) / static_cast<float>(totalCoarse);
			}
		}
		
		void reset() {
			completed = 0;
			totalCoarse = 0;
			currentCoarse = 0;
			overallFine = 0;
		}
	};

	/** @brief Database service - service which handles dark database loading and saving
	* Typically, a service wanting to handle database events will register as a listener, and handle the events given from this service.
	* There are some rules:
	* @li Mission loading will drop with DB_COMPLETE
	* @li Savegame loading will have DB_SAVEGAME as dbtarget (so the services handling savegame related chunks will not process the mission's chunks)
	*/
	class OPDELIB_EXPORT DatabaseService : public Service, public PrioritizedMessageSource<DatabaseChangeMsg> {
		public:
			DatabaseService(ServiceManager *manager, const std::string& name);
			virtual ~DatabaseService();

			/// Loads a game database. Can be either savegame, or mission
			void load(const std::string& filename);
			
			/// Loads a game system. drops all current data
			void loadGameSys(const std::string& filename);

			/// Unload the game data. Release all the data that are connected to a game's mission in progress
			void unload();
	
			/// Listener that receives events every now and then while loading
			typedef Callback<DatabaseProgressMsg> ProgressListener;
			
			/// Progress Listener shared_ptr
			typedef shared_ptr<ProgressListener> ProgressListenerPtr;
			
			/// Setter for the progress listener.
			void setProgressListener(const ProgressListenerPtr& listener) { mProgressListener = listener; };
            
			/// Clears (unsets) the progress listener (disabling it)
			void unsetProgressListener() { mProgressListener = NULL; };
	
			/// A free to use fine step function that calls the Progress Listener to reflect the loading progress
			/// Use this especially in some long-to load services
			void fineStep(int count);
			
		protected:
			virtual bool init();

			/// Retrieve a readonly database file by it's name
			FileGroupPtr getDBFileNamed(const std::string& filename);

			/// Load and assign a mission database to the db (has to be a SaveGame), then loads the gamesys for the loaded miss file
			void _loadMissionDB(const FileGroupPtr& db);

			/// Load and assign a gamesys database to the db (has to be a mission or savegame)
			void _loadGameSysDB(const FileGroupPtr& db);

			/// Overriden broadcast to support progress reports
			virtual void broadcastMessage(const DatabaseChangeMsg& msg);

			/// a reverse order message broadcaster - this one is used in unloading
			virtual void broadcastMessageReversed(const DatabaseChangeMsg& msg);

			FileGroupPtr mCurDB;
			
			/// Used to report to the Progress Listener
			DatabaseProgressMsg mLoadingStatus;
			
			ProgressListenerPtr mProgressListener;
	};

	/// Shared pointer to game service
	typedef shared_ptr<DatabaseService> DatabaseServicePtr;


	/// Factory for the DatabaseService objects
	class OPDELIB_EXPORT DatabaseServiceFactory : public ServiceFactory {
		public:
			DatabaseServiceFactory();
			~DatabaseServiceFactory() {};

			/** Creates a DatabaseService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

			virtual const uint getMask(); 
		private:
			static std::string mName;
	};
}


#endif
