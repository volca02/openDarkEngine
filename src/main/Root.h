/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
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

#ifndef __ROOT_H
#define __ROOT_H

#include "config.h"

#include "compat.h"
#include "integers.h"

#include "OpdeCommon.h"

#include "ConsoleBackend.h"
#include "OgreOpdeLogConnector.h"
#include "OpdeServiceManager.h"
#include "OpdeSingleton.h"
#include "ServiceCommon.h"
#include "logger.h"

#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <Overlay/OgreOverlaySystem.h>

/** OpenDarkEngine namespace. Holds the whole openDarkEngine project. */
namespace Opde {

class Tracer;

/** OPDE core class. Used to initialize the whole engine. Singleton */
class Root : public Singleton<Root> {
public:
    /** Initializes the opde core
     * @param serviceMask the mask of the services which should be used (others
     * will be ignored and unreachable)
     * @param logFileName - optional log file name - when specified, logging to
     * file will be initialized automatically
     */
    Root(uint serviceMask = SERVICE_ALL, const char *logFileName = NULL);

    /** stops the opde core, does cleanup */
    ~Root();

    // ----------------- Methods used for boostrapping --------------------
    /// Loads a config file, given it's file name, which contains resource
    /// locations configuration
    void loadResourceConfig(const std::string &fileName);

    /// Loads a config file with opde settings
    void loadConfigFile(const std::string &fileName);

    /// Only a wrapper around the
    /// Ogre::ResourceGroupManager::addResourceLocation
    void addResourceLocation(const std::string &name,
                             const std::string &typeName,
                             const std::string &secName,
                             bool recursive = false);

    /// Only a wrapper around the
    /// Ogre::ResourceGroupManager::removeResourceLocation
    void removeResourceLocation(const std::string &name,
                                const std::string &secName);

    /** To be called when bootstrapping process was finished */
    void bootstrapFinished();

    /** @returns a pointer to the logger used */
    Logger *getLogger() { return mLogger.get(); };

    /** @returns a pointer to the service manager */
    ServiceManager *getServiceManager() { return mServiceMgr.get(); };

    /** Creates a new logger instance that logs to a file (Logger will be
     * automatically destroyed on termination) */
    void logToFile(const std::string &fname);

    /** A shortcut to set loglevel. Valid values are 0-4 */
    void setLogLevel(int level);

protected:
    /// Registers all the service factories to the Service Manger
    void registerServiceFactories();

    /// Creates all the loop modes
    void setupLoopModes();

    std::unique_ptr<Logger> mLogger;
    std::unique_ptr<Tracer> mTracer;
    std::unique_ptr<ServiceManager> mServiceMgr;
    std::unique_ptr<Ogre::Root> mOgreRoot;
    std::unique_ptr<Ogre::LogManager> mOgreLogManager;
    std::unique_ptr<OgreOpdeLogConnector> mOgreOpdeLogConnector;

    /// @deprecated
    std::unique_ptr<ConsoleBackend> mConsoleBackend;

    typedef std::vector<std::unique_ptr<LogListener>> LogListenerList;

    LogListenerList mLogListeners;

    const unsigned int mServiceMask;

    /// Factory for case-less filesystem archives
    std::unique_ptr<Ogre::ArchiveFactory> mDirArchiveFactory;

    /// Factory for Zip archives (Quickfix for ogre 1.9 bug)
    std::unique_ptr<Ogre::ArchiveFactory> mZipArchiveFactory;

    /// Factory for Crf archives (zip archives with archivename prefix)
    std::unique_ptr<Ogre::ArchiveFactory> mCrfArchiveFactory;

    // If ogre is not used, these point to particular managers we hijack
    std::unique_ptr<Ogre::ResourceGroupManager> mResourceGroupManager;
    std::unique_ptr<Ogre::ArchiveManager> mArchiveManager;
};

} // namespace Opde

#endif
