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

#include "config.h"

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "SharedPtr.h"
#include "DrawSheet.h"
#include "ManualFonFileLoader.h"
#include "Array.h"
#include "RenderedImage.h"
#include "RenderedLabel.h"
#include "TextureAtlas.h"

#include "RenderService.h"

#include <OgreViewport.h>
#include <OgreRenderQueueListener.h>

#include <stack>

namespace Opde {
	// Forward decl.
	class TextureAtlas;
	class FontDrawSource;

	/** @brief Draw Service - 2D rendering service.
	 * @author volca
	 * @note some parts written by Patryn as well
	 */
	class OPDELIB_EXPORT DrawService : public ServiceImpl<DrawService>, public Ogre::RenderQueueListener {
		public:
			/// Constructor
			DrawService(ServiceManager *manager, const std::string& name);

			/// Destructor
			virtual ~DrawService();

			/** Creates a new DrawSheet and inserts it into internal map.
			 * @param sheetName a unique sheet name
			 * @return new DrawSheet if none of that name exists, or the currently existing if sheet with that name already found
			 * */
			DrawSheetPtr createSheet(const std::string& sheetName);

			/** Destroys the given sheet, and removes it from the internal map
			 * @param sheet The sheet to destroy
			 */
			void destroySheet(const DrawSheetPtr& sheet);

			/** Returns the sheet of the given name
			 * @param sheetName the name of the sheet to return
			 * @return The sheet pointer, or NULL if none of that name exists
			 */
			DrawSheetPtr getSheet(const std::string& sheetName) const;

			/** Sets the active (currently displayed) sheet.
			 * @param sheet The sheet to display (or none if the parameter is NULL)
			 */
			void setActiveSheet(const DrawSheetPtr& sheet);
			
			/** Gets the currently active sheet */
			inline const DrawSheetPtr& getActiveSheet() const { return mActiveSheet; };

			/** Creates a DrawSource that represents a specified image.
			 * Also creates a material that is used to render the image.
			 * @param img The image name
			 * @return the draw source usable for draw operations
			 */
			DrawSourcePtr createDrawSource(const std::string& img, const std::string& group);
			
			/** Creates a rendered image (e.g. a sprite)
			 * @param draw The image source for this operation
			 */
			RenderedImage* createRenderedImage(const DrawSourcePtr& draw);
			
			/** Destroys a rendered image. For convenience. Calls destroyDrawOperation 
			 */
			void destroyRenderedImage(RenderedImage* ri);

			/** Creates a rendered label (e.g. a text)
			 * @param fds The font to use for the rendering
			 * @param label Optional label to start with
			 */
			RenderedLabel* createRenderedLabel(const FontDrawSourcePtr& fds, const std::string& label = "");

			/** Destroys the specified draw operation (any ancestor)
			 * @param dop The draw operation to destroy
			 */
			void destroyDrawOperation(DrawOperation* dop);

			/** Atlas factory. Generates a new texture atlas usable for storing numerous different images - which are then combined into a single draw call.
			 * @return New atlas pointer
			 */
			TextureAtlasPtr createAtlas();

			/** Destroys the given instance of texture atlas.
			 * @param atlas The atlas to be destroyed
			 */
			void destroyAtlas(const TextureAtlasPtr& atlas);

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
			void _queueAtlasForRebuild(TextureAtlas* atlas);

			/// Maximal Z value (maximal Z order of the 2d draw operation)
			static const int MAX_Z_VALUE;

			/// Render queue listener related
			void renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation);

			/// Render queue listener related
			void renderQueueEnded(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation);

			/** Font loading interface. It is capable of loading .fon files only at the moment.
			 * @note To supply additional settings, use the
			 */
			FontDrawSourcePtr loadFont(const TextureAtlasPtr& atlas, const std::string& name, const std::string& group);

			/** supplies the palette info - needed for 8Bit palletized font color loads. The specified palette is then used
			 * in further font loading operations.
			 */
			void setFontPalette(Ogre::ManualFonFileLoader::PaletteType paltype, const Ogre::String& fname = "", const Ogre::String& group = "");
			
			/** Getter for the current actual pixel width of the screen
			 * @todo Once the resolution handling is ok, rewrite to use mWidth instead (the same for height)
			*/
			inline size_t getActualWidth() const { /*return mWidth;*/ return mViewport->getActualWidth(); };
			
			/// Getter for the current actual pixel height of the screen
			inline size_t getActualHeight() const { /*return mHeight;*/ return mViewport->getActualHeight(); };
			
			/** registers a draw source ID as a holder of a image name and resource group name combination 
			*/
			void registerDrawSource(const DrawSourcePtr& ds, const Ogre::String& img, const Ogre::String& group);
			
			/// unregisters a draw source from the resource name to draw source mapping
			void unregisterDrawSource(const DrawSourcePtr& ds);
			
		protected:
			// Service related:
			bool init();
			void bootstrapFinished();
			void shutdown();
			
			void clear();

			/// Loads the LG's fon file and populates the given font instance with it's glyphs
			void loadFonFile(const std::string& name, const std::string& group, FontDrawSourcePtr fon);

			/// Loads the current palette from a specified pcx file
			void loadPaletteFromPCX(const Ogre::String& fname, const Ogre::String& group);

			/// Loads the palette from an external file
			void loadPaletteExternal(const Ogre::String& fname, const Ogre::String& group);

			DrawOperation::ID getNewDrawOperationID();

			/// Rebuilds all queued atlasses (so those can be used for rendering)
			void rebuildAtlases();

			/// frees the currently used font palette
			void freeCurrentPal();
			
			/// Callback from render service - resolution changed on the render window
			void onRenderServiceMsg(const RenderServiceMsg& msg);
			
			/// Finalizes the creation of object
			void postCreate(DrawOperation* dop);
			
			Ogre::String getResourcePath(const Ogre::String& res, const Ogre::String& grp);

			typedef std::map<std::string, DrawSheetPtr> SheetMap;
			typedef std::stack<size_t> IDStack;
			typedef SimpleArray<DrawOperation*> DrawOperationArray;
			typedef std::map<DrawSource::ID, TextureAtlasPtr> TextureAtlasMap;
			typedef std::set<TextureAtlas*> AtlasList;
			typedef std::list<DrawSourceBasePtr> DrawSourceList;
			typedef std::map<std::string, DrawSourcePtr> ResourceDrawSourceMap;

		private:
			SheetMap mSheetMap;
			DrawSheetPtr mActiveSheet;
			DrawOperation::ID mDrawOpID;
			DrawSource::ID mDrawSourceID; // draw sources in atlas have the same ID
			IDStack mFreeIDs;
			DrawOperationArray mDrawOperations;
			ResourceDrawSourceMap mResourceMap;

			Ogre::RenderSystem* mRenderSystem;

			Ogre::Real mXTextelOffset;
			Ogre::Real mYTextelOffset;

			Ogre::Viewport* mViewport;

			Ogre::SceneManager* mSceneManager;

			AtlasList mAtlasesForRebuild;

			static const RGBAQuad msMonoPalette[2];
			static RGBAQuad msDefaultPalette[256];
			static RGBAQuad msAAPalette[256];

			RGBAQuad* mCurrentPalette;

			DrawSourceList mDrawSources;
			
			RenderServicePtr mRenderService;
			
			TextureAtlasMap mAtlasMap;
			
			size_t mWidth;
			size_t mHeight;
			
			MessageSource<RenderServiceMsg>::ListenerID mRenderServiceCallBackID;
		};

	/// Shared pointer to the draw service
	typedef shared_ptr<DrawService> DrawServicePtr;


	/// Factory for the DrawService objects
	class OPDELIB_EXPORT DrawServiceFactory : public ServiceFactory {
		public:
			DrawServiceFactory();
			~DrawServiceFactory() {};

			/** Creates a GameService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

			virtual const uint getMask();
			
			virtual const size_t getSID();
		private:
			static std::string mName;
	};
}


#endif
