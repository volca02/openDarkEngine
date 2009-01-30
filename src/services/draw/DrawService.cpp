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

	/*----------------------------------------------------*/
	/*-------------------- DrawService -------------------*/
	/*----------------------------------------------------*/
	DrawService::DrawService(ServiceManager *manager, const std::string& name) : Service(manager, name),
			mSheetMap(),
			mActiveSheet(NULL),
			mDrawOpID(0),
			mDrawSourceID(0),
			mViewport(NULL) {
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
		}
	}

	//------------------------------------------------------
	void DrawService::renderQueueEnded(uint8 queueGroupId, const String& invocation, bool& skipThisInvocation) {

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
