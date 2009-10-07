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
		mMaterial = Ogre::MaterialManager::getSingleton().create("M_" + mAtlasName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		
		// Here, we insert the white 2x2 texture into the draw sources. It is always there,
		// so we have a direct pointer to it in the atlas as well.
		// it is used for texture-less objects (Governed by vertex colour alone)
		// Inspiration was taken from Canvas, as with other things
		// image:
		uint32_t* pixels = new uint32_t[4];
		for (size_t i = 0; i < 4; i++)
			pixels[i] = 0x0ffffffff; // a white colour, that is
		
		// draw source:
		mVertexColour = DrawSourcePtr(new DrawSource(mOwner));
		mVertexColour->getImage()->loadDynamicImage(reinterpret_cast<Ogre::uchar*>(pixels), 2, 2, 1, Ogre::PF_BYTE_BGRA, false);
		mVertexColour->setSourcePixmapPointer(pixels);
		mVertexColour->updatePixelSizeFromImage();
		mVertexColour->setSourceID(mAtlasID);
		
		// register:
		_addDrawSource(mVertexColour);
	}

	//------------------------------------------------------
	TextureAtlas::~TextureAtlas() {
		// release all the draw sources
		mVertexColour.setNull();
		mMyDrawSources.clear();

		// and get rid of the allocation info too
		delete mAtlasAllocation;
		

		mMyFonts.clear();
		
		DrawSourceList::iterator dit = mMyDrawSources.begin();
		DrawSourceList::iterator dend = mMyDrawSources.end();
		
		while (dit != dend) {
			mOwner->unregisterDrawSource(*dit++);
		}
		
		mMyDrawSources.clear();
		
		dropResources();
		
		Ogre::MaterialManager::getSingleton().remove(mMaterial->getName());
	}

	//------------------------------------------------------
	DrawSourcePtr TextureAtlas::createDrawSource(const Ogre::String& imgName, const Ogre::String& groupName) {
		// Load as single first, but wit the same id
		// First we load the image.
		DrawSourcePtr ds(new DrawSource(mOwner, imgName, groupName, mMaterial));

		ds->setSourceID(mAtlasID);

		_addDrawSource(ds);

		// register the draw source
		mOwner->registerDrawSource(ds, imgName, groupName);

		return ds;
	}


	 
	
	//------------------------------------------------------
	void TextureAtlas::_addFont(FontDrawSource* fdsp) {
		mMyFonts.push_back(fdsp);
		markDirty();
	}
	
	//------------------------------------------------------
	void TextureAtlas::_removeFont(FontDrawSource* fdsp) {
		mMyFonts.remove(fdsp);
		markDirty();
	}

	//------------------------------------------------------
	void TextureAtlas::_addDrawSource(const DrawSourcePtr& ds) {
		// just insert into the list
		mMyDrawSources.push_back(ds);
		markDirty();
	}

	//------------------------------------------------------
	void TextureAtlas::_removeDrawSource(const DrawSourcePtr& ds) {
		// just insert into the list
		mMyDrawSources.remove(ds);
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
			DrawSourceList::iterator it = mMyDrawSources.begin();

			while (it != mMyDrawSources.end()) {
				
				const DrawSourcePtr& ds = *it++;

				const PixelSize& ps = ds->getPixelSize();
				area += ps.getPixelArea();
				
				LOG_VERBOSE("TextureAtlas: (%s) Trying to place %d x %d (%d -> %d)", mAtlasName.c_str(), ps.width, ps.height, ps.getPixelArea(), area);

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
		
		LOG_INFO("TextureAtlas: (%s) Creating atlas with dimensions %d x %d", mAtlasName.c_str(), mAtlasSize.width, mAtlasSize.height);

		if (mTexture.isNull())
			prepareResources();
		// TODO: Reallocate the texture here if needed!
		
		Ogre::HardwarePixelBufferSharedPtr pixelBuffer = mTexture->getBuffer();
		pixelBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD);
		
		const Ogre::PixelBox& targetBox = pixelBuffer->getCurrentLock();
		
		size_t pixelsize = Ogre::PixelUtil::getNumElemBytes(targetBox.format);
		size_t rowsize = targetBox.rowPitch * pixelsize;

		Ogre::uint8* dstData = static_cast<Ogre::uint8*>(targetBox.data);

		// We'll iterate over all draw sources, painting the pixels onto the allocated space
		DrawSourceList::iterator it = mMyDrawSources.begin();

		while (it != mMyDrawSources.end()) {
			const DrawSourcePtr& ds = *it++;

			// render all pixels into the right place
			FreeSpaceInfo* fsi = reinterpret_cast<FreeSpaceInfo*>(ds->getPlacementPtr());
			
			assert(fsi);

			// render into the specified place
			unsigned char* conversionBuf = NULL;
			

			const PixelSize& dps = ds->getPixelSize();
			Ogre::Image* img = ds->getImage();
			Ogre::PixelBox srcPixels = img->getPixelBox();
			
			// convert if the source data don't match
			if(img->getFormat() != Ogre::PF_BYTE_BGRA) {
					conversionBuf = new unsigned char[img->getWidth() * img->getHeight() * pixelsize];
					Ogre::PixelBox convPixels(Ogre::Box(0, 0, dps.width, dps.height), Ogre::PF_BYTE_BGRA, conversionBuf);
					Ogre::PixelUtil::bulkPixelConversion(srcPixels, convPixels);
					srcPixels = convPixels;
			}

			size_t srcrowsize = srcPixels.rowPitch * pixelsize;

			Ogre::uint8* srcData = static_cast<Ogre::uint8*>(srcPixels.data);
			
			// TODO: we're always handling 32bit data, so we could as well transfer 4 bytes each iteration instead of one (speedup)
			for(size_t row = 0; row < dps.height; row++) {
					for(size_t col = 0; col < srcrowsize; col++) {
							dstData[((row + fsi->y) * rowsize) + (fsi->x * pixelsize) + col] =
								srcData[(row * srcrowsize) + col];
					}
			}

			delete[] conversionBuf;
			
			// Convert the full draw source pixel coordinates to the atlas contained ones (initializes the texturing coordinates transform)
			ds->atlas(mMaterial, fsi->x, fsi->y, mAtlasSize.width, mAtlasSize.height);
		}

		 // for debug, write the texture to a file
		/*unsigned char *readrefdata = static_cast<unsigned char*>(targetBox.data);		
				     
		Ogre::Image img;
		img = img.loadDynamicImage (readrefdata, mTexture->getWidth(),
		    mTexture->getHeight(), mTexture->getFormat());	
		img.save(mAtlasName + ".png");*/

		// and close the pixel buffer of the atlas at the end
		pixelBuffer->unlock();
		mIsDirty = false;
	}

	//------------------------------------------------------
	void TextureAtlas::enlarge(size_t area) {
		dropResources();
		
		// TODO: destroy the prev. texture.
		// Ogre::TextureManager::getSingleton().unload(mAtlasTexture);

		// mark dirty the atlas
		markDirty();
		
		LOG_DEBUG("TextureAtlas: (%s) Enlarging atlas from %d x %d", mAtlasName.c_str(), mAtlasSize.width, mAtlasSize.height);

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


		LOG_DEBUG("TextureAtlas: (%s) Enlarged atlas to %d x %d", mAtlasName.c_str(), mAtlasSize.width, mAtlasSize.height);

		delete mAtlasAllocation;
		mAtlasAllocation = new FreeSpaceInfo(0,0,mAtlasSize.width, mAtlasSize.height);
		
		// destroy the old invalid texture
		if (!mTexture.isNull()) {
			Ogre::TextureManager::getSingleton().remove(mTexture->getName());
			mTexture.setNull();
		}
	}

	//------------------------------------------------------
	void TextureAtlas::markDirty() {
		mIsDirty = true;

		// queue this atlas for automatic rebuilding
		mOwner->_queueAtlasForRebuild(this);
	}

	//------------------------------------------------------
	void TextureAtlas::prepareResources() {
		mTexture = Ogre::TextureManager::getSingleton().createManual(mAtlasName,
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			Ogre::TEX_TYPE_2D,
			mAtlasSize.width,
			mAtlasSize.height,
			1,
			Ogre::PF_BYTE_BGRA,
			Ogre::TU_STATIC_WRITE_ONLY);

		Ogre::TextureUnitState* tus = mMaterial->getTechnique(0)->getPass(0)->createTextureUnitState(mTexture->getName());

		tus->setTextureFiltering(Ogre::FO_NONE, Ogre::FO_NONE, Ogre::FO_NONE);

		Ogre::Pass *pass = mMaterial->getTechnique(0)->getPass(0);
		pass->setAlphaRejectSettings(Ogre::CMPF_GREATER, 128);
		pass->setLightingEnabled(false);
	}
	
	//------------------------------------------------------
	void TextureAtlas::dropResources() {
		if (!mTexture.isNull()) {
			Ogre::TextureUnitState* tus = mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(mTexture->getName());
			
			if (tus) {
				int index = mMaterial->getTechnique(0)->getPass(0)->getTextureUnitStateIndex(tus);
				mMaterial->getTechnique(0)->getPass(0)->removeTextureUnitState(index);
			}
			
			Ogre::TextureManager::getSingleton().remove(mTexture->getName());
			mTexture.setNull();
		}
	}

};
