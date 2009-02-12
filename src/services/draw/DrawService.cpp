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

#include <OgreTexture.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreTextureUnitState.h>
#include <OgreTextureManager.h>
#include <OgreMaterial.h>
#include <OgreMaterialManager.h>

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
		
		// TODO: if (mCurrentPalette != DefaultPalette)
		delete[] mCurrentPalette;
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
			DrawSheet* sheet = new DrawSheet(sheetName);
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
		
		Ogre::DataStreamPtr Stream = Ogre::ResourceGroupManager::getSingleton().openResource(name, name, true, NULL);
		FilePtr fontFile = new OgreFile(Stream);
		
		fontFile->readElem(&header.Format,2);
		fontFile->readElem(&header.Unknown,1);
		fontFile->readElem(&header.Palette,1);
		fontFile->readElem(&header.Zeros1, 1, 32);
		fontFile->readElem(&header.FirstChar, 2);
		fontFile->readElem(&header.LastChar, 2);
		fontFile->readElem(&header.Zeros2, 1, 32);
		fontFile->readElem(&header.WidthOffset, 4);
		fontFile->readElem(&header.BitmapOffset, 4);
		fontFile->readElem(&header.RowWidth, 2);
		fontFile->readElem(&header.NumRows, 2);
	
		// what format do we have?
		DarkPixelFormat dpf;
		const RGBAQuad* curpalette;
		
		if (header.Format == 0) { 
			dpf = DPF_MONO;
			curpalette = msMonoPalette;
		} else {
			dpf = DPF_8BIT;
			curpalette = mCurrentPalette;
		}
			
		
		size_t nchars = header.LastChar - header.FirstChar + 1; 
		
		uint16_t* columns = new uint16_t[nchars + 1];
			
		// seek to the char column specs
		fontFile->seek(header.WidthOffset, File::FSEEK_BEG);
		fontFile->readElem(columns, 2, nchars+1);
		
		// seek to the bitmap part
		fontFile->seek(header.BitmapOffset, File::FSEEK_BEG);
		
		size_t bitmapSize = fontFile->size() - header.BitmapOffset;  
		
		unsigned char *bitmap = new unsigned char[bitmapSize];
		unsigned char *bmpos = bitmap;
		
		fontFile->read(bitmap, bitmapSize);
		
		// load the characters
		for (FontCharType chr = header.FirstChar; chr <= header.LastChar; ++chr) {
			// span is columns[chr] to columns[chr+1]
			size_t width = columns[chr+1] - columns[chr];
			PixelSize ps(width, header.NumRows);
			fon->addGlyph(chr, ps, dpf, header.RowWidth, bmpos, curpalette);
			bmpos += header.RowWidth * header.NumRows;
		}
		
		delete[] bitmap;
		delete[] columns;
	}

	
	//------------------------------------------------------
	void DrawService::setFontPalette(Ogre::ManualFonFileLoader::PaletteType paltype, const Ogre::String& fname) {
		// Code written by patryn, reused here for the new font rendering pipeline support
		/*
		ExternalPaletteHeader PaletteHeader;
		WORD Count;
		char *Buffer, *C;
		BYTE S;
		unsigned int I;

		// Open the file
		Ogre::DataStreamPtr Stream;
		
		try 
		{
			Stream = Ogre::ResourceGroupManager::getSingleton().openResource(mPaletteFileName, mFontGroup, true);
			mPaletteFile = new OgreFile(Stream);
		} catch(Ogre::FileNotFoundException) {
			// Could not find resource, use the default table
			LogManager::getSingleton().logMessage("Specified palette file not found - using default palette!");
			return (RGBQUAD*)ColorTable;
		}
		
		RGBQUAD *Palette = new RGBQUAD[256];
		if (!Palette)
			return NULL;
		
		if((mPaletteType == ePT_PCX) || (mPaletteType == ePT_DefaultBook))	// PCX file specified...
		{
			RGBQUAD *Palette = new RGBQUAD[256];
			
			// Test to see if we're facing a PCX file. (0A) (xx) (01)
			uint8_t Manuf, Enc;
			mPaletteFile->read(&Manuf, 1);
			mPaletteFile->seek(2);
			mPaletteFile->read(&Enc, 1);
			
			if (Manuf != 0x0A || Enc != 0x01) // Invalid file, does not seem like a PCX at all
			{ 
				delete[] Palette; // Clean up!
				LogManager::getSingleton().logMessage("Invalid palette file specified - seems not to be a PCX file!");
				return (RGBQUAD*)ColorTable; // Should not matter - the cast (if packed)
			}
			
			BYTE BPP;
			mPaletteFile->readElem(&BPP, 1);
			
			mPaletteFile->seek(3 * 256 + 1, File::FSEEK_END);
			BYTE Padding;
			mPaletteFile->readElem(&Padding, 1);
			
			if((BPP == 8) && (Padding == 0x0C)) //Make sure it is an 8bpp and a valid PCX
			{
				// Byte sized structures - endianness always ok
				for (unsigned int I = 0; I < 256; I++)
				{
					
					mPaletteFile->read(&Palette[I].rgbRed, 1);
					mPaletteFile->read(&Palette[I].rgbGreen, 1);
					mPaletteFile->read(&Palette[I].rgbBlue, 1);
					Palette[I].rgbReserved = 0;
				}
			} else 
			{
				delete[] Palette; // Clean up!
				LogManager::getSingleton().logMessage("Invalid palette file specified - not 8 BPP or invalid Padding!");
				return (RGBQUAD*)ColorTable; // Return default palette
			}			
			return Palette;
		}
		
		if (mPaletteType != ePT_External) 
		{
			delete[] Palette; // Clean up!
			LogManager::getSingleton().logMessage("Invalid palette type specified!");
			return (RGBQUAD*)ColorTable;
		}
		
		// We're sure that we have external palette here:
		mPaletteFile->readStruct(&PaletteHeader, ExternalPaletteHeader_Format, sizeof(ExternalPaletteHeader));
		if (PaletteHeader.RiffSig == 0x46464952)
		{
			if (PaletteHeader.PSig1 != 0x204C4150)
			{
				delete []Palette;
				return NULL;
			}
			mPaletteFile->seek(2L, StdFile::FSEEK_CUR);
			mPaletteFile->readElem(&Count, 2);
			if (Count > 256)
				Count = 256;
			for (I = 0; I < Count; I++)
			{
				mPaletteFile->readStruct(&Palette[I], RGBQUAD_Format, sizeof(RGBQUAD));
				S = Palette[I].rgbRed;
				Palette[I].rgbRed = Palette[I].rgbBlue;
				Palette[I].rgbBlue = S;
			}
		}
		else if (PaletteHeader.RiffSig == 0x4353414A)
		{
			mPaletteFile->seek(0);
			Buffer = new char[3360];
			if (!Buffer)
			{
				delete []Palette;
				return NULL;
			}
			mPaletteFile->read(Buffer, 3352);
			if (strncmp(Buffer, "JASC-PAL", 8))
			{
				delete []Buffer;
				delete []Palette;
				return NULL;
			}
			C = strchr(Buffer, '\n')+1;
			C = strchr(C, '\n')+1;
			Count = (WORD)strtoul(C, NULL, 10);
			if (Count > 256)
				Count = 256;
			for (I = 0; I < Count; I++)
			{
				C = strchr(C, '\n')+1;
				Palette[I].rgbRed = (BYTE)strtoul(C, &C, 10);
				C++;
				Palette[I].rgbGreen = (BYTE)strtoul(C, &C, 10);
				C++;
				Palette[I].rgbBlue = (BYTE)strtoul(C, &C, 10);
			}
			delete []Buffer;
		}
		else
		{
			mPaletteFile->seek(0);
			RGBTRIPLE P;
			for (I = 0; I < mPaletteFile->size() / sizeof(RGBTRIPLE); I++)
			{
				mPaletteFile->readStruct(&P, RGBTRIPLE_Format, sizeof(RGBTRIPLE));
				Palette[I].rgbRed = P.rgbtBlue;
				Palette[I].rgbGreen = P.rgbtGreen;
				Palette[I].rgbBlue = P.rgbtRed;
			}
		}
		return Palette;
		*/
	}

	//------------------------------------------------------
	DrawSourcePtr DrawService::createDrawSource(const std::string& img, const std::string& group) {

		DrawSourcePtr ds = new DrawSource();

		// First we load the image.
		TexturePtr tex = Ogre::TextureManager::getSingleton().load(img, group, TEX_TYPE_2D, 1);

		MaterialPtr mat = Ogre::MaterialManager::getSingleton().create(img, group);
		TextureUnitState* tus = mat->getTechnique(0)->getPass(0)->createTextureUnitState(tex->getName());

		tus->setTextureFiltering(Ogre::FO_NONE, Ogre::FO_NONE, Ogre::FO_NONE);

		Pass *pass = mat->getTechnique(0)->getPass(0);
		pass->setAlphaRejectSettings(CMPF_GREATER, 128);
		pass->setLightingEnabled(false);

		mat->load();

		ds->sourceID = mDrawSourceID++;
		ds->material = mat;
		ds->texture = tex;
		ds->pixelSize.width  = tex->getWidth();
		ds->pixelSize.height = tex->getHeight();
		ds->size = Vector2(1.0f, 1.0f);
		ds->displacement = Vector2(0, 0);

		return ds;
	}

	//------------------------------------------------------
	RenderedImage* DrawService::createRenderedImage(DrawSourcePtr& draw) {
		DrawOperation::ID opID = getNewDrawOperationID();
		RenderedImage* ri = new RenderedImage(this, opID, draw);

		// register so we'll be able to remove it
		mDrawOperations[opID] = ri;

		return ri;
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
