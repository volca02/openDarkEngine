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

	
	/*
 * FILE_TYPE [*] [*]
 * Magic-number of file.
 * Seems to be a bitmap representing what chunks might
 * appear in the file, but I can't find any specific associations.
 * Some archaic COWs I have used only the high-word.

struct DarkDBChunkFILE_TYPE
{
        uint32  type;
};
#define FILE_TYPE_VBR   0x00000500 // 0000 0000 0101
#define FILE_TYPE_SAV   0x00011900 // 0001 0001 1001
#define FILE_TYPE_MIS   0x00031900 // 0011 0001 1001
#define FILE_TYPE_GAM   0x00040200 // 0100 0000 0010
#define FILE_TYPE_COW   0x00071F00 // 0111 0001 1111
#define FILE_TYPE_MASK_ARCHAIC  0xFFFF0000
// bit 8 - not on GAM
// bit 9 - only on GAM
// bit 10 - only on VBR
// bit 11 - only on MIS/SAV
// bit 12 - only on MIS/SAV
// bit 16 - MIS/SAV
// bit 17 - MIS
// bit 18 - GAM
	*/
	
	/** Database type encoding. Designed to be compatible with the FILE_TYPE tag for later replacement.
	* All this info derived directly from Telliamed's DarkUtils FILE_TYPE description (Thanks again for all the work).
	*
	* [....:....|....:.A98|7654:3210|....:....]
	*
	* The high 3 bits (A98) contain historical data distribution guidelines: (A == ABSTRACT OBJECTS, 9==MISSON DATA, 8==CONCRETE OBJECTS)
	*
	* The Second lowest byte contains newer data separation guidelines (to be combined with the high word):
	*
	* Concrete data : bit 1 (1). Active on databases that contain concrete object tree
	* Abstract data : bit 2 (2). Active on databases that contain gamesys (abstract object tree and other abstract definitions - recipes)
	* VBR      data : bit 3 (4). Active only on VBR databases - that contain brush lists
	* 
	*/
	typedef enum {
		/// Contains concrete objects (not necessarily object database!)
		DBM_CONCRETE = 0x0100,
		/// Contains Abstract objects
		DBM_ABSTRACT = 0x0200,
		/// Multibrush database - indicates multibrushes/minibrushes are present - VBR files have this (those have 0x0500 mask to be precise). Verified. 
		DBM_MBRUSHES  = 0x0400,
		/// Unknown. Present in cow only
		DBM_UNKNOWN1  = 0x0800,
		/// Contains concrete objects/links/properties, probably (or maybe AI state, sound propagation state, dunno...). A Mask because I can't yet separate which is what
		DBM_CONCRETE_OLP  = 0x11900, // TODO: Decompose this one
		// High level bits (old encoding):
		/// Concrete object mask
		DBM_OBJTREE_CONCRETE = 0x010000,
		/// Mission data present mask
		DBM_MIS_DATA = 0x020000,
		/// Gamesys data present
		DBM_OBJTREE_GAMESYS = 0x040000,
		/// A complete - full database (e.g. VBR, also means all for DB_DROPPING)
		DBM_COMPLETE = 0x071F00,
		// File types follow
		/// GAM file type
		DBM_FILETYPE_GAM = 0x040200,
		/// VBR file type
		DBM_FILETYPE_VBR = 0x000500,
		/// SAV file type
		DBM_FILETYPE_SAV = 0x011900,
		/// MIS file type
		DBM_FILETYPE_MIS = 0x031900,
		/// COW file type
		DBM_FILETYPE_COW = 0x071F00,
	} DatabaseMask;

	/// The database change message
	/*
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
	*/
	
	
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
	
	// Forward decl.
	class DatabaseService;
	
	/** Listener for database events. Pure abstract
	* @todo We need flags for various parts of the object system (compatible with the mask 'FILE_TYPE' in tag file databases)
	*/
	class DatabaseListener {
		friend class DatabaseService;
		
		protected:
			virtual void onDBLoad(const FileGroupPtr& db, uint32_t curmask) = 0;
			virtual void onDBSave(const FileGroupPtr& db, uint32_t tgtmask) = 0;
			virtual void onDBDrop(uint32_t dropmask) = 0;
			
	};

	/** @brief Database service - service which handles dark database loading and saving
	* Typically, a service wanting to handle database events will register as a listener, and handle the events given from this service.
	* There are some rules:
	* @li Mission loading will drop with DB_COMPLETE
	* @li Savegame loading will have DB_SAVEGAME as dbtarget (so the services handling savegame related chunks will not process the mission's chunks)
	*/
	class OPDELIB_EXPORT DatabaseService : public ServiceImpl<DatabaseService> {
		public:
			DatabaseService(ServiceManager *manager, const std::string& name);
			virtual ~DatabaseService();

			/// High-level load. Loads a game database including parent databases as needed, dropping previous loaded data
			void load(const std::string& filename, uint32_t loadMask);
			
			/// Loads a game database without dropping any data
			void mergeLoad(const std::string& filename, uint32_t loadMask);
			
			/// Recursive load of all databases in hierarchy, without dropping
			void recursiveMergeLoad(const std::string& filename, uint32_t loadMask);
			
			/// Saves a game database, writing all data fitting the specified mask
			void save(const std::string& filename, uint32_t loadMask);
			
			/// Unload the game data. Release all the data fitting the mask specified
			void unload(uint32_t dropMask);
	
			/// Listener that receives events every now and then while loading
			typedef Callback<DatabaseProgressMsg> ProgressListener;
			
			/// Progress Listener shared_ptr
			typedef shared_ptr<ProgressListener> ProgressListenerPtr;
			
			/// Setter for the progress listener.
			void setProgressListener(const ProgressListenerPtr& listener) { mProgressListener = listener; };
            
			/// Clears (unsets) the progress listener (disabling it)
			void unsetProgressListener() { mProgressListener.setNull(); };
	
			/// A free to use fine step function that calls the Progress Listener to reflect the loading progress
			/// Use this especially in some long-to load services
			void fineStep(int count);
			
			/** Registers a listener.
			* @param listener A pointer to the listening class
			* @param priority desired loading priority (order) of the listener
			* @note The same pointer has to be supplied to the unregisterListener in order to succeed with unregistration
			*/
			void registerListener(DatabaseListener* listener, size_t priority);

			/** Unregisters a listener.
			* @param listener ID returned by the registerListener call
			* @note The pointer has to be the same as the one supplied to the registerListener
			*/
			void unregisterListener(DatabaseListener* listener);
			
		protected:
			virtual bool init();
			
			/** Reads the File_Type tag from the specified database. 
			* @param db The database to load the tag value from
			* @return the FILE_TYPE first 4 bytes as uint32_t, or 0 if something bad happens (not found, tag too short)
			*/
			uint32_t getFileType(const FileGroupPtr& db);

			/// Retrieve a readonly database file by it's name
			FileGroupPtr getDBFileNamed(const std::string& filename);

			/** Gets the database tag name containing the file name of the parent database based on the FILE_TYPE value */
			const char* getParentDBTagName(uint32_t fileType);
			
			/** loads a file name from the specified tag */
			std::string loadFileNameFromTag(const FileGroupPtr& db, const char* tagname);
			
			/// Calls onDBLoad on all listeners obeying priorities
			void broadcastOnDBLoad(const FileGroupPtr& db, uint32_t curmask);
			
			/// Calls onDBLoad on all listeners obeying priorities
			void broadcastOnDBSave(const FileGroupPtr& db, uint32_t tgtmask);
			
			/// Calls onDBLoad on all listeners obeying priorities
			void broadcastOnDBDrop(uint32_t dropmask);
			
			FileGroupPtr mCurDB;
			
			/// Used to report to the Progress Listener
			DatabaseProgressMsg mLoadingStatus;
			
			ProgressListenerPtr mProgressListener;
			
			/// Map of db. load listeners
			typedef std::multimap< size_t, DatabaseListener* > Listeners;
			
			Listeners mListeners;
	};

	/// Shared pointer to Database service
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
			
			virtual const size_t getSID();
		private:
			static std::string mName;
	};
}


#endif
