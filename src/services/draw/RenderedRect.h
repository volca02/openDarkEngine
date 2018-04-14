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

#ifndef __RENDEREDRECT_H
#define __RENDEREDRECT_H

#include "DrawCommon.h"
#include "DrawOperation.h"

#include <OgreVector3.h>

namespace Opde {
/** Rendered rectangle. A single on-screen rectangle with a specified colour. */
class RenderedRect : public DrawOperation {
public:
    RenderedRect(DrawService *owner, DrawOperation::ID id,
                 const TextureAtlasPtr &atlas);

    virtual ~RenderedRect();

    void visitDrawBuffer(DrawBuffer *db);

    virtual DrawSourceBasePtr getDrawSourceBase();

    /// sets a new colour of the rect
    void setColour(const Ogre::ColourValue &col);

    /// sets a new width of the rect.
    void setWidth(size_t width);

    /// sets a new height of the rect.
    void setHeight(size_t height);

    /// sets a new size of the rect (both width and height)
    void setSize(const PixelSize &size);

protected:
    /// override that updates the image and marks dirty
    void _rebuild();

    void _setNewDrawSource();

    DrawQuad mDrawQuad;

    bool mInClip;

    TextureAtlasPtr mAtlas;

    DrawSourcePtr mVertexColourDS;

private:
    PixelSize mPixelSize;
};

}; // namespace Opde

#endif
