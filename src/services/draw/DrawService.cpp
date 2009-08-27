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

#include "config.h"
#include "integers.h"
#include "DrawService.h"
#include "OpdeException.h"
#include "ServiceCommon.h"
#include "RenderService.h"
#include "TextureAtlas.h"
#include "FonFormat.h"
#include "StringTokenizer.h"

#include <OgreTexture.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreTextureUnitState.h>
#include <OgreTextureManager.h>
#include <OgreMaterial.h>
#include <OgreMaterialManager.h>

// lg palette for default - moved to external definition file for readability reasons...
#include <LGPalette.h>

using namespace std;
using namespace Ogre;

namespace Opde {

	const int DrawService::MAX_Z_VALUE = 1024;
	const RGBAQuad DrawService::msMonoPalette[2] = {{0,0,0,0}, {255,255,255,255}};

	/*----------------------------------------------------*/
	/*-------------------- DrawService -------------------*/
	/*----------------------------------------------------*/
	template<> const size_t ServiceImpl<DrawService>::SID = __SERVICE_ID_DRAW;
	
	DrawService::DrawService(ServiceManager *manager, const std::string& name) : ServiceImpl<DrawService>(manager, name),
			mSheetMap(),
			mActiveSheet(NULL),
			mDrawOpID(0),
			mDrawSourceID(0),
			mViewport(NULL),
			mCurrentPalette(NULL),
			mWidth(1),
			mHeight(1) {

		mCurrentPalette = msDefaultPalette;
	}

	//------------------------------------------------------
	DrawService::~DrawService() {
		// destroy all sheets...
		SheetMap::iterator it = mSheetMap.begin();

		// destroy all sheets
		for (; it != mSheetMap.end(); ++it) {
			it->second->clear();
		}

		mSheetMap.clear();

		// destroy all draw operations left
		for (size_t idx = 0; idx < mDrawOperations.size(); ++idx) {
			// delete
			DrawOperation* dop = mDrawOperations[idx];
			
			if (dop != NULL)
				dop->clear();
			
			delete dop;
			mDrawOperations[idx] = NULL;
		}

		TextureAtlasMap::iterator end = mAtlasMap.end();
		for (TextureAtlasMap::iterator it = mAtlasMap.begin(); it != end; ++it) {
			destroyAtlas(it->second);
		}
		
		mAtlasMap.clear();
		
		mDrawOperations.clear();

		// destroy all draw sources
		mDrawSources.clear();
		
		mResourceMap.clear();

		freeCurrentPal();
	}

	//------------------------------------------------------
	bool DrawService::init() {
		return true;
	}

	//------------------------------------------------------
	void DrawService::bootstrapFinished() {
		mRenderService = GET_SERVICE(RenderService);
		mViewport = mRenderService->getDefaultViewport();

		mRenderSystem = Ogre::Root::getSingleton().getRenderSystem();
		mXTextelOffset = mRenderSystem->getHorizontalTexelOffset();
		mYTextelOffset = mRenderSystem->getVerticalTexelOffset();

		mSceneManager = mRenderService->getSceneManager();
		mSceneManager->addRenderQueueListener(this);
		
		mRenderServiceCallBackID = 
			mRenderService->registerListener(
					RenderService::ListenerPtr(
							new ClassCallback<RenderServiceMsg, DrawService>(this, &DrawService::onRenderServiceMsg))
				);
	}

	//------------------------------------------------------
	void DrawService::shutdown() {
		// get rid of the render queue listener stuff
		mSceneManager->removeRenderQueueListener(this);
		
		mRenderService->unregisterListener(mRenderServiceCallBackID);
		mRenderService.setNull(); // break the circle 
	}

	//------------------------------------------------------
	DrawSheetPtr DrawService::createSheet(const std::string& sheetName) {
		assert(!sheetName.empty());

		SheetMap::iterator it = mSheetMap.find(sheetName);

		if (it != mSheetMap.end())
			return it->second;
		else {
			DrawSheetPtr sheet(new DrawSheet(this, sheetName));
			mSheetMap[sheetName] = sheet;
			return sheet;
		}
	};

	//------------------------------------------------------
	void DrawService::destroySheet(const DrawSheetPtr& sheet) {
		// find it in the map, remove, then delete
		SheetMap::iterator it = mSheetMap.begin();

		while(it != mSheetMap.end()) {

			if (it->second == sheet) {
				SheetMap::iterator cur = it++;

				mSheetMap.erase(cur);
			} else {
				++it;
			}
		}

		if (mActiveSheet == sheet)
			setActiveSheet(DrawSheetPtr(NULL));
		
		sheet->clear();
		// and we're done
	}

	//------------------------------------------------------
	DrawSheetPtr DrawService::getSheet(const std::string& sheetName) const {
		SheetMap::const_iterator it = mSheetMap.find(sheetName);

		if (it != mSheetMap.end())
			return it->second;
		else
			return DrawSheetPtr(NULL);
	}

	//------------------------------------------------------
	void DrawService::setActiveSheet(const DrawSheetPtr& sheet) {
		if (mActiveSheet != sheet) {
			if (!mActiveSheet.isNull())
				mActiveSheet->deactivate();

			mActiveSheet = sheet;

			if (!mActiveSheet.isNull())
				mActiveSheet->activate();
		}
	}

	//------------------------------------------------------
	void DrawService::renderQueueStarted(uint8 queueGroupId, const String& invocation, bool& skipThisInvocation) {
		// Clear Z buffer to be ready to render overlayed meshes and stuff
		if(queueGroupId == RENDER_QUEUE_BACKGROUND) {
			// and rebuild the atlasses as needed
			rebuildAtlases();
			
			// and render the 2d parts
			if (!mActiveSheet.isNull()) {
				// update the render queue to contain the rendereables of the sheet
				mActiveSheet->queueRenderables(mSceneManager->getRenderQueue());
			}
			
		} else if(queueGroupId == RENDER_QUEUE_OVERLAY) { 
			Ogre::Root::getSingleton().getRenderSystem()->clearFrameBuffer(Ogre::FBT_DEPTH);
		}
	}

	//------------------------------------------------------
	void DrawService::rebuildAtlases() {
		AtlasList::iterator it = mAtlasesForRebuild.begin();
		AtlasList::iterator end = mAtlasesForRebuild.end();

		while (it != end) {
			(*it++)->build();
		}

		mAtlasesForRebuild.clear();
	}

	//------------------------------------------------------
	void DrawService::renderQueueEnded(uint8 queueGroupId, const String& invocation, bool& skipThisInvocation) {

	}

	//------------------------------------------------------
	FontDrawSourcePtr DrawService::loadFont(const TextureAtlasPtr& atlas, const std::string& name, const std::string& group) {
		// load the font according to the specs
		FontDrawSourcePtr nfs(new FontDrawSource(atlas, name));

		atlas->_addFont(nfs);
		
		// now we'll load the glyphs from the file
		loadFonFile(name, group, nfs);

		return nfs;
	}

	//------------------------------------------------------
	void DrawService::loadFonFile(const std::string& name, const std::string& group, FontDrawSourcePtr fon) {
		DarkFontHeader header;

		Ogre::DataStreamPtr Stream = Ogre::ResourceGroupManager::getSingleton().openResource(name, group, true, NULL);
		FilePtr fontFile(new OgreFile(Stream));

		fontFile->readElem(&header.Format,2); // 0
		fontFile->readElem(&header.Unknown,1); // 2
		fontFile->readElem(&header.Palette,1); // 3
		fontFile->readElem(&header.Zeros1, 1, 32); // 4
		fontFile->readElem(&header.FirstChar, 2); // 36
		fontFile->readElem(&header.LastChar, 2); // 38
		fontFile->readElem(&header.Zeros2, 1, 32); // 40
		fontFile->readElem(&header.WidthOffset, 4); // 72
		fontFile->readElem(&header.BitmapOffset, 4); // 76
		fontFile->readElem(&header.RowWidth, 2); // 80
		fontFile->readElem(&header.NumRows, 2); // 82

		LOG_DEBUG("DrawService: Loading font '%s'", name.c_str());
		
		// what format do we have?
		DarkPixelFormat dpf = DPF_8BIT;
		const RGBAQuad* curpalette = mCurrentPalette;

		if (header.Format == 0) {
			dpf = DPF_MONO;
			curpalette = msMonoPalette;
			LOG_DEBUG("DrawService: Font is monochromatic");
		} else if (header.Format == 0x0CCCC) {
			LOG_DEBUG("DrawService: Font is antialiased");
			if (header.Palette != 0) // 0 == use current, otherwise we'll use the default one
				curpalette = msAAPalette;
			// these are inverted! At least it seems so.
		} else {
			LOG_DEBUG("DrawService: Font 8Bit with palette");
			if (header.Palette != 0) // 0 == use current, otherwise we'll use the default one
				curpalette = msDefaultPalette;
		}
		
		size_t nchars = header.LastChar - header.FirstChar + 1;

		uint16_t* columns = new uint16_t[nchars + 1];

		// seek to the char column specs
		fontFile->seek(header.WidthOffset, File::FSEEK_BEG);
		fontFile->readElem(columns, 2, nchars + 1);

		// seek to the bitmap part
		fontFile->seek(header.BitmapOffset, File::FSEEK_BEG);

		size_t bitmapSize = fontFile->size() - header.BitmapOffset;

		unsigned char *bitmap = new unsigned char[bitmapSize];

		fontFile->read(bitmap, bitmapSize);

		// load the characters
		for (FontCharType n = 0; n < nchars; ++n) {
			assert(columns[n+1] >= columns[n]);
			
			// span is columns[chr] to columns[chr+1]
			size_t width = columns[n+1] - columns[n];
			PixelSize ps(width, header.NumRows);
			
			fon->addGlyph(n + header.FirstChar, ps, dpf, header.RowWidth, bitmap, columns[n], curpalette);
		}

		delete[] bitmap;
		delete[] columns;
		LOG_INFO("DrawService: Loaded font '%s'", name.c_str());
	}


	//------------------------------------------------------
	void DrawService::setFontPalette(Ogre::ManualFonFileLoader::PaletteType paltype, const Ogre::String& fname, const Ogre::String& group) {
		switch (paltype) {
			case ManualFonFileLoader::ePT_Default:
				freeCurrentPal();
				break;
			case ManualFonFileLoader::ePT_DefaultBook:
			case ManualFonFileLoader::ePT_PCX:
				loadPaletteFromPCX(fname, group);
				break;
			case ManualFonFileLoader::ePT_External:
				loadPaletteExternal(fname, group);
				break;
			default:
				freeCurrentPal();
				LOG_ERROR("DrawService: Invalid type for palette specified (not loading '%s'). Using default palette instead.", fname.c_str());
		}
	}

	//------------------------------------------------------
	void DrawService::registerDrawSource(const DrawSourcePtr& ds, const Ogre::String& img, const Ogre::String& group) {
		// TODO: Code
		mResourceMap.insert(std::make_pair(getResourcePath(img, group), ds));
	}
	
	//------------------------------------------------------
	void DrawService::unregisterDrawSource(const DrawSourcePtr& ds) {
		ResourceDrawSourceMap::iterator it = mResourceMap.begin();
		
		while (it != mResourceMap.end()) {
			DrawSourcePtr cds = it->second;
			
			if (cds == ds) {
				ResourceDrawSourceMap::iterator th = it++;
				mResourceMap.erase(th);
			} else {
				++it;
			}
		}
	}
	
	//------------------------------------------------------
	void DrawService::loadPaletteFromPCX(const Ogre::String& fname, const Ogre::String& group) {
		// Code written by patryn, reused here for the new font rendering pipeline support
		// Open the file
		Ogre::DataStreamPtr stream;
		FilePtr paletteFile;

		freeCurrentPal();

		try {
			stream = Ogre::ResourceGroupManager::getSingleton().openResource(fname, group, true);
			paletteFile = FilePtr(new OgreFile(stream));
		} catch(Ogre::FileNotFoundException) {
			// Could not find resource, use the default table
			LOG_ERROR("DrawService: Specified palette file not found - using default palette!");
			return;
		}

		mCurrentPalette = new RGBAQuad[256];

		// Test to see if we're facing a PCX file. (0A) (xx) (01)
		uint8_t manuf, enc;
		paletteFile->read(&manuf, 1);
		paletteFile->seek(2);
		paletteFile->read(&enc, 1);

		if (manuf != 0x0A || enc != 0x01) { // Invalid file, does not seem like a PCX at all
			freeCurrentPal();
			LOG_ERROR("DrawService: invalid palette file specified (%s) - seems not to be a PCX file!", fname.c_str());
			return;
		}

		uint8_t bpp;
		paletteFile->readElem(&bpp, 1);

		paletteFile->seek(3 * 256 + 1, File::FSEEK_END);
		uint8_t padding;
		paletteFile->readElem(&padding, 1);

		if((bpp == 8) && (padding == 0x0C)) { //Make sure it is an 8bpp and a valid PCX
			// Byte sized structures - endianness always ok
			for (unsigned int i = 0; i < 256; i++) {
				paletteFile->read(&mCurrentPalette[i].red, 1);
				paletteFile->read(&mCurrentPalette[i].green, 1);
				paletteFile->read(&mCurrentPalette[i].blue, 1);
				mCurrentPalette[i].alpha = i == 0 ? 0 : 255;
			}
		} else {
			freeCurrentPal();
			LOG_ERROR("DrawService: Invalid palette file (%s) specified - not 8 BPP or invalid Padding!", fname.c_str());
			return;
		}

		return;
	}

	//------------------------------------------------------
	void DrawService::loadPaletteExternal(const Ogre::String& fname, const Ogre::String& group) {
		// Code written by patryn, reused here for the new font rendering pipeline support
		ExternalPaletteHeader paletteHeader;
		uint16_t count;
		unsigned int i;

		freeCurrentPal();

		// Open the file
		Ogre::DataStreamPtr stream;
		FilePtr paletteFile;

		try {
			stream = Ogre::ResourceGroupManager::getSingleton().openResource(fname, group, true);
			paletteFile = FilePtr(new OgreFile(stream));
		} catch(Ogre::FileNotFoundException) {
			// Could not find resource, use the default table
			LOG_ERROR("DrawService: Specified palette file not found - using default palette!");
			return;
		}

		mCurrentPalette = new RGBAQuad[256];

		// We're sure that we have external palette here:
		paletteFile->readElem(&paletteHeader.RiffSig, sizeof(uint32_t));
		paletteFile->readElem(&paletteHeader.RiffLength, sizeof(uint32_t));
		paletteFile->readElem(&paletteHeader.PSig1, sizeof(uint32_t));
		paletteFile->readElem(&paletteHeader.PSig2, sizeof(uint32_t));
		paletteFile->readElem(&paletteHeader.Length, sizeof(uint32_t));

		if (paletteHeader.RiffSig == 0x46464952) {
			if (paletteHeader.PSig1 != 0x204C4150) {
				freeCurrentPal();
				LOG_ERROR("DrawService: Invalid external palette signature (%s)!", fname.c_str());
				return;
			}

			paletteFile->seek(2L, StdFile::FSEEK_CUR);
			paletteFile->readElem(&count, 2);
			if (count > 256)
				count = 256;

			for (i = 0; i < count; i++)
			{
				paletteFile->read(&mCurrentPalette[i].blue, 1);
				paletteFile->read(&mCurrentPalette[i].green, 1);
				paletteFile->read(&mCurrentPalette[i].red, 1);
				paletteFile->read(&mCurrentPalette[i].alpha, 1);
				mCurrentPalette[i].alpha = i == 0 ? 0 : 255; // alpha read from file is most probably bogus, so we're just skipping it
			}
		} else if (paletteHeader.RiffSig == 0x4353414A) {
			paletteFile->seek(0); // it is a text file JASC!
			std::string line = paletteFile->getLine();

			if (line != "JASC-PAL") {
				LOG_ERROR("Not a RIFF nor JASC-PAL file although is seemed so (%s). Defaulting the palette.", fname.c_str());
				freeCurrentPal();
				return;
			}

			// version (0100?)
			line = paletteFile->getLine();

			// First line contains the count of records
			line = paletteFile->getLine();

			count = Ogre::StringConverter::parseLong(line);

			if (count > 256)
				count = 256;

			for (i = 0; i < count; i++)	{
				line = paletteFile->getLine();
				WhitespaceStringTokenizer toker(line, true);

				mCurrentPalette[i].red   = Ogre::StringConverter::parseUnsignedInt(toker.next());
				mCurrentPalette[i].green = Ogre::StringConverter::parseUnsignedInt(toker.next());
				mCurrentPalette[i].blue  = Ogre::StringConverter::parseUnsignedInt(toker.next());
				mCurrentPalette[i].alpha = i == 0 ? 0 : 255;
			}
		} else {
			// raw file
			paletteFile->seek(0);

			for (i = 0; !paletteFile->eof(); i++) {
				paletteFile->read(&mCurrentPalette[i].blue, 1);
				paletteFile->read(&mCurrentPalette[i].green, 1);
				paletteFile->read(&mCurrentPalette[i].red, 1);
				mCurrentPalette[i].alpha = i == 0 ? 0 : 255;
			}
		}
	}

	//------------------------------------------------------
	DrawSourcePtr DrawService::createDrawSource(const std::string& img, const std::string& group) {
		Ogre::String pth = getResourcePath(img, group);
		
		ResourceDrawSourceMap::iterator it = mResourceMap.find(pth);
		
		if (it != mResourceMap.end())
			return it->second;
		
		// First we load the image.
		TexturePtr tex = Ogre::TextureManager::getSingleton().load(img, group, TEX_TYPE_2D, 1);

		MaterialPtr mat = Ogre::MaterialManager::getSingleton().create(img, group);
		TextureUnitState* tus = mat->getTechnique(0)->getPass(0)->createTextureUnitState(tex->getName());

		tus->setTextureFiltering(Ogre::FO_NONE, Ogre::FO_NONE, Ogre::FO_NONE);

		Pass *pass = mat->getTechnique(0)->getPass(0);
		pass->setAlphaRejectSettings(CMPF_GREATER, 128);
		pass->setLightingEnabled(false);

		mat->load();

		// will set up the pixelsize automatically for us
		DrawSourcePtr ds(new DrawSource(this, mDrawSourceID++, mat, tex));

		mDrawSources.push_back(ds);
		
		registerDrawSource(ds, img, group);
		
		return ds;
	}

	//------------------------------------------------------
	RenderedImage* DrawService::createRenderedImage(const DrawSourcePtr& draw) {
		DrawOperation::ID opID = getNewDrawOperationID();
		RenderedImage* ri = new RenderedImage(this, opID, draw);

		// register so we'll be able to remove it
		mDrawOperations[opID] = ri;
		
		postCreate(ri);

		return ri;
	}
	
	//------------------------------------------------------
	void DrawService::destroyRenderedImage(RenderedImage* ri) {
		destroyDrawOperation(ri);
	}

	//------------------------------------------------------
	RenderedLabel* DrawService::createRenderedLabel(const FontDrawSourcePtr& fds, const std::string& label) {
		DrawOperation::ID opID = getNewDrawOperationID();
		RenderedLabel* rl = new RenderedLabel(this, opID, fds, label);

		// register so we'll be able to remove it
		mDrawOperations[opID] = rl;

		postCreate(rl);
		
		return rl;
	}

	//------------------------------------------------------
	size_t DrawService::getNewDrawOperationID() {
		// do we have some free id's in the stack?
		if (mFreeIDs.empty()) {
			// raise the id
			mDrawOpID++;

			size_t prev_size = mDrawOperations.size();
			mDrawOperations.grow(mDrawOpID * 2);

			// and clean up the allocated mess
			for (size_t pos = prev_size; pos < mDrawOpID * 2; ++pos)
				mDrawOperations[pos] = NULL;

			return mDrawOpID;
		} else {
			size_t newid = mFreeIDs.top();
			mFreeIDs.pop();
			return newid;
		}
	}

	//------------------------------------------------------
	void DrawService::destroyDrawOperation(DrawOperation* dop) {
		DrawOperation::ID id = dop->getID();
		mDrawOperations[id] = NULL;
		// recycle the id for reuse
		mFreeIDs.push(id);
		
		dop->clear();
		delete dop;
	}

	//------------------------------------------------------
	Ogre::Real DrawService::convertToScreenSpaceX(int x, size_t width) const {
		Ogre::Real res = x;

		res = ((res + mXTextelOffset) / (Ogre::Real)(width)) * 2.0f - 1.0f;
		
		return res;
	}
	
	
	//------------------------------------------------------
	Ogre::Real DrawService::convertToScreenSpaceY(int y, size_t height) const {
		Ogre::Real res = y;
		
		res = ((res + mYTextelOffset) / (Ogre::Real)(height)) * -2.0f + 1.0f;

		return res;
	}
				
	//------------------------------------------------------
	Ogre::Real DrawService::convertToScreenSpaceZ(int z) const {
		Ogre::Real depth = mRenderSystem->getMaximumDepthInputValue() - mRenderSystem->getMinimumDepthInputValue();

		if (z < 0)
			z = 0;

		if (z > MAX_Z_VALUE)
			z = MAX_Z_VALUE;

		return mRenderSystem->getMaximumDepthInputValue() - (z * depth / MAX_Z_VALUE);
	}
	

	//------------------------------------------------------
	void DrawService::_queueAtlasForRebuild(TextureAtlas* atlas) {
		mAtlasesForRebuild.insert(atlas);
	}

	//------------------------------------------------------
	TextureAtlasPtr DrawService::createAtlas() {
		//
		TextureAtlasPtr ta(new TextureAtlas(this, getNewDrawOperationID()));

		mAtlasMap.insert(std::make_pair(ta->getAtlasID(), ta));

		return ta;
	}

	//------------------------------------------------------
	void DrawService::destroyAtlas(const TextureAtlasPtr& atlas) {
		/* TODO: INVALID:
		mFreeIDs.push(atlas->getAtlasID());
		delete atlas;
		*/
		LOG_INFO("DrawService::destroyAtlas ignored");
	}


	//------------------------------------------------------
	void DrawService::freeCurrentPal() {
		if (mCurrentPalette != msDefaultPalette) {
			delete[] mCurrentPalette;
			mCurrentPalette = msDefaultPalette;
		}
	}
	
	//------------------------------------------------------
	void DrawService::onRenderServiceMsg(const RenderServiceMsg& msg) {
		mWidth = msg.size.width;
		mHeight = msg.size.height;
		
		// inform all sheets...
		SheetMap::iterator it = mSheetMap.begin();

		while(it != mSheetMap.end()) {
			const DrawSheetPtr& sht = (it++)->second;
			
			sht->_setResolution(mWidth, mHeight);
		}
	}
	
	//------------------------------------------------------
	void DrawService::postCreate(DrawOperation *dop) {
		dop->_notifyActiveSheet(mActiveSheet.ptr());
	}
	
	//------------------------------------------------------
	Ogre::String DrawService::getResourcePath(const Ogre::String& res, const Ogre::String& grp) {
		return grp + ':' + res;
	}

	//-------------------------- Factory implementation
	std::string DrawServiceFactory::mName = "DrawService";

	DrawServiceFactory::DrawServiceFactory() : ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
	};

	const std::string& DrawServiceFactory::getName() {
		return mName;
	}

	const uint DrawServiceFactory::getMask() {
		return SERVICE_ENGINE;
	}

	const size_t DrawServiceFactory::getSID() {
		return DrawService::SID;
	}

	Service* DrawServiceFactory::createInstance(ServiceManager* manager) {
		return new DrawService(manager, mName);
	}

}
