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
#include "DatabaseService.h"
#include "OpdeException.h"
#include "logger.h"

#include <OgreResourceGroupManager.h>

using namespace std;
using namespace Ogre;

namespace Opde {

	/*--------------------------------------------------------*/
	/*-------------------- DatabaseService -------------------*/
	/*--------------------------------------------------------*/
	DatabaseService::DatabaseService(ServiceManager *manager, const std::string& name) : Service(manager, name), mCurDB(NULL), mProgressListener(NULL) {

	}

    //------------------------------------------------------
    bool DatabaseService::init() {
        // Create all services listening to the database messages...
	    ServiceManager::getSingleton().createByMask(SERVICE_DATABASE_LISTENER);
			return true;
    }

	//------------------------------------------------------
	DatabaseService::~DatabaseService() {
	}

	//------------------------------------------------------
	void DatabaseService::load(const std::string& filename) {
	    LOG_DEBUG("DatabaseService::load - Loading requested file %s", filename.c_str());
        // if there is another database in hold
		if (!mCurDB.isNull())
			unload();

	    // Try to find the database
		mCurDB = getDBFileNamed(filename);

        mLoadingStatus.reset();
        
        // TODO: Overall coarse step - calculated from TagFile Count * mListeners.size()
        mLoadingStatus.totalCoarse = mListeners.size() * 2;
        
		// Currently hardcoded to mission db
		_loadMissionDB(mCurDB);

		LOG_DEBUG("DatabaseService::load - end()");
	}
	
	//------------------------------------------------------
	void DatabaseService::loadGameSys(const std::string& filename) {
	    LOG_DEBUG("DatabaseService::loadGameSys - Loading requested file %s", filename.c_str());
        // if there is another database in hold
		if (!mCurDB.isNull())
			unload();

	    // Try to find the database
		mCurDB = getDBFileNamed(filename);

        mLoadingStatus.reset();
        
        // TODO: Overall coarse step - calculated from TagFile Count * mListeners.size()
        mLoadingStatus.totalCoarse = mListeners.size();
        
		// Currently hardcoded to mission db
		_loadGameSysDB(mCurDB);

		LOG_DEBUG("DatabaseService::loadGameSys - end()");
	}

	//------------------------------------------------------
	FileGroupPtr DatabaseService::getDBFileNamed(const std::string& filename) {
	    // TODO: Group of of the resource through the configuration service, once written
	    Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource(filename, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);
		FilePtr fp = new OgreFile(stream);

		return new DarkFileGroup(fp);
	}

	//------------------------------------------------------
	void DatabaseService::unload() {
	    LOG_DEBUG("DatabaseService::unload");
	    DatabaseChangeMsg m;

	    m.change = DBC_DROPPING;
        m.dbtype = DBT_COMPLETE;
        m.dbtarget = DBT_COMPLETE; // No meaning here, at least now.
        // ^ It could be used to unload f.e. only mission or savegame, thus saving reload. This is a nice subject to think about, yes

        m.db = mCurDB;

        broadcastMessageReversed(m);

		/// Wipe out the files we used
		mCurDB.setNull();

		LOG_DEBUG("DatabaseService::unload - end()");
	}
	
	//------------------------------------------------------
	void DatabaseService::fineStep(int count) {
	    // recalculate the status
        mLoadingStatus.overallFine += count;
        
        // Won't probably do anything, but could, in theory
        mLoadingStatus.recalc();
                    
        // call the progress listener if it is set
        if (!mProgressListener.isNull()) {
            (*mProgressListener)(mLoadingStatus);
        }
	}

	//------------------------------------------------------
	void DatabaseService::_loadMissionDB(const FileGroupPtr& db) {
	    LOG_DEBUG("DatabaseService::_loadMissionDB");

		// Get the gamesys, load
		// GAM_FILE
		FilePtr fdm = db->getFile("GAM_FILE");

		// never happened
		size_t gft_size = fdm->size();
		char* data = new char[gft_size + 1];

		data[0] = 0x0;
		data[gft_size] = 0x0;

		fdm->read(data, fdm->size());

		FileGroupPtr gs = getDBFileNamed(data);

		delete[] data;

		_loadGameSysDB(gs);

		// Load the Mission
        // Create a DB change message, and broadcast
        DatabaseChangeMsg m;

        m.change = DBC_LOADING;
        m.dbtype = DBT_MISSION;
        m.dbtarget = DBT_MISSION; // TODO: Hardcoded
        m.db = db;

        broadcastMessage(m);
	}

	//------------------------------------------------------
	void DatabaseService::_loadGameSysDB(const FileGroupPtr& db) {
	    LOG_DEBUG("DatabaseService::_loadGameSysDB");

        // Create a DB change message, and broadcast
        DatabaseChangeMsg m;

        m.change = DBC_LOADING;
        m.dbtype = DBT_GAMESYS;
        m.dbtarget = DBT_MISSION; // TODO: Hardcoded
        m.db = db;

        broadcastMessage(m);
	}

  
    //------------------------------------------------------
    void DatabaseService::broadcastMessage(const DatabaseChangeMsg& msg) {
        Listeners::iterator it = mListeners.begin();

        for (; it != mListeners.end(); ++it) {
            // Use the callback functor to fire the callback
            (*it->second)(msg);
            
            // recalculate the status
            mLoadingStatus.currentCoarse++;
            
            mLoadingStatus.recalc();
                        
            // call the progress listener if it is set
            if (!mProgressListener.isNull()) {
                (*mProgressListener)(mLoadingStatus);
            }
        }
    }

	//------------------------------------------------------
    void DatabaseService::broadcastMessageReversed(const DatabaseChangeMsg& msg) {
        Listeners::reverse_iterator it = mListeners.rbegin();

        for (; it != mListeners.rend(); ++it) {
            // Use the callback functor to fire the callback
            (*it->second)(msg);
            
            // recalculate the status
            mLoadingStatus.currentCoarse++;
            
            mLoadingStatus.recalc();
                        
            // call the progress listener if it is set
            if (!mProgressListener.isNull()) {
                (*mProgressListener)(mLoadingStatus);
            }
        }
    }

	//-------------------------- Factory implementation
	std::string DatabaseServiceFactory::mName = "DatabaseService";

	DatabaseServiceFactory::DatabaseServiceFactory() : ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
	};

	const std::string& DatabaseServiceFactory::getName() {
		return mName;
	}

	const uint DatabaseServiceFactory::getMask() {
		return SERVICE_CORE;
	}

	Service* DatabaseServiceFactory::createInstance(ServiceManager* manager) {
		return new DatabaseService(manager, mName);
	}

}
