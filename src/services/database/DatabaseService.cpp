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

#include "ServiceCommon.h"
#include "DatabaseService.h"
#include "OpdeException.h"
#include "logger.h"

#include <OgreTimer.h>
#include <OgreResourceGroupManager.h>

using namespace std;
using namespace Ogre;

namespace Opde {

/*--------------------------------------------------------*/
/*-------------------- DatabaseService -------------------*/
/*--------------------------------------------------------*/
template<> const size_t ServiceImpl<DatabaseService>::SID = __SERVICE_ID_DATABASE;

DatabaseService::DatabaseService(ServiceManager *manager, const std::string& name) :
    ServiceImpl<DatabaseService>(manager, name),
    mCurDB(NULL),
    mProgressListener(NULL),
    mTimer(NULL) {

}

//------------------------------------------------------
bool DatabaseService::init() {
    mTimer = new Ogre::Timer();
    // Create all services listening to the database messages...
    ServiceManager::getSingleton().createByMask(SERVICE_DATABASE_LISTENER);
    return true;
}

//------------------------------------------------------
DatabaseService::~DatabaseService() {
    delete mTimer;
}

//------------------------------------------------------
void DatabaseService::load(const std::string& filename, uint32_t loadMask) {
    LOG_DEBUG("DatabaseService::load - Loading requested file %s", filename.c_str());

    mLoadingStatus.reset();

    // drop first
    unload(DBM_COMPLETE);

    // do a recursive load as needed
    recursiveMergeLoad(filename, loadMask);

    LOG_DEBUG("DatabaseService::load - end()");
}

//------------------------------------------------------
void DatabaseService::recursiveMergeLoad(const std::string& filename, uint32_t loadMask) {
    // load the database, but see for the parents first
    LOG_INFO("DatabaseService::recursiveMergeLoad - Loading file %s", filename.c_str());

    FileGroupPtr db = getDBFileNamed(filename);

    // the coarse count is lifted every call to ensure all the databases are counted
    mLoadingStatus.totalCoarse += mListeners.size();

    uint32_t ft = getFileType(db);

    const char* parentDbTag = getParentDBTagName(ft);

    if (parentDbTag != NULL) {
        // load the parent first
        // but filter out any bits overriden by overlay database
        // this is to ensure that .sav data will be loaded instead of .mis, for example

        std::string parentFile = loadFileNameFromTag(db, parentDbTag);

        LOG_INFO("DatabaseService::recursiveMergeLoad - Recursing to file %s", parentFile.c_str());
        recursiveMergeLoad(parentFile, loadMask & ~ft);
    }

    LOG_INFO("DatabaseService::recursiveMergeLoad - Recurse end. Loading file %s", filename.c_str());
    broadcastOnDBLoad(db, ft & loadMask);
}

//------------------------------------------------------
void DatabaseService::mergeLoad(const std::string& filename, uint32_t loadMask) {
    FileGroupPtr db = getDBFileNamed(filename);
    uint32_t ft = getFileType(db);

    LOG_DEBUG("DatabaseService::mergeLoad - Merge load of file %s", filename.c_str());

    mLoadingStatus.reset();
    mLoadingStatus.totalCoarse += mListeners.size();

    broadcastOnDBLoad(db, ft & loadMask);

    LOG_DEBUG("DatabaseService::mergeLoad - end()");
}

//------------------------------------------------------
void DatabaseService::save(const std::string& filename, uint32_t saveMask) {
    // just prepare the progress and delegate to the broadcast
    FilePtr fp = FilePtr(new StdFile(filename, File::FILE_RW));

    FileGroupPtr tgtdb = FileGroupPtr(new DarkFileGroup());

    LOG_DEBUG("DatabaseService::save - Save to file %s, mask %X", filename.c_str(), saveMask);

    mLoadingStatus.reset();
    mLoadingStatus.totalCoarse += mListeners.size();

    broadcastOnDBSave(tgtdb, saveMask);

    // And write the saveMask as FILE_TYPE
    FilePtr fpf = tgtdb->createFile("FILE_TYPE", 0, 1); // The version is fixed, really. Nothing to invent here
    fpf->writeElem(&saveMask, sizeof(uint32_t));

    // And Write!
    tgtdb->write(fp);
}

//------------------------------------------------------
void DatabaseService::unload(uint32_t dropMask) {
    LOG_DEBUG("DatabaseService::unload");

    broadcastOnDBDrop(dropMask);

    /// Wipe out the file we used
    mCurDB.reset();

    LOG_DEBUG("DatabaseService::unload - end()");
}

//------------------------------------------------------
FileGroupPtr DatabaseService::getDBFileNamed(const std::string& filename) {
    // TODO: Group of of the resource through the configuration service, once written
    Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource(filename, ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
    FilePtr fp(new OgreFile(stream));

    return FileGroupPtr(new DarkFileGroup(fp));
}

//------------------------------------------------------
void DatabaseService::fineStep(int count) {
    // recalculate the status
    mLoadingStatus.overallFine += count;

    // Won't probably do anything, but could, in theory
    mLoadingStatus.recalc();

    // call the progress listener if it is set
    if (mProgressListener) {
        (*mProgressListener)(mLoadingStatus);
    }
}

//------------------------------------------------------
void DatabaseService::registerListener(DatabaseListener* listener, size_t priority) {
    mListeners.insert(std::make_pair(priority, listener));
}

//------------------------------------------------------
void DatabaseService::unregisterListener(DatabaseListener* listener) {
    Listeners::iterator it = mListeners.begin();

    while (it != mListeners.end()) {

        if (it->second == listener) {
            Listeners::iterator rem = it++;
            mListeners.erase(rem);
        } else {
            it++;
        }
    }
}

//------------------------------------------------------
uint32_t DatabaseService::getFileType(const FileGroupPtr& db) {
    // TODO: load FILE_TYPE for the first parameter
    FilePtr fttag = db->getFile("FILE_TYPE");

    if (!fttag) {
        // TODO: Exception, rather
        LOG_FATAL("Database file did not contain FILE_TYPE tag");
        return 0;
    }


    if (fttag->size() < sizeof(uint32_t)) {
        // TODO: Exception, rather
        LOG_FATAL("Database file did contain an invalid FILE_TYPE tag");
        return 0;
    }

    uint32_t filetype;

    fttag->readElem(&filetype, sizeof(uint32_t));

    return filetype;
}

//------------------------------------------------------
const char* DatabaseService::getParentDBTagName(uint32_t fileType) {
    // fixed for now
    if (fileType == DBM_FILETYPE_SAV)
        return "MIS_FILE";

    if (fileType == DBM_FILETYPE_MIS)
        return "GAM_FILE";

    return NULL;
}

//------------------------------------------------------
std::string DatabaseService::loadFileNameFromTag(const Opde::FileGroupPtr& db, const char* tagname) {
    FilePtr fdm = db->getFile(tagname);

    size_t gft_size = fdm->size();

    std::string res;

    char* data = NULL;

    data = new char[gft_size + 1];
    data[0] = 0x0;
    data[gft_size] = 0x0;

    fdm->read(data, fdm->size()); // TODO: Catch exception

    res = std::string(data);

    delete[] data;


    return res;
}

//------------------------------------------------------
void DatabaseService::broadcastOnDBLoad(const FileGroupPtr& db, uint32_t curmask) {
    Listeners::iterator it = mListeners.begin();

    for (; it != mListeners.end(); ++it) {
        unsigned long sttime = mTimer->getMilliseconds();

        // Inform about the load
        it->second->onDBLoad(db, curmask);

        unsigned long now = mTimer->getMilliseconds();

        LOG_INFO("DatabaseService: Operation took %f seconds", (float)(now-sttime) / 1000);

        // recalculate the status
        mLoadingStatus.currentCoarse++;

        mLoadingStatus.recalc();

        // call the progress listener if it is set
        if (mProgressListener) {
            (*mProgressListener)(mLoadingStatus);
        }
    }
}

//------------------------------------------------------
void DatabaseService::broadcastOnDBSave(const FileGroupPtr& db, uint32_t tgtmask) {
    Listeners::iterator it = mListeners.begin();

    for (; it != mListeners.end(); ++it) {
        unsigned long sttime = mTimer->getMilliseconds();

        // Inform about the save
        it->second->onDBSave(db, tgtmask);

        unsigned long now = mTimer->getMilliseconds();

        LOG_INFO("DatabaseService: Operation took %f seconds", (float)(now-sttime) / 1000);

        // recalculate the status
        mLoadingStatus.currentCoarse++;

        mLoadingStatus.recalc();

        // call the progress listener if it is set
        if (mProgressListener) {
            (*mProgressListener)(mLoadingStatus);
        }
    }
}

//------------------------------------------------------
void DatabaseService::broadcastOnDBDrop(uint32_t dropmask) {
    Listeners::reverse_iterator it = mListeners.rbegin();

    for (; it != mListeners.rend(); ++it) {
        unsigned long sttime = mTimer->getMilliseconds();

        // Inform about the drop
        it->second->onDBDrop(dropmask);

        unsigned long now = mTimer->getMilliseconds();

        LOG_INFO("DatabaseService: Operation took %f seconds", (float)(now-sttime) / 1000);

        // recalculate the status
        mLoadingStatus.currentCoarse++;

        mLoadingStatus.recalc();

        // call the progress listener if it is set
        if (mProgressListener) {
            (*mProgressListener)(mLoadingStatus);
        }
    }
}

//-------------------------- Factory implementation
std::string DatabaseServiceFactory::mName = "DatabaseService";

DatabaseServiceFactory::DatabaseServiceFactory() : ServiceFactory() {
};

const std::string& DatabaseServiceFactory::getName() {
    return mName;
}

const uint DatabaseServiceFactory::getMask() {
    return SERVICE_CORE;
}

const size_t DatabaseServiceFactory::getSID() {
    return DatabaseService::SID;
}

Service* DatabaseServiceFactory::createInstance(ServiceManager* manager) {
    return new DatabaseService(manager, mName);
}

}
