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

/**
 @file LightService.h
 @brief A light service implementation. Anything related to lighting is resolved
 here.
 */

#ifndef __LIGHTSERVICE_H
#define __LIGHTSERVICE_H

#include "Array.h"
#include "Vector3.h"

#include "ServiceCommon.h"
#include "OpdeService.h"
#include "OpdeServiceFactory.h"
#include "config.h"

#include <OgreVector2.h>

namespace Ogre {
class DarkLight;
} // namespace Ogre

using Ogre::DarkLight;

namespace Opde {

class LightAtlasList;
class LightService;
class WRLightInfo;
class WRPolygonTexturing;
class LightMap;
class WRCell;

struct LightTableEntry {
    // Ctor that reads the data from WR tag
    LightTableEntry(const FilePtr &tag, bool rgb);

    Vector3 pos;        // 12
    Vector3 rot;        // 12 - 24
    Vector3 brightness; // 4 - 28
    float cone_inner;   // 4 - 32 - TODO: This is cos(alpha), not radians!
    float cone_outer;   // 4 - 36
    float radius;       // 4 - 40
};

class LightsForCell;

/// shared_ptr to LightsForCell
typedef shared_ptr<LightsForCell> LightsForCellPtr;

/** @brief Light Service - service which handles lights - Brush lights, static
 * lights, anim lights and dynamic lights. Lights are compiled into a list when
 * the portalization is done, and then obtain a list index (id). This is
 * referenced by animlights and probably dynamic lights as well, when there is a
 * request to change the brightness/position. Static lights don't respond to
 * Light/Spotlight property changes, as those properties (although exposed by
 * this service) are Dromed side only. */
class LightService : public ServiceImpl<LightService> {
public:
    friend class WorldRepService;

    /** Constructor
     * @param manager The ServiceManager that created this service
     * @param name The name this service should have (For Debugging/Logging)
     */
    LightService(ServiceManager *manager, const std::string &name);

    /// Destructor
    virtual ~LightService();

    /// Sets the size, in bytes, of the pixel size of the lightmaps
    void setLightPixelSize(size_t size);

    /** Loads the light definitions for the given cell from the current position
     * of the tag file
     */
    LightsForCellPtr _loadLightDefinitionsForCell(
        size_t cellID, const FilePtr &tag, size_t num_anim_lights,
        size_t num_textured, const std::vector<WRPolygonTexturing> &face_infos);

    /** loads the light definition table from the specified file.
     * @note this method is internal, used by WorldRepService when loading the
     * WR tag file */
    void _loadTableFromTagFile(const FilePtr &tag);

    /** @return the light for the given light id (not object id). Used when
     * connecting the light with the cells it affects (see WorldRepService) */
    DarkLight *getLightForID(int id);

    /** Prepares the light service after loading - atlasses the lightmaps,
     * creates lights and attaches them to the BSP leaves */
    void build();

    /** removes all the stored lights, cleans the atlases, etc. */
    void clear();

    /** Lightmap info getter */
    const WRLightInfo &getLightInfo(size_t cellID, size_t faceID);

    /// Returns the atlas index for cell id and it's polygon id
    size_t getAtlasForCellPolygon(size_t cellID, size_t faceID);

    /// Returns atlas texture for the given atlas index
    Ogre::TexturePtr getAtlasTexture(size_t idx);

protected:
    /// Service initialization
    bool init();

    /// puts all the read light maps into atlases
    void atlasLightMaps();

    void _setCells(std::vector<std::unique_ptr<WRCell>> *cells)
    {
        mCells = cells;
    }

    LightsForCell *getLightsForCell(size_t cellID);

    /// produces a light (by creating it using the scene manager)
    DarkLight *_produceLight(const LightTableEntry &entry, size_t id,
                             bool dynamic);

    /// Lists of all light map atlases
    LightAtlasList *mAtlasList;

    // Version of the lightmap - either 1 or 2 - directly means the stored
    // lightmap pixel size as well
    size_t mLightPixelSize;

    /// count of the static lights, as read from the tag
    size_t mStaticLightCount;

    /// count of the dynamic lights, as read from the tag
    size_t mDynamicLightCount;

    // borrowed cells from worldrep service
    std::vector<std::unique_ptr<WRCell>> *mCells = nullptr;

    // and the tables of the lights (indexed by the light id)

    /// table of lights.
    SimpleArray<DarkLight *> mLights;

    /// ref to the render service
    RenderServicePtr mRenderService;

    /// scene manager ref (for light management). DarkSceneManager expected
    Ogre::SceneManager *mSceneMgr;
};

/// Shared pointer to light service
typedef shared_ptr<LightService> LightServicePtr;

/// Factory for the LightService objects
class LightServiceFactory : public ServiceFactory {
public:
    LightServiceFactory();
    ~LightServiceFactory(){};

    /** Creates a LightService instance */
    Service *createInstance(ServiceManager *manager);

    const std::string &getName() override;
    const uint getMask() override;
    const size_t getSID() override;

private:
    static const std::string mName;
};
} // namespace Opde

#endif
