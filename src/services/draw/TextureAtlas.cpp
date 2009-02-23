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

// Portions inspired by ajs' Atlas code

#include "TextureAtlas.h"
#include "DrawService.h"
#include "FontDrawSource.h"
#include "logger.h"

#include <OgreTextureManager.h>
#include <OgreStringConverter.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreTechnique.h>
#include <OgreMaterialManager.h>

#include <list>

namespace Opde {

	/*----------------------------------------------------*/
	/*-------------------- TextureAtlas ------------------*/
	/*----------------------------------------------------*/
	TextureAtlas::TextureAtlas(DrawService* owner, DrawSource::ID id) :
			DrawSourceBase(),
			mOwner(owner),
			mAtlasID(id),
			mMyDrawSources(),
			mIsDirty(false),
			mAtlasSize(1,1) {

		mAtlasAllocation = new FreeSpaceInfo(0,0,1,1);
		mAtlasName = "DrawAtlas" + Ogre::StringConverter::toString(mAtlasID);
	}

	//------------------------------------------------------
	TextureAtlas::~TextureAtlas() {
		// release all the draw sources
		mMyDrawSources.clear();

		// and get rid of the allocation info too
		delete mAtlasAllocation;
		
		FontSet::iterator fit = mMyFonts.begin();
		FontSet::iterator fend = mMyFonts.end();
		
		while (fit != fend) {
			delete *fit++;
		}
		
		DrawSourceSet::iterator dit = mMyDrawSources.begin();
		DrawSourceSet::iterator dend = mMyDrawSources.end();
		
		while (dit != dend) {
			delete *dit++;
		}
		
		mMyDrawSources.clear();
	}

	//------------------------------------------------------
	DrawSource* TextureAtlas::createDrawSource(const Ogre::String& imgName, const Ogre::String& groupName) {
		// Load as single first, but wit the same id
		// First we load the image.
		DrawSource* ds = new DrawSource();

		ds->loadImage(imgName, groupName);

		ds->setSourceID(mAtlasID);

		mMyDrawSources.push_back(ds);
		markDirty();

		return ds;
	}


	//------------------------------------------------------
	FontDrawSource* TextureAtlas::createFont(const std::string& name) {
		// creates a new font instance to be filled with glyphs
		FontDrawSource* fdsp = new FontDrawSource(this, name);

		mMyFonts.push_back(fdsp);
		markDirty();

		return fdsp;
	}

	//------------------------------------------------------
	void TextureAtlas::_addDrawSource(DrawSource* ds) {
		// just insert into the list
		mMyDrawSources.push_back(ds);
		markDirty();
	}

	//------------------------------------------------------
	void TextureAtlas::build() {
		if (!mIsDirty)
			return;

		bool fitted;

		size_t area = 0;

		// build the fonts (if this didn't happen already)
		// so we'll be sure the glyphs are there to be atlassed
		FontSet::iterator fit = mMyFonts.begin();

		while (fit != mMyFonts.end()) {
			FontDrawSource* fdsp = *fit++;

			if (!fdsp->isBuilt())
				fdsp->build();
		}

		// First, we sort by size of the DrawSource
		mMyDrawSources.sort(DrawSourceLess());

		// now try to allocate all the draw sources. If we fail, grow and try again
		do {
			fitted = true;
			area = 0;

			// try to fit
			DrawSourceSet::iterator it = mMyDrawSources.begin();

			while (it != mMyDrawSources.end()) {
				
				DrawSource* ds = *it++;

				const PixelSize& ps = ds->getPixelSize();
				area += ps.getPixelArea();
				
				LOG_DEBUG("TextureAtlas: Trying to place %d x %d (%d -> %d)", ps.width, ps.height, ps.getPixelArea(), area);

				// try to allocate
				FreeSpaceInfo* fsi = mAtlasAllocation->allocate(ps.width, ps.height);

				if (fsi) {
					ds->setPlacementPtr(fsi);
				} else {
					fitted = false;
					break;
				}
			}
			// fitted?

			if (!fitted) // nope - Enlarge!
				enlarge(area);
		} while (!fitted);
		
		LOG_INFO("TextureAtlas: Creating atlas '%s' with dimensions %d x %d", mAtlasName.c_str(), mAtlasSize.width, mAtlasSize.height);

		// it seems we're here because we fitted! Lets paint the textures into the atlas
		mTexture = Ogre::TextureManager::getSingleton().createManual(mAtlasName,
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			Ogre::TEX_TYPE_2D,
			mAtlasSize.width,
			mAtlasSize.height,
			0,
			Ogre::PF_BYTE_BGRA,
			Ogre::TU_STATIC_WRITE_ONLY);

		mMaterial = Ogre::MaterialManager::getSingleton().create("M_" + mAtlasName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		Ogre::TextureUnitState* tus = mMaterial->getTechnique(0)->getPass(0)->createTextureUnitState(mTexture->getName());

		tus->setTextureFiltering(Ogre::FO_NONE, Ogre::FO_NONE, Ogre::FO_NONE);

		Ogre::Pass *pass = mMaterial->getTechnique(0)->getPass(0);
		pass->setAlphaRejectSettings(Ogre::CMPF_GREATER, 128);
		pass->setLightingEnabled(false);


		 Ogre::HardwarePixelBufferSharedPtr pixelBuffer = mTexture->getBuffer();
		 pixelBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD);
		 const Ogre::PixelBox& targetBox = pixelBuffer->getCurrentLock();
		 size_t pixelsize = Ogre::PixelUtil::getNumElemBytes(targetBox.format);
		 size_t rowsize = targetBox.rowPitch * pixelsize;

		 Ogre::uint8* dstData = static_cast<Ogre::uint8*>(targetBox.data);

		 // We'll iterate over all draw sources, painting the pixels onto the allocated space
		 DrawSourceSet::iterator it = mMyDrawSources.begin();

		while (it != mMyDrawSources.end()) {
			DrawSource* ds = *it++;

			// render all pixels into the right place
			FreeSpaceInfo* fsi = reinterpret_cast<FreeSpaceInfo*>(ds->getPlacementPtr());

			// render into the specified place
			unsigned char* conversionBuf = NULL;
			

			const PixelSize& dps = ds->getPixelSize();
			Ogre::Image& img = ds->getImage();
			Ogre::PixelBox srcPixels = img.getPixelBox();
			
			// convert if the source data don't match
			if(img.getFormat() != Ogre::PF_BYTE_BGRA) {
					conversionBuf = new unsigned char[img.getWidth() * img.getHeight() * pixelsize];
					Ogre::PixelBox convPixels(Ogre::Box(0, 0, dps.width, dps.height), Ogre::PF_BYTE_BGRA, conversionBuf);
					Ogre::PixelUtil::bulkPixelConversion(srcPixels, convPixels);
					srcPixels = convPixels;
			}

			size_t srcrowsize = srcPixels.rowPitch * pixelsize;

			Ogre::uint8* srcData = static_cast<Ogre::uint8*>(srcPixels.data);
			
			for(size_t row = 0; row < dps.height; row++) {
					for(size_t col = 0; col < srcrowsize; col++) {
							dstData[((row + fsi->y) * rowsize) + (fsi->x * pixelsize) + col] =
								srcData[(row * srcrowsize) + col];
					}
			}

			delete[] conversionBuf;

			// TODO: This needs rewrite. Multiple builds on the same images and !bam! the coordinates are lost
			// convert the full texturing coords to the atlassed ones
			ds->atlas(mMaterial, fsi->x, fsi->y, mAtlasSize.width, mAtlasSize.height);
		}


		 // for debug, write the texture to a file
		 
		unsigned char *readrefdata = static_cast<unsigned char*>(targetBox.data);		
				     
		Ogre::Image img;
		img = img.loadDynamicImage (readrefdata, mTexture->getWidth(),
		    mTexture->getHeight(), mTexture->getFormat());	
		img.save(mAtlasName + ".png");
		  
		pixelBuffer->unlock();
	}

	//------------------------------------------------------
	void TextureAtlas::enlarge(size_t area) {
		// TODO: destroy the prev. texture.
		// Ogre::TextureManager::getSingleton().unload(mAtlasTexture);

		// mark dirty the atlas
		markDirty();
		
		LOG_DEBUG("TextureAtlas: Enlarging atlas from %d x %d", mAtlasSize.width, mAtlasSize.height);

		size_t size;

		// a trick to note - always enlarged at least once, but can be more times to fit at least the area specified
		do {
			// enlarge
			if (mAtlasSize.height > mAtlasSize.width)
				mAtlasSize.width  *= 2;
			else
				mAtlasSize.height *= 2;

			size = mAtlasSize.width * mAtlasSize.height;
		} while (size < area);


		LOG_DEBUG("TextureAtlas: Enlarged atlas to %d x %d", mAtlasSize.width, mAtlasSize.height);

		delete mAtlasAllocation;
		mAtlasAllocation = new FreeSpaceInfo(0,0,mAtlasSize.width, mAtlasSize.height);
	}

	//------------------------------------------------------
	void TextureAtlas::markDirty() {
		mIsDirty = true;

		// queue this atlas for automatic rebuilding
		mOwner->_queueAtlasForRebuild(this);
	}
};
