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

#include "DarkCommon.h"
#include "OpdeService.h"
#include "OpdeServiceFactory.h"
#include "PrioritizedMessageSource.h"
#include "ServiceCommon.h"
#include "SharedPtr.h"
#include "DatabaseCommon.h"

namespace Ogre {
class Timer;
} // namespace Ogre

namespace Opde {

/** @brief Database service - service which handles dark database loading and
 * saving Typically, a service wanting to handle database events will register
 * as a listener, and handle the events given from this service. There are some
 * rules:
 * @li Mission loading will drop with DB_COMPLETE
 * @li Savegame loading will have DB_SAVEGAME as dbtarget (so the services
 * handling savegame related chunks will not process the mission's chunks)
 */
class DatabaseService : public ServiceImpl<DatabaseService> {
public:
    DatabaseService(ServiceManager *manager, const std::string &name);
    virtual ~DatabaseService();

    /// High-level load. Loads a game database including parent databases as
    /// needed, dropping previous loaded data
    void load(const std::string &filename, uint32_t loadMask);

    /// Loads a game database without dropping any data
    void mergeLoad(const std::string &filename, uint32_t loadMask);

    /// Recursive load of all databases in hierarchy, without dropping
    void recursiveMergeLoad(const std::string &filename, uint32_t loadMask);

    /// Saves a game database, writing all data fitting the specified mask
    void save(const std::string &filename, uint32_t loadMask);

    /// Unload the game data. Release all the data fitting the mask specified
    void unload(uint32_t dropMask);

    /// Listener that receives events every now and then while loading
    typedef Callback<DatabaseProgressMsg> ProgressListener;

    /// Progress Listener shared_ptr
    typedef shared_ptr<ProgressListener> ProgressListenerPtr;

    /// Setter for the progress listener.
    void setProgressListener(const ProgressListenerPtr &listener) {
        mProgressListener = listener;
    };

    /// Clears (unsets) the progress listener (disabling it)
    void unsetProgressListener() { mProgressListener.reset(); };

    /// A free to use fine step function that calls the Progress Listener to
    /// reflect the loading progress Use this especially in some long-to load
    /// services
    void fineStep(int count);

    /** Registers a listener.
     * @param listener A pointer to the listening class
     * @param priority desired loading priority (order) of the listener
     * @note The same pointer has to be supplied to the unregisterListener in
     * order to succeed with unregistration
     */
    void registerListener(DatabaseListener *listener, size_t priority);

    /** Unregisters a listener.
     * @param listener ID returned by the registerListener call
     * @note The pointer has to be the same as the one supplied to the
     * registerListener
     */
    void unregisterListener(DatabaseListener *listener);

protected:
    virtual bool init();

    /** Reads the File_Type tag from the specified database.
     * @param db The database to load the tag value from
     * @return the FILE_TYPE first 4 bytes as uint32_t, or 0 if something bad
     * happens (not found, tag too short)
     */
    uint32_t getFileType(const FileGroupPtr &db);

    /// Retrieve a readonly database file by it's name
    FileGroupPtr getDBFileNamed(const std::string &filename);

    /** Gets the database tag name containing the file name of the parent
     * database based on the FILE_TYPE value */
    const char *getParentDBTagName(uint32_t fileType);

    /** loads a file name from the specified tag */
    std::string loadFileNameFromTag(const FileGroupPtr &db,
                                    const char *tagname);

    /// Calls onDBLoad on all listeners obeying priorities
    void broadcastOnDBLoad(const FileGroupPtr &db, uint32_t curmask);

    /// Calls onDBLoad on all listeners obeying priorities
    void broadcastOnDBSave(const FileGroupPtr &db, uint32_t tgtmask);

    /// Calls onDBLoad on all listeners obeying priorities
    void broadcastOnDBDrop(uint32_t dropmask);

    FileGroupPtr mCurDB;

    /// Used to report to the Progress Listener
    DatabaseProgressMsg mLoadingStatus;

    ProgressListenerPtr mProgressListener;

    /// Map of db. load listeners
    typedef std::multimap<size_t, DatabaseListener *> Listeners;

    Listeners mListeners;

    Ogre::Timer *mTimer;
};

/// Factory for the DatabaseService objects
class DatabaseServiceFactory : public ServiceFactory {
public:
    DatabaseServiceFactory();
    ~DatabaseServiceFactory(){};

    /** Creates a DatabaseService instance */
    Service *createInstance(ServiceManager *manager);

    const std::string &getName() override;

    const uint getMask() override;
    
    const size_t getSID() override;

private:
    static std::string mName;
};
} // namespace Opde

#endif
