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
 *
 *	  $Id$
 *
 *****************************************************************************/

#ifndef LATLAS_H
#define LATLAS_H

#include <map>
#include <vector>

#include "ConsoleCommandListener.h"
#include "FreeSpaceInfo.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreImage.h"
#include "OgreTextureManager.h"
#include "OgreVector2.h"
#include "OgreVector3.h"
#include "integers.h"

namespace Opde {

/// A structure holding info for one texture in an atlas
typedef struct {
    int atlasnum;
    float u, v;   // shift of the texture (UV) in the atlas (position)
    float su, sv; // size of the texture (UV) in the atlas (relative to atlas
                  // size - 0-1)
} AtlasInfo;

/** A structure representing a lightmap pixel */
typedef struct lmpixel {
    unsigned char R, G, B;

    /** Converts the RGB values to the A8R8G8B8 format
     * \return uint32 holding the A8R8G8B8 version of the stored RGB */
    Ogre::uint32 ARGB() { return (R << 16) | (G << 8) | B; }

    lmpixel(unsigned char nR, unsigned char nG, unsigned char nB) {
        R = nR;
        G = nG;
        B = nB;
    }

    lmpixel() {
        R = 0;
        G = 0;
        B = 0;
    }
} lmpixel;

// Forward declaration
class LightAtlas;

/** A class representing a switchable lightmap. It holds one static lightmap,
 * which can't be switched, and a set of lightmaps indexed by light number,
 * which can have their'e intensity modulated. The resulting lightmap is
 * recalculated every time an actual chage happens. Please use
 * Opde::LightAtlasList::setLightIntensity if you want to set an intensity to a
 * certain light. Calling the method here would not refresh the lightmap
 * texture.
 * @see LightAtlasList */
class LightMap {
    // Friend of atlas, so the atlas is able to set uv && suv for us
    friend class LightAtlas;

    /** Information about the lightmap position in the atlas */
    FreeSpaceInfo *mPosition;

    /** static lightmap */
    lmpixel *mStaticLmap;

    typedef std::map<int, lmpixel *> ObjectToLightMap;

    /** A map of the switchable lightmaps */
    ObjectToLightMap mSwitchableLmaps;

    typedef std::map<int, float> LightIntensityMap;

    /** A map of the actual intensities */
    LightIntensityMap mIntensities;

    /** Lightmap's size in pixels */
    unsigned int mSizeX, mSizeY;

    /** Atlas holding the lightmap */
    LightAtlas *mOwner;

    /** Repositioning from 0-1 to the Atlas coords */
    Ogre::Vector2 mUV;

    /** Size of the lmap in the atlas*/
    Ogre::Vector2 mSizeUV;

    /** Lightmap's tag value */
    int mTag;

public:
    /** Constructor - takes the targetting freespaceinfo, size of the lightmap
     * and initializes our buffer with the static lightmap. This class will
     * manage the unallocation of the lighymap as needed, just pass the pointer,
     * it will deallocate the lmap in the destructor
     * @note The static_lightmap will be delete[]'d in destructor, together with
     * any switchable lightmaps present */
    LightMap(unsigned int sx, unsigned int sy, lmpixel *static_lightmap,
             int tag = 0)
        : mSizeX(sx), mSizeY(sy), mTag(tag) {
        mStaticLmap = static_lightmap;

        mPosition = NULL;
    }

    /** Destructor. Frees all allocated lightmaps */
    ~LightMap() {
        delete[] mStaticLmap;

        ObjectToLightMap::iterator it = mSwitchableLmaps.begin();

        for (; it != mSwitchableLmaps.end(); ++it)
            delete[] it->second;

        mSwitchableLmaps.clear();
    }

    /** Set the targetting placement of the lightmap in the atlas _owner */
    void setPlacement(LightAtlas *_owner, FreeSpaceInfo *tgt);

    /** Gets an owner atlas of this lightmap
     * \return LightAtlas containing this light map */
    const LightAtlas *getOwner() { return mOwner; }

    /** Converts input UV coords to the Atlassed coords (after UV
     * transformation) */
    Ogre::Vector2 toAtlasCoords(Ogre::Vector2 in_uv) {
        in_uv = in_uv * mSizeUV; // dunno why, this one does not work written
                                 // in_uv *= mSizeUV;
        in_uv += mUV;

        return in_uv;
    }

    /** Helping static method. Prepares an RGB version of the given buffer
     * containing v1 or v2 lightmap
     * @note Returns NEW buffer that the caller is responsible to manage!
     * @todo If somebody finds time, do this two stage - one return required
     * size, two convert into supplied buffer */
    static lmpixel *convert(char *data, int sx, int sy, int ver);

    /** Adds a switchable lightmap with identification id to the lightmap list
     * (has to be of the same size).
     * @param id the ID of the light the lightmaps belongs to.
     * @param data are the actual values converted to RGB. Unallocation is
     * handled in destructor */
    void AddSwitchableLightmap(int id, lmpixel *data);

    /** The main intensity setting function
     * @param id The id of the light (not object id, but internal light id)
     * @param intensity the new intensity of the light (0.0f-1.0f)
     */
    void setLightIntensity(int id, float intensity);

    /** Refreshes the texture's pixel buffer with the final version of all
     * lightmaps */
    void refresh();

    /// @return the dimensions of this atlas in pixels
    std::pair<int, int> getDimensions() const;

    /** @return the atlas index */
    int getAtlasIndex();

    /** Returns a freely specified lightmap's tag.
    * Tags are used to identify the texture which the lightmap modulates.
    * The atlas list tries to minimize the number of atlases containing the
    lightmaps of the same tag. / That minimizes the batch count of the
    rendering, thus improving performance.
    */
    int getTag() { return mTag; };
};

/** Class holding one set of light maps. Uses a HardwarePixelBufferSharedPtr
 * class for texture storage. This class is used for atlasing a light map set
 * into one bigger lightmap texture containing the light maps. This accelerates
 * rendering. This texture atlas implementation is targetted at lightmaps. Uses
 * tag set to tell if the atlas already contains the specified tag of lightmap
 */
class OPDELIB_EXPORT LightAtlas {
private:
    /** The count of the stored light maps */
    int mCount;

    /** The maximum atlas size */
    static int mMaxSize;

    /** Global index of this atlas in the atlas list */
    int mIdx;

    /** Texture pointer for this atlas */
    Ogre::TexturePtr mTex;

    /** The resulting pixel buffer */
    Ogre::HardwarePixelBufferSharedPtr mAtlas;

    /** The name of the resulting resource */
    Ogre::String mName;

    /** A vector containing Free Space rectangles left in the Atlas */
    FreeSpaceInfo *mFreeSpace;

    typedef std::vector<LightMap *> LightMapVector;

    /** Already inserted lightmaps - held for dealocation and building */
    LightMapVector mLightmaps;

    typedef std::map<int, std::set<LightMap *>> LightIDMap;

    /** Light ID number to the LightMap set mapping - changing light intensity
     * will iterate throught the set of lmaps, and regenerate */
    LightIDMap mLights;

    /// set of tags this atlas contains (used to minimize the texture*atlas
    /// combinations)
    typedef std::set<int> TagSet;

    /** Tag values of this atlas */
    TagSet mTagSet;

    /** The dimension of the atlas (starts at 16x16 - 16 here). As the atlas is
     * rectangular we only need one */
    int mSize;

protected:
    /// grows the atlas to the newly specified dimensions
    void growAtlas(int newSize);

    /// places the lightmap without any refreshes
    bool placeLightMap(LightMap *lmap);

public:
    /** constructor
     * @param idx The atlas index
     * @param tag The initial tag this atlas will hold
     */
    LightAtlas(int idx, int tag = 0);

    /// Destructor. Frees all lightmaps
    ~LightAtlas();

    /// Builds the texture. Called once per life of the atlas, writes the
    /// initial version of the atlas texture
    void build();

    /** Adds a light map to this atlas.
     * @param lmap Lightmap to be added
     */
    bool addLightMap(LightMap *lmap);

    /** renders the prepared light map buffers into a texture - copies the pixel
     * data to the 'atlas' */
    bool render();

    /** Updates the pixel buffer with a new version of the lightmap (for example
     * after light intensity change). This version converts the uint32 (shifted
     * by 8) to 0-255 range (checks limits)
     * \warning Must be called after atlas locking, otherwise the program will
     * crash ! */
    inline void updateLightMapBuffer(FreeSpaceInfo &fsi, uint32_t *lR,
                                     uint32_t *lG, uint32_t *lB);

    /** Register that animated light ID maps to the LightMap instance */
    void registerAnimLight(int id, LightMap *target);

    /** Sets the intensity of the light
     * @see LightAtlasList::setLightIntensity */
    void setLightIntensity(int id, float intensity);

    /** Returns the Light Map Atlas order number */
    int getIndex();

    /** Returns the pixel count of unmapped area of this atlas */
    int getUnusedArea();

    /** Returns the area in pixels that is used by lmaps */
    int getUsedArea();

    /** Returns the size of the atlas in pixels */
    int getPixelCount();

    /** returns the tag number of this atlas */
    int hasTag(int tag) {
        TagSet::iterator it = mTagSet.find(tag);
        return it != mTagSet.end();
    };

    /** adds a new tag into the atlas */
    void addTag(int tag) { mTagSet.insert(tag); };

    /// returns a comma separated string of all tags in the set
    std::string getTagStr();

    /** Sets the maximum atlas size */
    static void setMaxSize(int Size) { mMaxSize = Size; };

    Ogre::TexturePtr getTexture() { return mTex; }
};

/** @brief A holder of a number of the light map atlases.
 * The main class in the family of lightmap management. Responsible for all
 * lightmap atlases.
 * Use this class to work with the light map storage and light switching. */
class LightAtlasList : public ConsoleCommandListener {
public:
    /// helper operator that compares on the pixel count of lightmaps. Used to
    /// sort the lightmaps by size (helps atlassing effectivity)
    struct lightMapLess {
        bool operator()(const LightMap *a, const LightMap *b) const;
    };

private:
    typedef std::vector<LightAtlas *> LightAtlasVector;

    /** A list of the light map atlases */
    LightAtlasVector mList;

    /** Pre-render lightmap list*/
    typedef std::map<int, std::vector<LightMap *>> TextureLightMapQueue;

    /// List of lightmaps that are waiting for processing
    TextureLightMapQueue mLightMapQueue;

protected:
    bool placeLightMap(LightMap *lmap);

public:
    LightAtlasList();

    /** A destructor, unallocates all the previously allocated lightmaps */
    virtual ~LightAtlasList();

    /** Adds a light map holding place and a static lightmap.
     * @note The U/V mapping of the lightmap is not valid until the atlas list
     * is rendered */
    LightMap *addLightmap(int ver, int texture, char *buf, int w, int h,
                          AtlasInfo &destinfo);

    /** get's the current count of the atlases stored.
     * \return int Count of the atlases */
    int getCount();

    /** Prepares the Light Atlases to be used as textures.
     * atlases all the lightmaps, in Texture, size orders (texture being major
     * ordering rule) Then calls the LightAtlas.render method
     * @see Ogre::LightAtlas.render()
     */
    bool render();

    /// Light intensity setter - refreshes the lightmaps containing the light
    /// with the new intensity
    void setLightIntensity(int id, float value) {
        std::vector<LightAtlas *>::iterator atlases = mList.begin();

        for (; atlases != mList.end(); ++atlases) {
            (*atlases)->setLightIntensity(id, value);
        }
    };

    LightAtlas *getAtlas(int idx) { return mList.at(idx); }

    /// console command listener
    virtual void commandExecuted(std::string command, std::string parameters);
};

} // namespace Opde
#endif
