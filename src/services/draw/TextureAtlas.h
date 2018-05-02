/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2009 openDarkEngine team
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

#ifndef __TEXTUREATLAS_H
#define __TEXTUREATLAS_H

#include "DrawCommon.h"

namespace Opde {
// Forward decl.
class DrawService;
class FontDrawSource;
class FreeSpaceInfo;

/** Texture atlas for DrawSource grouping. Textures created within this atlas
 * are grouped together into a single rendering call when used as a source for
 * draw operations, resulting in better performance.
 */
class TextureAtlas : public DrawSourceBase {
public:
    /** Constructor. Creates a new atlas for texture storage */
    TextureAtlas(DrawService *owner, DrawSource::ID id);

    /** destructor */
    ~TextureAtlas();

    /** Creates an atlased draw source */
    DrawSourcePtr createDrawSource(const Ogre::String &imgName,
                                   const Ogre::String &groupName);

    /** Adds a font instance (to be filled with glyphs afterwards) to this atlas
     * @note You probably don't want to use this. You'll want to use
     * DrawService::loadFont instead */
    void _addFont(FontDrawSource *fdsp);

    /** Removes the specified font from the atlas */
    void _removeFont(FontDrawSource *fdsp);

    /** returns this Atlase's source ID */
    inline DrawSource::ID getAtlasID() const { return mAtlasID; };

    /** Internal tool to allow external addition of draw sources. Used by font
     * code. */
    void _addDrawSource(const DrawSourcePtr &ds);

    /** Internal tool to allow removal of draw sources */
    void _removeDrawSource(const DrawSourcePtr &ds);

    /** Builds the atlas. Locks it for further additions, makes it useable */
    void build();

    /// Owner getter
    inline DrawService *getOwner() const { return mOwner; };

    /// Returns a draw source for vertex colour rendering (2x2 white pixels)
    const DrawSourcePtr &getVertexColourDrawSource() const {
        return mVertexColour;
    };

protected:
    void enlarge(size_t area);

    void markDirty();

    void prepareResources();

    void dropResources();

    DrawService *mOwner;

    DrawSource::ID mAtlasID;

    typedef std::list<DrawSourcePtr> DrawSourceList;
    typedef std::list<FontDrawSource *> FontSet;

    DrawSourceList mMyDrawSources;
    FontSet mMyFonts;

    std::unique_ptr<FreeSpaceInfo> mAtlasAllocation;

    bool mIsDirty; // TODO: Replace by mIsBuilt

    PixelSize mAtlasSize;

    Ogre::String mAtlasName; // atlas texture name

    /// Used with vertex colour (texture less) rendering
    DrawSourcePtr mVertexColour;
};
}; // namespace Opde

#endif
