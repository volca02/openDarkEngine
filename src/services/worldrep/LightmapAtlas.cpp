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

#include "LightmapAtlas.h"
#include "ConsoleBackend.h"
#include "OgreStringConverter.h"
#include "OpdeException.h"
#include "WRCommon.h"
#include "compat.h"
#include "integers.h"
#include "material/MaterialService.h"

using namespace Ogre;

namespace Opde {

int LightAtlas::mMaxSize;

Vector3 operator*(float a, const LMPixel &b) {
    return Vector3(a * b.R, a * b.G, a * b.B);
}

// ---------------- LightAtlas Methods ----------------------
/*
        A single texture, containing many lightmaps. used in the texturelist
   construction
*/
LightAtlas::LightAtlas(int idx, int tag)
    : mCount(0), mIdx(idx), mTex(), mAtlas(), mFreeSpace(), mSize(1)
{
    mName = "@lightmap" +
            idx; // so we can find the atlas by number in the returned AtlasInfo

    mFreeSpace.reset(new FreeSpaceInfo(0, 0, mSize, mSize));

    mCount = 0;

    mTagSet.insert(tag);
}

LightAtlas::~LightAtlas() {
    mFreeSpace.reset();

    if (mTex) {
        TextureManager::getSingleton().remove(mName);
        mTex.reset();
    }

    // delete all the lmaps.
    mLightmaps.clear();
}

std::string LightAtlas::getTagStr() {
    TagSet::iterator it = mTagSet.begin();

    std::stringstream sstr;

    while (it != mTagSet.end()) {
        int cur = *it++;

        sstr << cur;

        if (it != mTagSet.end())
            sstr << ", ";
    }

    return sstr.str();
}

void LightAtlas::growAtlas(int newSize) {
    if (newSize <= mSize)
        return;

    mSize = newSize;

    // initialise the free space - initially whole lmap
    mFreeSpace.reset(new FreeSpaceInfo(0, 0, mSize, mSize));

    // place the lightmaps again
    for (auto &lm : mLightmaps) {
        // place the lmap
        // should not fail, since the lmaps fitted to prev atlas.
        if (!placeLightMap(lm))
            OPDE_EXCEPT("Could not fit after growth!");
    }
}

int LightAtlas::getUsedArea() {
    int area = 0;

    // place the lightmaps again
    for (auto &lm : mLightmaps) {
        std::pair<int, int> dim = lm->getDimensions();
        area += dim.first * dim.second;
    }

    return area;
}

bool LightAtlas::addLightMap(LightMap *lmap) {
    // Dynamic allocation of lmap atlas size. If we didn't fit, try throwing
    // all the mapping away, then remap all
    while (!placeLightMap(lmap)) {
        if (mSize >= mMaxSize) // have some sane maximum
            return false;

        growAtlas(2 * mSize);
    }

    // and insert into our list
    mLightmaps.push_back(lmap);
    addTag(lmap->getTag());
    mCount++;

    return true;
}

bool LightAtlas::placeLightMap(LightMap *lmap) {
    std::pair<int, int> dim = lmap->getDimensions();

    FreeSpaceInfo *area = mFreeSpace->allocate(dim.first, dim.second);

    if (area == NULL)
        return false;

    // calculate some important UV conversion data
    // +0.5? to display only the inner transition of lmap texture, the outer
    // goes to black color
    lmap->mUV.x = ((float)area->x) / (float)mSize;
    lmap->mUV.y = ((float)area->y) / (float)mSize;

    // size conversion to atlas coords
    lmap->mSizeUV.x = ((float)dim.first) / (float)mSize;
    lmap->mSizeUV.y = ((float)dim.second) / (float)mSize;

    lmap->setPlacement(this, area);

    // finally increment the count of stored lightmaps
    return true;
}

bool LightAtlas::render() {
    if (mLightmaps.size() == 0) // no lmaps, no action
        return true;

    // (re)create the texture
    if (mTex) {
        TextureManager::getSingleton().remove(mName);
        mTex.reset();
    }

    // We place the lightmaps into a separate resource group for easy unloading
    mTex = TextureManager::getSingleton().createManual(
        mName, MaterialService::TEMPTEXTURE_RESOURCE_GROUP, TEX_TYPE_2D, mSize,
        mSize, 0, PF_X8R8G8B8, TU_DYNAMIC_WRITE_ONLY);

    mAtlas = mTex->getBuffer(0, 0);

    // Erase the lmap atlas pixel buffer
    mAtlas->lock(HardwareBuffer::HBL_DISCARD);

    const PixelBox &pb = mAtlas->getCurrentLock();

    uint32 *data = static_cast<uint32 *>(pb.data);
    for (int y = 0; y < mSize; y++) {
        memset(data, 0, mSize * sizeof(uint32));
        data += pb.rowPitch;
    }

    std::vector<LightMap *>::const_iterator lmaps_it = mLightmaps.begin();

    for (; lmaps_it != mLightmaps.end(); ++lmaps_it) {
        assert((*lmaps_it)->mOwner == this);
        (*lmaps_it)->refresh();
    }

    mAtlas->unlock();

    return true;
}

inline void LightAtlas::updateLightMapBuffer(FreeSpaceInfo &fsi, uint32_t *lR,
                                             uint32_t *lG, uint32_t *lB) {
    const PixelBox &pb = mAtlas->getCurrentLock();

    uint32 *data = static_cast<uint32 *>(pb.data) + fsi.y * pb.rowPitch;
    for (int y = 0; y < fsi.h; y++) {
        uint32 w = (y * fsi.w);
        for (int x = 0; x < fsi.w; x++) {

            uint32 R, G, B;

            R = lR[x + w] >> 8;
            G = lG[x + w] >> 8;
            B = lB[x + w] >> 8;

            if (R > 255)
                R = 255;
            if (G > 255)
                G = 255;
            if (B > 255)
                B = 255;

            uint32 ARGB = (R << 16) | (G << 8) | B;

            // Write a A8R8G8B8 conversion of the lmpixel
            data[x + fsi.x] = ARGB;
        }
        data += pb.rowPitch;
    }
}

void LightAtlas::registerAnimLight(int id, LightMap *target) {
    // Try to find the set, if not make a new one...
    std::map<int, std::set<LightMap *>>::iterator light_it = mLights.find(id);

    if (light_it != mLights.end()) {
        light_it->second.insert(target);
    } else {
        std::set<LightMap *> new_set;
        new_set.insert(target);
        mLights.insert(std::make_pair(id, new_set));
    }
}

void LightAtlas::setLightIntensity(int id, float intensity) {
    if (mLightmaps.size() == 0)
        return;

    // iterate throught the map lights, and call the setLightIntensity on all
    // registered lightmaps
    std::map<int, std::set<LightMap *>>::iterator light_it = mLights.find(id);

    if (light_it != mLights.end()) {
        mAtlas->lock(HardwareBuffer::HBL_DISCARD);

        std::set<LightMap *>::const_iterator lmaps_it =
            light_it->second.begin();

        for (; lmaps_it != light_it->second.end(); lmaps_it++)
            (*lmaps_it)->setLightIntensity(id, intensity);

        mAtlas->unlock();
    }
}

int LightAtlas::getIndex() { return mIdx; }

int LightAtlas::getUnusedArea() { return mFreeSpace->getLeafArea(); }

int LightAtlas::getPixelCount() { return mSize * mSize; }

// ---------------- LightAtlasList Methods ----------------------
LightAtlasList::LightAtlasList() {
    Opde::ConsoleBackend::getSingleton().registerCommandListener(
        std::string("light"), dynamic_cast<ConsoleCommandListener *>(this));
    Opde::ConsoleBackend::getSingleton().setCommandHint(
        std::string("light"),
        "Switch a light intensity: light LIGHT_NUMBER INTENSITY(0-1)");
    Opde::ConsoleBackend::getSingleton().registerCommandListener(
        std::string("lmeff"), dynamic_cast<ConsoleCommandListener *>(this));
    Opde::ConsoleBackend::getSingleton().setCommandHint(
        std::string("lmeff"), "lightmap atlas efficiency calculator");
}

LightAtlasList::~LightAtlasList() {
    mAtlases.clear();
}

bool LightAtlasList::placeLightMap(LightMap *lmap) {
    if (mAtlases.empty())
        mAtlases.emplace_back(new LightAtlas(0));

    int last = mAtlases.size();

    // iterate through existing Atlases, and see if any of them accepts our
    // lightmap
    for (int i = 0; i < last; i++) {
        if (!mAtlases.at(i)->hasTag(lmap->getTag()))
            continue;

        if (mAtlases.at(i)->addLightMap(lmap))
            return false;
    }

    // pass two - without the tag
    for (int i = 0; i < last; i++) {
        if (mAtlases.at(i)->addLightMap(lmap)) // will addTag internally
            return false;
    }

    // add new atlas to list if none of them accepted the lmap
    std::unique_ptr<LightAtlas> la(new LightAtlas(last, lmap->getTag()));
    la->addLightMap(lmap);

    mAtlases.emplace_back(std::move(la));
    return true;
}

std::unique_ptr<LightMap> LightAtlasList::addLightmap(int ver, int texture,
                                                      char *buf, int w, int h)
{
    // convert the pixel representation
    std::unique_ptr<LMPixel[]> cdata{LightMap::convert(buf, w, h, ver)};
    // construct the lmap
    std::unique_ptr<LightMap> lmap{
        new LightMap(w, h, std::move(cdata), texture)};

    // Insert the lightmap to the queue for atlas rendering after all are done.
    mLightMapQueue.push_back(lmap.get());

    return std::move(lmap);
}

int LightAtlasList::getCount() { return mAtlases.size(); }

bool LightAtlasList::render() {
    // Step 1. Atlas the queue
    // for all materials
    std::sort(mLightMapQueue.begin(), mLightMapQueue.end(), lightMapLess());
    for (auto &lm : mLightMapQueue) {
        placeLightMap(lm);
    }
    mLightMapQueue.clear();

    // Step2. Render
    for (auto &atlas : mAtlases) {
        if (!atlas->render())
            OPDE_EXCEPT("Could not render the lightmaps!");
    }

    // iterate through existing Atlases, and see if any of them accepts our
    // lightmap
    int used_pixels = 0;
    int total_pixels = 0;

    int last = mAtlases.size();

    for (auto &atlas : mAtlases) {
        int totc = atlas->getPixelCount();
        int used = atlas->getUsedArea();
        int s = atlas->getEdgeSize();
        used_pixels += used;
        total_pixels += totc;

        LOG_VERBOSE("Light Map Atlas: Atlas {tags %s} %dx%d : %d of %d used (%f%%) "
                    "(%d of %d so far)",
                    atlas->getTagStr().c_str(), s, s, used, totc,
                    100.0f * used / totc, used_pixels, total_pixels);
    }

    float percentage = 0;

    if (total_pixels > 0)
        percentage = 100.0f * used_pixels / total_pixels;

    LOG_INFO("Light Map Atlas: Total used light map atlasses pixels: %d of %d "
             "(%f%%). %d atlases total used.",
             used_pixels, total_pixels, percentage, last);

    return true;
}

void LightAtlasList::commandExecuted(std::string command,
                                     std::string parameters) {
    if (command == "light") {
        // Split the parameters
        size_t space_pos = parameters.find(' ');

        if (space_pos != std::string::npos) {
            // First substring to command, second to params
            std::string s_light = parameters.substr(0, space_pos);
            std::string s_intensity = parameters.substr(
                space_pos + 1, parameters.length() - (space_pos + 1));

            // TODO: Use logger for this.
            Opde::ConsoleBackend::getSingleton().putMessage(
                std::string("Doing light change"));

            int light = StringConverter::parseInt(s_light);
            float intensity = StringConverter::parseReal(s_intensity);

            // apply
            setLightIntensity(light, intensity);
        }
    } else if (command == "lmeff") {
        // calculate the lightmap coverage efficiency - per atlas and global one
        // (average) LOG_ERROR("Lmeff not implemented yet");
        int total_pixels = 0;
        int unused_pixels = 0;

        int last = mAtlases.size();

        // iterate through existing Atlases, and see if any of them accepts our
        // lightmap
        for (auto &atlas : mAtlases) {
            unused_pixels += atlas->getUnusedArea();
            total_pixels  += atlas->getPixelCount();
        }

        float percentage = 0;

        if (total_pixels > 0)
            percentage = 100.0f * unused_pixels / total_pixels;

        LOG_INFO("Total unused light map atlasses pixels: %d of %d (%f%%). %d "
                 "atlases total used.",
                 unused_pixels, total_pixels, percentage, last);

    } else
        LOG_ERROR("Command %s not understood by LightAtlasList",
                  command.c_str());
}

bool LightAtlasList::lightMapLess::operator()(const LightMap *a,
                                              const LightMap *b) const {
    std::pair<int, int> s1, s2;

    s1 = a->getDimensions();
    s2 = b->getDimensions();

    int area1 = s1.first * s1.second;
    int area2 = s2.first * s2.second;

    return (area1 < area2);
}

// ------------------------------- Lightmap class
void LightMap::refresh() {
    // First we calculate the new version of the lmap. Then we post it to the
    // atlas

    unsigned int lightMapSize = mSizeX * mSizeY;
    std::vector<uint32> lmapR; lmapR.resize(lightMapSize);
    std::vector<uint32> lmapG; lmapG.resize(lightMapSize);
    std::vector<uint32> lmapB; lmapB.resize(lightMapSize);

    // Copy the static lmap...
    for (unsigned int i = 0; i < lightMapSize; i++) {
        lmapR[i] = mStaticLmap[i].R << 8;
        lmapG[i] = mStaticLmap[i].G << 8;
        lmapB[i] = mStaticLmap[i].B << 8;
    }

    for (auto &p : mSwitchableLmaps) {
        int light_id = p.first;
        auto &act_lmap = p.second;

        float intensity = 0;

        std::map<int, float>::iterator intens_it = mIntensities.find(light_id);

        // Just to be sure we get the intensity
        if (intens_it != mIntensities.end())
            intensity = intens_it->second;
        else
            mIntensities.insert(std::make_pair(light_id, 0.0f));

        uint32_t Intens = static_cast<uint32_t>(intensity * 256);
        if (Intens > 0) {
            for (unsigned int i = 0; i < lightMapSize; i++) {
                lmapR[i] += Intens * act_lmap[i].R;
                lmapG[i] += Intens * act_lmap[i].G;
                lmapB[i] += Intens * act_lmap[i].B;
            }
        }
    }

    mOwner->updateLightMapBuffer(*mPosition,
                                 lmapR.data(),
                                 lmapG.data(),
                                 lmapB.data());

    // Get rid of the helping arrays
}

std::unique_ptr<LMPixel[]> LightMap::convert(char *data, int sx, int sy,
                                             int ver)
{
    std::unique_ptr<LMPixel[]> result(new LMPixel[sx * sy]);

    // old version - grayscale
    if (ver == 0) {
        for (int i = 0; i < (sx * sy); i++) {
            result[i].R = data[i];
            result[i].G = data[i];
            result[i].B = data[i];
        }
    } else {
        uint16 *buf_ptr = (uint16 *)data;

        for (int i = 0; i < (sx * sy); i++) {
            unsigned int xBGR = buf_ptr[i];

            result[i].R = (xBGR & 0x0001f) << 3;
            result[i].G = ((xBGR >> 5) & 0x0001f) << 3;
            result[i].B = ((xBGR >> 10) & 0x0001f) << 3;
        }
    }

    return std::move(result);
}

void LightMap::addSwitchableLightmap(int id, std::unique_ptr<LMPixel[]> &&data)
{
    mSwitchableLmaps.emplace(id, std::move(data));
    mIntensities[id] = 1.0f;
}

void LightMap::setLightIntensity(int id, float intensity) {
    std::map<int, float>::iterator intens = mIntensities.find(id);

    if (intens != mIntensities.end()) {
        // only if the value changed
        if (intens->second != intensity) {
            intens->second = intensity;

            refresh();
        }
    }
}

int LightMap::getAtlasIndex() { return mOwner->getIndex(); }

void LightMap::setPlacement(LightAtlas *_owner, FreeSpaceInfo *tgt) {
    mOwner = _owner;
    mPosition = tgt;

    // register as a light related lightmap
    std::map<int, float>::iterator it = mIntensities.begin();

    for (; it != mIntensities.end(); it++) {
        mOwner->registerAnimLight(it->first, this);
    }
}

std::pair<int, int> LightMap::getDimensions() const {
    return std::pair<int, int>(mSizeX, mSizeY);
}
} // namespace Opde
