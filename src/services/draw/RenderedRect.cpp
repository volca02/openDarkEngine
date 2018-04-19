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

#include "RenderedRect.h"
#include "DrawBuffer.h"
#include "DrawCommon.h"
#include "DrawService.h"
#include "DrawSheet.h"
#include "TextureAtlas.h"

using namespace Ogre;

namespace Opde {

/*----------------------------------------------------*/
/*-------------------- RenderedRect ------------------*/
/*----------------------------------------------------*/
RenderedRect::RenderedRect(DrawService *owner, DrawOperation::ID id,
                           const TextureAtlasPtr &atlas)
    : DrawOperation(owner, id), mAtlas(atlas) {

    mVertexColourDS = atlas->getVertexColourDrawSource();

    mDrawQuad.texCoords.left = mVertexColourDS->transformX(0);
    mDrawQuad.texCoords.right = mVertexColourDS->transformX(1.0f);
    mDrawQuad.texCoords.top = mVertexColourDS->transformY(0);
    mDrawQuad.texCoords.bottom = mVertexColourDS->transformY(1.0f);

    mDrawQuad.color = ColourValue(1.0f, 1.0f, 1.0f);

    mInClip = true;

    mPixelSize.width = 0;
    mPixelSize.height = 0;

    // to be sure we are up-to-date
    _markDirty();
}

//------------------------------------------------------
RenderedRect::~RenderedRect() { mVertexColourDS.reset(); }

//------------------------------------------------------
void RenderedRect::visitDrawBuffer(DrawBuffer *db) {
    // are we in the clip area?
    if (mInClip)
        db->_queueDrawQuad(&mDrawQuad);
}

//------------------------------------------------------
DrawSourceBasePtr RenderedRect::getDrawSourceBase() {
    return static_pointer_cast<DrawSourceBase>(mVertexColourDS);
}

//------------------------------------------------------
void RenderedRect::setColour(const Ogre::ColourValue &col) {
    mDrawQuad.color = col;
    _markDirty();
}

//------------------------------------------------------
void RenderedRect::setWidth(size_t width) {
    mPixelSize.width = width;
    _markDirty();
}

//------------------------------------------------------
void RenderedRect::setHeight(size_t height) {
    mPixelSize.height = height;
    _markDirty();
}

//------------------------------------------------------
void RenderedRect::setSize(const PixelSize &size) {
    mPixelSize = size;
    _markDirty();
}

//------------------------------------------------------
void RenderedRect::_rebuild() {
    mDrawQuad.texCoords.left = mVertexColourDS->transformX(0);
    mDrawQuad.texCoords.right = mVertexColourDS->transformX(1.0f);
    mDrawQuad.texCoords.top = mVertexColourDS->transformY(0);
    mDrawQuad.texCoords.bottom = mVertexColourDS->transformY(1.0f);

    mDrawQuad.positions.left =
        mActiveSheet->convertToScreenSpaceX(mPosition.first);
    mDrawQuad.positions.right =
        mActiveSheet->convertToScreenSpaceX(mPosition.first + mPixelSize.width);
    mDrawQuad.positions.top =
        mActiveSheet->convertToScreenSpaceY(mPosition.second);
    mDrawQuad.positions.bottom = mActiveSheet->convertToScreenSpaceY(
        mPosition.second + mPixelSize.height);
    mDrawQuad.depth = mActiveSheet->convertToScreenSpaceZ(mZOrder);

    mInClip = mClipOnScreen.clip(mDrawQuad);
}

} // namespace Opde
