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
		
		mMyFonts.clear();
	}

	//------------------------------------------------------
	DrawSourcePtr TextureAtlas::createDrawSource(const Ogre::String& imgName, const Ogre::String& groupName) {
		// Load as single first, but wit the same id
		// First we load the image.
		DrawSourcePtr ds = new DrawSource();

		ds->image.load(imgName, groupName);

		ds->sourceID = mAtlasID;
		ds->material.setNull();
		ds->texture.setNull();
		ds->pixelSize.width  = ds->image.getWidth();
		ds->pixelSize.height = ds->image.getHeight();
		ds->size = Ogre::Vector2(1.0f, 1.0f);
		ds->displacement = Ogre::Vector2(0, 0);

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
	void TextureAtlas::_addDrawSource(DrawSourcePtr& ds) {
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

		// First, we sort by size of the DrawSource
		mMyDrawSources.sort(DrawSourceLess());

		// build the fonts (if this didn't happen already)
		// so we'll be sure the glyphs are there to be atlassed
		FontSet::iterator fit = mMyFonts.begin();

		while (fit != mMyFonts.end()) {
			FontDrawSource* fdsp = *fit++;

			if (!fdsp->isBuilt())
				fdsp->build();
		}

		// now try to allocate all the draw sources. If we fail, grow and try again
		do {
			fitted = true;
			area = 0;

			// try to fit
			DrawSourceSet::iterator it = mMyDrawSources.begin();

			while (it != mMyDrawSources.end()) {
				DrawSourcePtr ds = *it++;

				area += ds->pixelSize.width * ds->pixelSize.height;

				// try to allocate
				FreeSpaceInfo* area = mAtlasAllocation->allocate(ds->pixelSize.width, ds->pixelSize.height);

				if (area) {
					ds->placement = area;
				} else {
					fitted = false;
					break;
				}
			}
			// fitted?

			if (!fitted) // nope - Enlarge!
				enlarge(area);
		} while (!fitted);

		// it seems we're here because we fitted! Lets paint the textures into the atlas
		mAtlasTexture = Ogre::TextureManager::getSingleton().createManual(mAtlasName,
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			Ogre::TEX_TYPE_2D,
			mAtlasSize.width,
			mAtlasSize.height,
			0,
			Ogre::PF_BYTE_BGRA,
			Ogre::TU_STATIC_WRITE_ONLY);

		mAtlasMaterial = Ogre::MaterialManager::getSingleton().create("M_" + mAtlasName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		Ogre::TextureUnitState* tus = mAtlasMaterial->getTechnique(0)->getPass(0)->createTextureUnitState(mAtlasTexture->getName());

		tus->setTextureFiltering(Ogre::FO_NONE, Ogre::FO_NONE, Ogre::FO_NONE);

		Ogre::Pass *pass = mAtlasMaterial->getTechnique(0)->getPass(0);
		pass->setAlphaRejectSettings(Ogre::CMPF_GREATER, 128);
		pass->setLightingEnabled(false);


		 Ogre::HardwarePixelBufferSharedPtr pixelBuffer = mAtlasTexture->getBuffer();
		 pixelBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD);
		 const Ogre::PixelBox& targetBox = pixelBuffer->getCurrentLock();
		 size_t pixelsize = Ogre::PixelUtil::getNumElemBytes(targetBox.format);
		 size_t rowsize = targetBox.rowPitch * pixelsize;

		 Ogre::uint8* dstData = static_cast<Ogre::uint8*>(targetBox.data);

		 // We'll iterate over all draw sources, painting the pixels onto the allocated space
		 DrawSourceSet::iterator it = mMyDrawSources.begin();

		while (it != mMyDrawSources.end()) {
			DrawSourcePtr ds = *it++;

			ds->material = mAtlasMaterial;

			// render all pixels into the right place
			FreeSpaceInfo* fsi = reinterpret_cast<FreeSpaceInfo*>(ds->placement);

			// render into the specified place
			unsigned char* conversionBuf = NULL;
			Ogre::PixelBox srcPixels = ds->image.getPixelBox();

			// convert if the source data don't match
			if(ds->image.getFormat() != Ogre::PF_BYTE_BGRA) {
					conversionBuf = new unsigned char[ds->texture->getWidth() * ds->texture->getHeight() * pixelsize];
					Ogre::PixelBox convPixels(Ogre::Box(0, 0, ds->pixelSize.width, ds->pixelSize.height), Ogre::PF_BYTE_BGRA, conversionBuf);
					Ogre::PixelUtil::bulkPixelConversion(srcPixels, convPixels);
					srcPixels = convPixels;
			}

			size_t srcrowsize = srcPixels.rowPitch * pixelsize;

			Ogre::uint8* srcData = static_cast<Ogre::uint8*>(srcPixels.data);

			for(size_t row = 0; row < ds->pixelSize.height; row++) {
					for(size_t col = 0; col < srcrowsize; col++) {
							dstData[((row + fsi->y) * rowsize) + (fsi->x * pixelsize) + col] =
								srcData[(row * srcrowsize) + col];
					}
			}

			delete[] conversionBuf;

			// TODO: This needs rewrite. Multiple builds on the same images and !bam! the coordinates are lost
			// convert the full texturing coords to the atlassed ones
			ds->displacement = Ogre::Vector2((Ogre::Real)fsi->x / mAtlasSize.width, (Ogre::Real)fsi->y / mAtlasSize.height);
			ds->size = Ogre::Vector2((Ogre::Real)ds->pixelSize.width / mAtlasSize.width, (Ogre::Real)ds->pixelSize.height / mAtlasSize.height);

		}

		 pixelBuffer->unlock();
	}

	//------------------------------------------------------
	void TextureAtlas::enlarge(size_t area) {
		// TODO: destroy the prev. texture.
		// Ogre::TextureManager::getSingleton().unload(mAtlasTexture);

		// mark dirty the atlas
		markDirty();

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
