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

#include "ServiceCommon.h"

#include "File.h"

#include "LightService.h"
#include "OpdeServiceManager.h"
#include "render/RenderService.h"
#include "worldrep/WRCell.h"
#include "worldrep/WRTypes.h"
#include "worldrep/LightsForCell.h"

#include "DarkLight.h"
#include "DarkBspNode.h"
#include "DarkSceneManager.h"

using namespace std;
using namespace Ogre;

namespace Opde {

/*----------------------------------------------------*/
/*------------------- LightTableEntry ----------------*/
/*----------------------------------------------------*/
LightTableEntry::LightTableEntry(const FilePtr &tag, bool rgb) {
    *tag >> pos >> rot;

    if (rgb) {
        *tag >> brightness;
    } else {
        *tag >> brightness.x;
        brightness.y = brightness.x;
        brightness.z = brightness.x;
    }

    *tag >> cone_inner >> cone_outer >> radius;
};


/*----------------------------------------------------*/
/*-------------------- LightService ------------------*/
/*----------------------------------------------------*/
template <> const size_t ServiceImpl<LightService>::SID = __SERVICE_ID_LIGHT;

LightService::LightService(ServiceManager *manager, const std::string &name)
    : ServiceImpl<Opde::LightService>(manager, name), mLightPixelSize(0) {

    mAtlasList.reset(new LightAtlasList());
}

//------------------------------------------------------
LightService::~LightService() {
    clear();
}

//------------------------------------------------------
void LightService::setLightPixelSize(size_t size) { mLightPixelSize = size; }

//------------------------------------------------------
void LightService::build() {
    atlasLightMaps();

    mAtlasList->render();
}

//------------------------------------------------------
void LightService::clear() {
    // TODO: mAtlasList->clear();

    // TODO: To be totally clean we would have to release all the created
    // lights! for now, this is done in WR - it calls clearScene
    mLights.clear();
    mCells = nullptr;
}

//------------------------------------------------------
bool LightService::init() {
    // nothing so far...
    mRenderService = GET_SERVICE(RenderService);
    mSceneMgr = mRenderService->getSceneManager();

    return true;
}

//------------------------------------------------------
void LightService::_loadTableFromTagFile(const FilePtr &tag) {
    // two counts - static lights, dynamic lights
    uint32_t nstatic, ndynamic;
    tag->read(&nstatic, sizeof(uint32_t));
    tag->read(&ndynamic, sizeof(uint32_t));

    mStaticLightCount = nstatic;
    mDynamicLightCount = ndynamic;

    // the arrays have to be empty, already
    assert(mLights.getSize() == 0);

    // now load, and create lights as we go
    mLights.grow(mStaticLightCount + mDynamicLightCount);

    for (size_t i = 0; i < mDynamicLightCount + mStaticLightCount; ++i) {
        LightTableEntry tentry(tag, mLightPixelSize != 1);

        // produce light according to the entry
        // the static lights do not traverse scene in order to calculate
        // visibility and are instead manually attached to BSP leaves. See the
        // end of this method
        mLights[i] = _produceLight(tentry, i, i >= mStaticLightCount);
    }

    // Now we have to iterate over the cells and attach lights to all affected
    for (auto &c : *mCells) {
        LightsForCell *lfc = c->getLights();

        // get the BSP leaf from SceneMgr
        BspNode *leaf = c->getBspNode();

        // for all lights in the cell
        for (size_t i = 0; i < lfc->light_count; ++i) {
            size_t lid = lfc->light_indices[i];

            if (lid < mStaticLightCount)
                mLights[lid]->affectsCell(leaf);
        }
    }
}

//------------------------------------------------------
DarkLight *LightService::getLightForID(int id) {
    // TODO: Code
    return NULL;
}

//------------------------------------------------------
const WRLightInfo &LightService::getLightInfo(size_t cellID, size_t faceID) {
    // find the LFC pointer for the cell
    return getLightsForCell(cellID)->getLightInfo(faceID);
}

//------------------------------------------------------
size_t LightService::getAtlasForCellPolygon(size_t cellID, size_t faceID) {
    //
    return getLightsForCell(cellID)->getAtlasForPolygon(faceID);
}

Ogre::TexturePtr LightService::getAtlasTexture(size_t idx) {
    return mAtlasList->getAtlas(idx)->getTexture();
}

//------------------------------------------------------------------------------
void LightService::atlasLightMaps() {
    // atlas all the cells
    for (const auto &cell : *mCells) {
        // atlas each
        cell->getLights()->atlasLightMaps(mAtlasList.get());
    }
}

//------------------------------------------------------------------------------
LightsForCell *LightService::getLightsForCell(size_t cellID) {
    if (!mCells) return nullptr;
    return (*mCells)[cellID]->getLights();
}

//------------------------------------------------------------------------------
DarkLight *LightService::_produceLight(const LightTableEntry &entry, size_t id,
                                       bool dynamic) {
    String lname =
        String("Light") + (dynamic ? 'D' : 'S') + StringConverter::toString(id);

    SceneNode *ln = mSceneMgr->createSceneNode(lname);
    DarkLight *l = static_cast<DarkLight *>(mSceneMgr->createLight(lname));

    // set the parameters
    ln->setPosition(entry.pos.x, entry.pos.y, entry.pos.z);
//    l->setPosition(entry.pos.x, entry.pos.y, entry.pos.z);
    if (entry.cone_inner < 0) {
        l->setType(Light::LT_POINT);
    } else {
        l->setType(Light::LT_SPOTLIGHT);
        l->setSpotlightInnerAngle(Radian(Math::ACos(entry.cone_inner)));
        l->setSpotlightOuterAngle(Radian(Math::ACos(entry.cone_outer)));
        l->setDirection(entry.rot.x, entry.rot.y, entry.rot.z);
    }

    if (entry.radius > 0) {
        l->setAttenuation(
            entry.radius, 1.0, 0.0,
            0.0); // lights with radius - constant brightness over radius
    } else {
        Vector3 color(entry.brightness.x, entry.brightness.y,
                      entry.brightness.z);

        // The range * 10 is just a test, but seems to be working quite nice
        l->setAttenuation(color.length() * 10, 0.0, 1.0,
                          0.0); // linear falloff for radius-less lights
    }

    l->setDiffuseColour(entry.brightness.x, entry.brightness.y,
                        entry.brightness.z);
    l->setSpecularColour(entry.brightness.x, entry.brightness.y,
                         entry.brightness.z);

    l->setIsDynamic(dynamic);

    ln->attachObject(l);
    // static_cast<DarkSceneManager *>(mSceneMgr)->queueLightForUpdate(l);

    return l;
}

//-------------------------- Factory implementation
const std::string LightServiceFactory::mName = "LightService";

LightServiceFactory::LightServiceFactory() : ServiceFactory() {}

const std::string &LightServiceFactory::getName() { return mName; }

const uint LightServiceFactory::getMask() { return SERVICE_RENDERER; }

const size_t LightServiceFactory::getSID() { return LightService::SID; }

Service *LightServiceFactory::createInstance(ServiceManager *manager) {
    return new LightService(manager, mName);
}

} // namespace Opde
