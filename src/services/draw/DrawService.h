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

#ifndef __DRAWSERVICE_H
#define __DRAWSERVICE_H

#include <stack>
#include <vector>

#include <OgreRenderQueueListener.h>

#include "DrawOperation.h"
#include "OpdeService.h"
#include "RGBAQuad.h"
#include "ServiceCommon.h"
#include "OpdeServiceFactory.h"


namespace Opde {

// Forward decl.
class TextureAtlas;
class FontDrawSource;
class DrawSheet;
class DrawOperation;
class RenderedImage;
class RenderedLabel;
class RenderedRect;
class RenderServiceMsg;

using DrawSheetPtr = std::shared_ptr<DrawSheet>;
using TextureAtlasPtr = std::shared_ptr<TextureAtlas>;

/** @brief Draw Service - 2D rendering service.
 * @author volca
 * @note some parts written by Patryn as well
 */
class DrawService : public ServiceImpl<DrawService>,
                    public Ogre::RenderQueueListener {
public:
    /// Constructor
    DrawService(ServiceManager *manager, const std::string &name);

    /// Destructor
    virtual ~DrawService();

    /** Creates a new DrawSheet and inserts it into internal map.
     * @param sheetName a unique sheet name
     * @return new DrawSheet if none of that name exists, or the currently
     * existing if sheet with that name already found
     * */
    DrawSheetPtr createSheet(const std::string &sheetName);

    /** Destroys the given sheet, and removes it from the internal map
     * @param sheet The sheet to destroy
     */
    void destroySheet(const DrawSheetPtr &sheet);

    /** Returns the sheet of the given name
     * @param sheetName the name of the sheet to return
     * @return The sheet pointer, or NULL if none of that name exists
     */
    DrawSheetPtr getSheet(const std::string &sheetName) const;

    /** Sets the active (currently displayed) sheet.
     * @param sheet The sheet to display (or none if the parameter is NULL)
     */
    void setActiveSheet(const DrawSheetPtr &sheet);

    /** Gets the currently active sheet */
    inline const DrawSheetPtr &getActiveSheet() const { return mActiveSheet; };

    /** Creates a DrawSource that represents a specified image.
     * Also creates a material that is used to render the image.
     * @param img The image name
     * @return the draw source usable for draw operations
     */
    DrawSourcePtr createDrawSource(const std::string &img,
                                   const std::string &group);

    /** Creates a rendered image (e.g. a sprite)
     * @param draw The image source for this operation
     */
    RenderedImage *createRenderedImage(const DrawSourcePtr &draw);

    /** Destroys a rendered image. For convenience. Calls destroyDrawOperation
     */
    void destroyRenderedImage(RenderedImage *ri);

    /** Creates a rendered label (e.g. a text)
     * @param fds The font to use for the rendering
     * @param label Optional label to start with
     */
    RenderedLabel *createRenderedLabel(const FontDrawSourcePtr &fds,
                                       const std::string &label = "");

    /** Destroys a rendered label. For convenience. Calls destroyDrawOperation
     */
    void destroyRenderedLabel(RenderedLabel *rl);

    /** Creates a rendered rectangle (e.g. a colour only rectangle render)
     * @param atlas The atlas for this operation
     * @note The redered rectangle still uses a texture (namely the vertex
     * colour texture). !That texture is used to group the rendering operations
     * together in one buffer to ensure !correct transparency/alpha handling.
     */
    RenderedRect *createRenderedRect(const TextureAtlasPtr &atlas);

    /** Destroys a rendered rectangle. For convenience. Calls
     * destroyDrawOperation
     */
    void destroyRenderedRect(RenderedRect *rr);

    /** Destroys the specified draw operation (any ancestor)
     * @param dop The draw operation to destroy
     */
    void destroyDrawOperation(DrawOperation *dop);

    /** Atlas factory. Generates a new texture atlas usable for storing numerous
     * different images - which are then combined into a single draw call.
     * @return New atlas pointer
     */
    TextureAtlasPtr createAtlas();

    /** Converts the given coordinate to the screen space x coordinate
     */
    Ogre::Real convertToScreenSpaceX(int x, size_t width) const;

    /** Converts the given coordinate to the screen space y coordinates
     */
    Ogre::Real convertToScreenSpaceY(int y, size_t height) const;

    /** Converts the given coordinate to the screen space y coordinates
     * @param z the depth in 0 - MAX_Z_VALUE range
     * @return Real number describing the depth
     */
    Ogre::Real convertToScreenSpaceZ(int z) const;

    /// Queues an atlas for rebuilding (on render queue started event)
    void _queueAtlasForRebuild(TextureAtlas *atlas);

    /// Maximal Z value (maximal Z order of the 2d draw operation)
    static const int MAX_Z_VALUE;

    /// Render queue listener related
    void renderQueueStarted(Ogre::uint8 queueGroupId,
                            const Ogre::String &invocation,
                            bool &skipThisInvocation);

    /// Render queue listener related
    void renderQueueEnded(Ogre::uint8 queueGroupId,
                          const Ogre::String &invocation,
                          bool &skipThisInvocation);

    /** Font loading interface. It is capable of loading .fon files only at the
     * moment.
     * @note To supply additional settings, use the
     */
    FontDrawSourcePtr loadFont(const TextureAtlasPtr &atlas,
                               const std::string &name,
                               const std::string &group);

    /** supplies the palette info - needed for 8Bit palletized font color loads.
     * The specified palette is then used in further font loading operations.
     */
    void setFontPalette(PaletteType paltype,
                        const Ogre::String &fname = "",
                        const Ogre::String &group = "");

    /** Getter for the current actual pixel width of the screen
     * @todo Once the resolution handling is ok, rewrite to use mWidth instead
     * (the same for height)
     */
    size_t getActualWidth() const;

    /// Getter for the current actual pixel height of the screen
    size_t getActualHeight() const;

    /** registers a draw source ID as a holder of a image name and resource
     * group name combination
     */
    void registerDrawSource(const DrawSourcePtr &ds, const Ogre::String &img,
                            const Ogre::String &group);

    /// unregisters a draw source from the resource name to draw source mapping
    void unregisterDrawSource(const DrawSourcePtr &ds);

protected:
    // Service related:
    bool init();
    void bootstrapFinished();
    void shutdown();

    void clear();

    /// Loads the LG's fon file and populates the given font instance with it's
    /// glyphs
    void loadFonFile(const std::string &name, const std::string &group,
                     FontDrawSourcePtr fon);

    /// Loads the current palette from a specified pcx file
    void loadPaletteFromPCX(const Ogre::String &fname,
                            const Ogre::String &group);

    /// Loads the palette from an external file
    void loadPaletteExternal(const Ogre::String &fname,
                             const Ogre::String &group);

    std::unique_ptr<DrawOperation> &allocDrawOpSlot();

    /// Rebuilds all queued atlasses (so those can be used for rendering)
    void rebuildAtlases();

    /// frees the currently used font palette
    void freeCurrentPal();

    /// Callback from render service - resolution changed on the render window
    void onRenderServiceMsg(const RenderServiceMsg &msg);

    /// Finalizes the creation of object
    void postCreate(DrawOperation *dop);

    Ogre::String getResourcePath(const Ogre::String &res,
                                 const Ogre::String &grp);

    typedef std::map<std::string, DrawSheetPtr> SheetMap;
    typedef std::vector<std::unique_ptr<DrawOperation>> DrawOperations;
    typedef std::unordered_set<TextureAtlasPtr> TextureAtlases;
    typedef std::set<TextureAtlas *> AtlasList;
    typedef std::vector<DrawSourceBasePtr> DrawSourceList;
    typedef std::map<std::string, DrawSourcePtr> ResourceDrawSourceMap;

private:
    SheetMap mSheetMap;
    DrawSheetPtr mActiveSheet;
    DrawSource::ID mDrawSourceID; // draw sources in atlas have the same ID
    DrawOperations mDrawOperations;
    ResourceDrawSourceMap mResourceMap;

    Ogre::RenderSystem *mRenderSystem;

    Ogre::Real mXTextelOffset;
    Ogre::Real mYTextelOffset;

    Ogre::Viewport *mViewport;

    Ogre::SceneManager *mSceneManager;

    // non-owning
    AtlasList mAtlasesForRebuild;

    static const RGBAQuad msMonoPalette[2];

    // TODO(volca): use std::array or similar here. No naked pointers
    RGBAQuad *mCurrentPalette;

    DrawSourceList mDrawSources;

    RenderServicePtr mRenderService;

    TextureAtlases mAtlases;

    size_t mWidth;
    size_t mHeight;

    MessageListenerID mRenderServiceCallBackID;
};

/// Shared pointer to the draw service
typedef shared_ptr<DrawService> DrawServicePtr;

/// Factory for the DrawService objects
class DrawServiceFactory : public ServiceFactory {
public:
    DrawServiceFactory();
    ~DrawServiceFactory(){};

    /** Creates a GameService instance */
    Service *createInstance(ServiceManager *manager);

    const std::string &getName() override;

    const uint getMask() override;

    const size_t getSID() override;

private:
    static const std::string mName;
};
} // namespace Opde

#endif
