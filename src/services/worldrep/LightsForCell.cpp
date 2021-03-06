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
 *****************************************************************************/

#include "LightsForCell.h"
#include "File.h"

namespace Opde {

LightsForCell::LightsForCell(const FilePtr &file, size_t num_anim_lights,
                             size_t num_textured, size_t light_size,
                             const std::vector<WRPolygonTexturing> &face_infos)
    : mLightSize(light_size),
      mNumTextured(num_textured),
      mNumAnimLights(num_anim_lights),
      mFaceInfos(face_infos),
      mAtlased(false)
{
    // TODO(volca): Rework these as vectors, read in a loop (endianity, etc)
    // load the light id's array
    anim_map = new int16_t[mNumAnimLights];
    file->read(anim_map, sizeof(int16_t) * mNumAnimLights);

    // 8. load the lightmap descriptors
    lm_infos = new WRLightInfo[mNumTextured];
    file->read(lm_infos, sizeof(WRLightInfo) * mNumTextured);

    // 9. load the lightmaps
    // alloc the space for all the pointers
    lmaps = new uint8_t **[mNumTextured];
    lmcounts = new uint8_t[mNumTextured];

    size_t i;
    for (i = 0; i < mNumTextured; i++) {
        // Calculate the number of lmaps
        int lmcount = countBits(lm_infos[i].animflags) + 1;

        lmcounts[i] = lmcount;
        // 10. allocate space for all the lmap pointers (1+Anim) for actual face
        lmaps[i] = new uint8_t *[lmcount];

        int lmsize = lm_infos[i].lx * lm_infos[i].ly * mLightSize;

        // for each lmap, put the ptr to data
        int lmap;

        // this stores the pointer to the anim lmaps too (all of them)
        for (lmap = 0; lmap < lmcount; lmap++) {
            // 11. Read one lightmap
            lmaps[i][lmap] = new uint8_t[lmsize];
            file->read(lmaps[i][lmap], lmsize);
        }
    }

    // list of object lights (anim+static) affecting this cell
    // _uint32 unk;
    // chunk->read(&unk, sizeof(_uint32));

    file->read(&light_count, sizeof(uint32_t));

    // 12. Light object indexes
    light_indices = new uint16_t[light_count];
    file->read(&(light_indices[0]), sizeof(uint16_t) * light_count);
}

//------------------------------------------------------
LightsForCell::~LightsForCell() {
    delete[] anim_map;

    delete[] lm_infos;

    for (size_t i = 0; i < mNumTextured; i++) {

        for (size_t l = 0; l < lmcounts[i]; l++)
            delete[] lmaps[i][l];

        delete[] lmaps[i];
    }

    delete[] lmaps;
    delete[] lmcounts;

    delete[] light_indices;

    mLightMaps.clear();
}

//------------------------------------------------------
void LightsForCell::atlasLightMaps(LightAtlasList *atlas) {
    assert(!mAtlased);

    int ver = mLightSize - 1;

    // Array of lmap references
    mLightMaps.resize(mNumTextured);

    for (size_t face = 0; face < mNumTextured; ++face) {
        auto lmap = atlas->addLightmap(ver, mFaceInfos[face].txt,
                                       (char *)lmaps[face][0],
                                       lm_infos[face].lx, lm_infos[face].ly);

        // Let's iterate through the animated lmaps
        // we have anim_map (array of light id's), cell->header->anim_lights
        // contains the count of them.
        int bit_idx = 1, lmap_order = 1;

        for (size_t anim_l = 0; anim_l < mNumAnimLights; anim_l++) {
            if ((lm_infos[face].animflags & bit_idx) > 0) {
                // There is a anim lmap for this light and face
                auto converted = LightMap::convert(
                    (char *)lmaps[face][lmap_order], lm_infos[face].lx,
                    lm_infos[face].ly, ver);

                lmap->addSwitchableLightmap(anim_map[anim_l],
                                            std::move(converted));

                lmap_order++;
            }

            bit_idx <<= 1;
        }

        // the lmap is fully populated, set it into the vector
        mLightMaps[face] = std::move(lmap);
    } // for each face

    mAtlased = true;
}

//------------------------------------------------------
const WRLightInfo &LightsForCell::getLightInfo(size_t face_id) {
    assert(face_id < mNumTextured);
    return lm_infos[face_id];
}

//------------------------------------------------------
size_t LightsForCell::getAtlasForPolygon(size_t face_id) {
    assert(mAtlased);

    assert(face_id < mNumTextured);

    return mLightMaps[face_id]->getAtlasIndex();
}

//------------------------------------------------------------------------------
int LightsForCell::countBits(uint32_t src) {
    // Contributed by TNH (Telliamed):
    // Found this trick in some code by Sean Barrett [TNH]
    int count = src;
    count = (count & 0x55555555) + ((count >> 1) & 0x55555555); // max 2
    count = (count & 0x33333333) + ((count >> 2) & 0x33333333); // max 4
    count = (count + (count >> 4)) & 0x0f0f0f0f; // max 8 per 4, now 8 bits
    count = (count + (count >> 8));              // max 16 per 8 bits
    count = (count + (count >> 16));             // max 32 per 8 bits
    return count & 0xff;
}

//------------------------------------------------------------------------------
Ogre::Vector2 LightsForCell::mapUV(size_t face_id,
                                   const Ogre::Vector2 &original)
{
    assert(face_id < mNumTextured);

    return mLightMaps[face_id]->toAtlasCoords(original);
}

} // namespace Opde
