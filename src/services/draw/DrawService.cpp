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

// lg palette for default - moved to external definition file for readibility reasons...
#include <LGPalette.h>

using namespace std;
using namespace Ogre;

namespace Opde {

	const int DrawService::MAX_Z_VALUE = 1024;
	const RGBAQuad DrawService::msMonoPalette[2] = {{0,0,0,0}, {255,255,255,255}};

	/*----------------------------------------------------*/
	/*-------------------- DrawService -------------------*/
	/*----------------------------------------------------*/
	DrawService::DrawService(ServiceManager *manager, const std::string& name) : Service(manager, name),
			mSheetMap(),
			mActiveSheet(NULL),
			mDrawOpID(0),
			mDrawSourceID(0),
			mViewport(NULL),
			mCurrentPalette(NULL) {

		mCurrentPalette = msDefaultPalette;
	}

	//------------------------------------------------------
	DrawService::~DrawService() {
		// destroy all sheets...
		SheetMap::iterator it = mSheetMap.begin();

		// destroy all sheets
		for (; it != mSheetMap.end(); ++it) {
			delete it->second;
		}

		mSheetMap.clear();

		// destroy all draw operations left
		for (size_t idx = 0; idx < mDrawOperations.size(); ++idx) {
			// delete
			delete mDrawOperations[idx];
			mDrawOperations[idx] = NULL;
		}

		mDrawOperations.clear();

		// destroy all draw sources
		for (DrawSourceSet::iterator it = mDrawSources.begin(); it != mDrawSources.end(); ++it) {
			delete *it;
		}

		mDrawSources.clear();

		freeCurrentPal();
	}

	//------------------------------------------------------
	bool DrawService::init() {
		return true;
	}

	//------------------------------------------------------
	void DrawService::bootstrapFinished() {
		RenderServicePtr rsp = GET_SERVICE(RenderService);
		mViewport = rsp->getDefaultViewport();

		mRenderSystem = Ogre::Root::getSingleton().getRenderSystem();
		mXTextelOffset = mRenderSystem->getHorizontalTexelOffset();
		mYTextelOffset = mRenderSystem->getVerticalTexelOffset();

		mSceneManager = rsp->getSceneManager();
		mSceneManager->addRenderQueueListener(this);

	}

	//------------------------------------------------------
	void DrawService::shutdown() {
		// get rid of the render queue listener stuff
		mSceneManager->removeRenderQueueListener(this);
	}

	//------------------------------------------------------
	DrawSheet* DrawService::createSheet(const std::string& sheetName) {
		assert(!sheetName.empty());

		SheetMap::iterator it = mSheetMap.find(sheetName);

		if (it != mSheetMap.end())
			return it->second;
		else {
			DrawSheet* sheet = new DrawSheet(this, sheetName);
			mSheetMap[sheetName] = sheet;
			return sheet;
		}
	};

	//------------------------------------------------------
	void DrawService::destroySheet(DrawSheet* sheet) {
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

		delete sheet;
	}

	//------------------------------------------------------
	DrawSheet* DrawService::getSheet(const std::string& sheetName) const {
		SheetMap::const_iterator it = mSheetMap.find(sheetName);

		if (it != mSheetMap.end())
			return it->second;
		else
			return NULL;
	}

	//------------------------------------------------------
	void DrawService::setActiveSheet(DrawSheet* sheet) {
		if (mActiveSheet != sheet) {
			if (mActiveSheet)
				mActiveSheet->deactivate();

			mActiveSheet = sheet;

			if (mActiveSheet)
				mActiveSheet->activate();
		}
	}

	//------------------------------------------------------
	void DrawService::renderQueueStarted(uint8 queueGroupId, const String& invocation, bool& skipThisInvocation) {
		// Clear Z buffer to be ready to render overlayed meshes and stuff
		if(queueGroupId == RENDER_QUEUE_OVERLAY) {
			Ogre::Root::getSingleton().getRenderSystem()->clearFrameBuffer(Ogre::FBT_DEPTH);

			// and rebuild the atlasses as needed
			rebuildAtlases();
		}
	}

	//------------------------------------------------------
	void DrawService::rebuildAtlases() {
		AtlasSet::iterator it = mAtlasesForRebuild.begin();
		AtlasSet::iterator end = mAtlasesForRebuild.end();

		while (it != end) {
			(*it++)->build();
		}

		mAtlasesForRebuild.clear();
	}

	//------------------------------------------------------
	void DrawService::renderQueueEnded(uint8 queueGroupId, const String& invocation, bool& skipThisInvocation) {

	}

	//------------------------------------------------------
	FontDrawSource* DrawService::loadFont(TextureAtlas* atlas, const std::string& name, const std::string& group) {
		// load the font according to the specs
		assert(atlas);
		FontDrawSource* nfs = atlas->createFont(name);

		// now we'll load the glyphs from the file
		loadFonFile(name, group, nfs);

		return nfs;
	}

	//------------------------------------------------------
	void DrawService::loadFonFile(const std::string& name, const std::string& group, FontDrawSource* fon) {
		DarkFontHeader header;

		Ogre::DataStreamPtr Stream = Ogre::ResourceGroupManager::getSingleton().openResource(name, group, true, NULL);
		FilePtr fontFile = new OgreFile(Stream);

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

		// what format do we have?
		DarkPixelFormat dpf = DPF_8BIT;
		const RGBAQuad* curpalette = mCurrentPalette;

		if (header.Format == 0) {
			dpf = DPF_MONO;
			curpalette = msMonoPalette;
		} else if (header.Format == 0x0CCCC) {
			curpalette = msAAPalette;
			// these are inverted! At least it seems so.
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
	void DrawService::loadPaletteFromPCX(const Ogre::String& fname, const Ogre::String& group) {
		// Code written by patryn, reused here for the new font rendering pipeline support
		// Open the file
		Ogre::DataStreamPtr stream;
		FilePtr paletteFile;

		freeCurrentPal();

		try {
			stream = Ogre::ResourceGroupManager::getSingleton().openResource(fname, group, true);
			paletteFile = new OgreFile(stream);
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

		BYTE bpp;
		paletteFile->readElem(&bpp, 1);

		paletteFile->seek(3 * 256 + 1, File::FSEEK_END);
		BYTE padding;
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
		WORD count;
		unsigned int i;

		freeCurrentPal();

		// Open the file
		Ogre::DataStreamPtr stream;
		FilePtr paletteFile;

		try {
			stream = Ogre::ResourceGroupManager::getSingleton().openResource(fname, group, true);
			paletteFile = new OgreFile(stream);
		} catch(Ogre::FileNotFoundException) {
			// Could not find resource, use the default table
			LOG_ERROR("DrawService: Specified palette file not found - using default palette!");
			return;
		}

		mCurrentPalette = new RGBAQuad[256];

		// We're sure that we have external palette here:
		paletteFile->readElem(&paletteHeader.RiffSig, sizeof(DWORD));
		paletteFile->readElem(&paletteHeader.RiffLength, sizeof(DWORD));
		paletteFile->readElem(&paletteHeader.PSig1, sizeof(DWORD));
		paletteFile->readElem(&paletteHeader.PSig2, sizeof(DWORD));
		paletteFile->readElem(&paletteHeader.Length, sizeof(DWORD));

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
	DrawSource* DrawService::createDrawSource(const std::string& img, const std::string& group) {
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
		DrawSource* ds = new DrawSource(mDrawSourceID++, mat, tex);

		mDrawSources.insert(ds);

		return ds;
	}

	//------------------------------------------------------
	RenderedImage* DrawService::createRenderedImage(DrawSource* draw) {
		DrawOperation::ID opID = getNewDrawOperationID();
		RenderedImage* ri = new RenderedImage(this, opID, draw);

		// register so we'll be able to remove it
		mDrawOperations[opID] = ri;

		return ri;
	}

	//------------------------------------------------------
	RenderedLabel* DrawService::createRenderedLabel(FontDrawSource* fds, const std::string& label) {
		DrawOperation::ID opID = getNewDrawOperationID();
		RenderedLabel* rl = new RenderedLabel(this, opID, fds, label);

		// register so we'll be able to remove it
		mDrawOperations[opID] = rl;

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
		delete dop;
	}

	//------------------------------------------------------
	Ogre::Vector3 DrawService::convertToScreenSpace(int x, int y, int z) {
		Ogre::Vector3 res(x,y,getConvertedDepth(z));
		// Portions inspired by/taken from ajs's Canvas code
		res.x = ((res.x + mXTextelOffset) / mViewport->getActualWidth()) * 2.0f - 1.0f;
		res.y = ((res.y + mYTextelOffset) / mViewport->getActualHeight()) * -2.0f + 1.0f;

		return res;
	}

	//------------------------------------------------------
	Ogre::Real DrawService::getConvertedDepth(int z) {
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
	TextureAtlas* DrawService::createAtlas() {
		//
		TextureAtlas* ta = new TextureAtlas(this, getNewDrawOperationID());

		// TODO: Stack of atlases for release purposes (cleanup)

		return ta;
	}

	//------------------------------------------------------
	void DrawService::destroyAtlas(TextureAtlas* atlas) {
		mFreeIDs.push(atlas->getAtlasID());
		delete atlas;
	}


	//------------------------------------------------------
	void DrawService::freeCurrentPal() {
		if (mCurrentPalette != msDefaultPalette) {
			delete[] mCurrentPalette;
			mCurrentPalette = msDefaultPalette;
		}
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

	Service* DrawServiceFactory::createInstance(ServiceManager* manager) {
		return new DrawService(manager, mName);
	}

}
