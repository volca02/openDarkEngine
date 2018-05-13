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

#ifndef __DRAWSHEET_H
#define __DRAWSHEET_H

#include "DrawBuffer.h"
#include "DrawOperation.h"
#include <OgreRenderQueue.h>

namespace Opde {
// forward decl.
class DrawService;

/** A 2D rendering sheet. Represents one visible screen.
 * Stores rendering operations, can queue itself for rendering to ogre.
 * Uses DrawBuffer for render op. storage */
class DrawSheet : public Ogre::MovableObject {
public:
    /// Constructor
    DrawSheet(DrawService *owner, const std::string &sheetName);

    /// Destructor
    ~DrawSheet();

    /// Activates the sheet, prepares it for rendering
    void activate();

    /// Deactivates the sheet
    void deactivate();

    /// Adds a draw operation to be rendered on this sheet
    void addDrawOperation(DrawOperation *drawOp);

    /// Removes the draw operation from this sheet.
    void removeDrawOperation(DrawOperation *toRemove);

    /// Called when a draw source changed for given operation
    void _sourceChanged(DrawOperation *op, const DrawSourcePtr &oldsrc);

    /** Does the core removal of draw operation, without any notification
     * @note Internal, use removeDrawOperation instead
     */
    void _removeDrawOperation(DrawOperation *toRemove);

    /// Internal: Marks a certain draw operation dirty in this sheet. Called
    /// internally on a DrawOp. change
    void _markDirty(DrawOperation *drawOp);

    /// Will remove all Buffers with zero Draw operations
    void purge();

    //--- MovableObject mandatory ---
    const Ogre::String &getMovableType() const;
    const Ogre::AxisAlignedBox &getBoundingBox() const;
    Ogre::Real getBoundingRadius() const;
    void visitRenderables(Ogre::Renderable::Visitor *vis,
                          bool debugRenderables);

    bool isVisible() const;

    /// Called to ensure all the DrawBuffers are current and reflect the
    /// requested state
    void rebuildBuffers();

    /** Sets a resolution override for this sheet.
     * @param override specifies whether to use the override or not (and use the
     * real screen resolution)
     * @param width the width to use in overriden state
     * @param height the height to use in the overriden state*/
    void setResolutionOverride(bool override, size_t width = 0,
                               size_t height = 0);

    /** Converts the given coordinate to the screen space x coordinate
     */
    Ogre::Real convertToScreenSpaceX(int x) const;

    /** Converts the given coordinate to the screen space y coordinates
     */
    Ogre::Real convertToScreenSpaceY(int y) const;

    /** Converts the given coordinate to the screen space y coordinates
     * @param z the depth in 0 - MAX_Z_VALUE range
     * @return Real number describing the depth
     */
    Ogre::Real convertToScreenSpaceZ(int z) const;

    /** Informs this sheet the viewport resolution changed. Internal, do not use
     * explicitly (use setResolutionOverride instead).
     */
    void _setResolution(size_t width, size_t height);

    /** Creates a clip rectangle with the specified screen coordinates.
     */
    void convertClipToScreen(const ClipRect &cr, ScreenRect &tgt) const;

    /** Queues all the Renderables into appropriate RenderQueueGroups
     */
    void queueRenderables(Ogre::RenderQueue *rq);

    /** Clears all data present in the sheet
     */
    void clear();

protected:
    DrawBuffer *getBufferForOperation(DrawOperation *drawOp,
                                      bool autoCreate = false);

    DrawBuffer *getBufferForSourceID(DrawSourceBase::ID id);

    /// this does the rendering - updates the render queue with the buffers to
    /// display
    void _updateRenderQueue(Ogre::RenderQueue *queue);

    /// marks all buffers as dirty
    void markBuffersDirty();

    /// All draw buffers for the sheet as map
    DrawBufferMap mDrawBufferMap;

    /// True if this sheet is rendered
    bool mActive;

    std::unordered_set<DrawOperation*> mDrawOps;

    /// Sheet name
    std::string mSheetName;

    /// onwer of this sheet
    DrawService *mOwner;

    /// screen resolution override for this sheet
    bool mResOverride;

    /// Resolution of the screen (either updated or overriden)
    size_t mWidth, mHeight;
};
} // namespace Opde

#endif
