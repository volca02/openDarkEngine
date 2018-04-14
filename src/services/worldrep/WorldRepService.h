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

#ifndef __WORLDREPSERVICE_H
#define __WORLDREPSERVICE_H

#include "config.h"

#include "Callback.h"
#include "DarkSceneManager.h"
#include "Ogre.h"
#include "OpdeService.h"
#include "OpdeServiceManager.h"
#include "WRCell.h"
#include "WRTypes.h"
#include "database/DatabaseService.h"
#include "integers.h"
#include "light/LightService.h"
#include "render/RenderService.h"

#include <OgreDefaultHardwareBufferManager.h>
#include <OgreHardwareBufferManager.h>
#include <OgreStaticFaceGroup.h>
#include <vector>

#include "FileGroup.h"
#include "SharedPtr.h"

// The name of the group that stores the built textures and materials
#define TEMPTEXTURE_RESOURCE_GROUP "WrTextures"

namespace Opde {

/** @brief WorldRep service - Level geometry loader.
 *
 * This service is responsible for the level geometry initialization.
 * @note Should handle world-geometry related methods later on. For example -
 * Light switching */
class OPDELIB_EXPORT WorldRepService : public ServiceImpl<WorldRepService>,
                                       public DatabaseListener {
public:
    /** Initializes the Service */
    WorldRepService(ServiceManager *manager, const std::string &name);

    /** Destructs the WorldRepService instance, and unallocates the data, if
     * any. */
    virtual ~WorldRepService();

    Ogre::SceneManager *getSceneManager();

protected:
    virtual bool init();
    virtual void bootstrapFinished();
    void shutdown();

    /** Database load callback
     * @see DatabaseListener::onDBLoad */
    void onDBLoad(const FileGroupPtr &db, uint32_t curmask);

    /** Database save callback
     * @see DatabaseListener::onDBSave */
    void onDBSave(const FileGroupPtr &db, uint32_t tgtmask);

    /** Database drop callback
     * @see DatabaseListener::onDBDrop */
    void onDBDrop(uint32_t dropmask);

    /** Internal method. Clears all the used data and scene */
    void clearData();

    /** Unloads the worldrep. Clears the scene */
    void unload();

    /** Internal method which loads the data from the found WR/WRRGB chunk, and
     * constructs level geometry for the SceneManager */
    void loadFromChunk(FilePtr &wrChunk, size_t lightSize);

    /** Sets sky box according to the SKYMODE chunk contents. Does not do NewSky
     */
    void setSkyBox(const FileGroupPtr &db);

    /** Internal method. Creates a BspNode instance tree, and supplies it to the
     * sceneManager */
    void createBSP(unsigned int BspRows, WRBSPNode *tree);

    // TODO: Move this away... RenderService->getRoot?
    Ogre::Root *mRoot;

    // TODO: Move this away... RenderService->getSceneManager
    Ogre::DarkSceneManager *mSceneMgr;

    /** Loaded structure of the cells */
    WRCell **mCells;

    uint32_t mExtraPlaneCount;
    Ogre::Plane *mExtraPlanes;

    /** Cell count from header */
    uint32_t mNumCells;

    /// -- The vertex and index buffers
    Ogre::VertexData *mVertexData;

    /// system-memory buffer
    Ogre::HardwareIndexBufferSharedPtr mIndexes;

    /// Face groups
    Ogre::StaticFaceGroup *mFaceGroups;

    /// Database service
    DatabaseServicePtr mDatabaseService;

    /// Render service
    RenderServicePtr mRenderService;

    /// Light service
    LightServicePtr mLightService;

    /// holder of the level geometry
    Ogre::DarkGeometry *mWorldGeometry;
};

/// Shared pointer to worldrep service
typedef shared_ptr<WorldRepService> WorldRepServicePtr;

/// Factory for the WorldRep service
class OPDELIB_EXPORT WorldRepServiceFactory : public ServiceFactory {
public:
    WorldRepServiceFactory();
    ~WorldRepServiceFactory(){};

    /** Creates a WorldRepService instance */
    Service *createInstance(ServiceManager *manager);

    const std::string &getName();

    virtual const uint getMask();

    virtual const size_t getSID();

private:
    static std::string mName;
};
} // namespace Opde

#endif
