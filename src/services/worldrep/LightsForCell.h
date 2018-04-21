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

#ifndef __LIGHTSFORCELL_H
#define __LIGHTSFORCELL_H

#include <vector>

#include "DarkCommon.h"
#include "integers.h"
#include "worldrep/WRTypes.h"

namespace Opde {

/*----------------------------------------------------*/
/*------------------ LightsForCell -------------------*/
/*----------------------------------------------------*/
/// class holding all the light info loaded for a single cell.
/// Constructed/destructed using LightService
class LightsForCell {
    friend class LightService;

public:
    /** Constructor
     * @param file The tag to load from
     * @param light_size The size of the pixel in bytes (1=>grayscale lmaps,
     * 2=>16 bit rgb lmaps)
     */
    LightsForCell(const FilePtr &file, size_t num_anim_lights,
                  size_t num_textured, size_t light_size,
                  const std::vector<WRPolygonTexturing> &face_infos);
    ~LightsForCell();

    void atlasLightMaps(LightAtlasList *atlas);

    const WRLightInfo &getLightInfo(size_t face_id);

    size_t getAtlasForPolygon(size_t face_id);

    int countBits(uint32_t src);

    /** Maps the given UV to the atlas UV */
    Ogre::Vector2 mapUV(size_t face_id, const Ogre::Vector2 &original);

protected:
    /** animated lights map (e.g. bit to object number mapping) - count is to be
     * found in the header */
    int16_t *anim_map; // index by bit num, and ya get the object number the
                       // animated lightmap belongs to

    WRLightInfo *lm_infos;

    // Lightmaps:
    // The count of lightmaps per face index
    uint8_t *lmcounts;
    uint8_t ***lmaps; // poly number, lmap number --> pointer to data (may be 2
                      // bytes per pixel)

    // objects that are in this leaf when loaded (we may skip this if we add it
    // some other way in system) (Maybe it's only a light list affecting our
    // cell)
    uint32_t light_count;
    uint16_t *light_indices; // the object index number

    /* Array referencing the lightmaps inserted into the atlas */
    LightMap **lightMaps;

    /** Size of the lightmap element */
    size_t mLightSize;

    /** Textured polygon count of the cell this structure is associated with */
    size_t mNumTextured;

    /** Count of anim lights in the cell */
    size_t mNumAnimLights;

    /** Borrowed reference to face infos, used to determine the texture of
     * polygon */
    const std::vector<WRPolygonTexturing> &mFaceInfos;

    bool mAtlased;
};

} // namespace Opde

#endif /* __LIGHTSFORCELL_H */
