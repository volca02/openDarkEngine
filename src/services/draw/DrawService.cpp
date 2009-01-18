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

#include <OgreTexture.h>
#include <OgreTextureManager.h>
#include <OgreMaterial.h>
#include <OgreMaterialManager.h>

using namespace std;
using namespace Ogre;

namespace Opde {

	/*----------------------------------------------------*/
	/*-------------------- DrawService -------------------*/
	/*----------------------------------------------------*/
	DrawService::DrawService(ServiceManager *manager, const std::string& name) : Service(manager, name),
			mSheetMap(),
			mActiveSheet(NULL) {
	}

	//------------------------------------------------------
	DrawService::~DrawService() {
		// destroy all sheets...
		SheetMap::iterator it = mSheetMap.begin();

		for (; it != mSheetMap.end(); ++it) {
			delete it->second;
		}

		mSheetMap.clear();
	}

	//------------------------------------------------------
	bool DrawService::init() {
		return true;
	}

	//------------------------------------------------------
	void DrawService::bootstrapFinished() {
		//
	}


	//------------------------------------------------------
	DrawSheet* DrawService::createSheet(const std::string& sheetName) {
		assert(!sheetName.empty());

		SheetMap::iterator it = mSheetMap.find(sheetName);

		if (it != mSheetMap.end())
			return it->second;
		else
			return NULL;
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
	DrawSourcePtr& DrawService::createDrawSource(const std::string& img, const std::string& group) {
		/*TODO:
		// First we load the image.
		// TexturePtr tex = Ogre::TextureManager().getSingleton().create(img, group);

		MaterialPtr mat = Ogre::MaterialManager().getSingleton().create(img, group);

		DrawSourcePtr ds = new DrawSource();

		ds->material = mat;
		ds->texture = tex;
		ds->pixelSize.first = tex->getWidth();
		ds->pixelSize.second = tex->getHeight();
		ds->size = Vector2(1.0f, 1.0f);
		ds->displacement = Vector2(0, 0);
		*/
	}

	//------------------------------------------------------
	/* TODO:
	  RenderedImage* DrawService::createRenderedImage(DrawSourcePtr& draw) {
		DrawOperation::ID opID = getNewOperationID();
		RenderedImage* ri = new RenderedImage(opID, draw);

		// register so we'll be able to remove it
		mDrawOperations[opID] = ri;

		return ri;
	}*/

	//------------------------------------------------------
	size_t DrawService::getNewDrawOperationID() {
		// do we have some free id's in the stack?
		if (mFreeIDs.empty()) {
			// raise the id
			mDrawOpID++;
			mDrawOperations.grow(mDrawOpID * 2);
		} else {
			size_t newid = mFreeIDs.top();
			mFreeIDs.pop();
			return newid;
		}
	}

	//------------------------------------------------------
	void DrawService::destroyDrawOperation(DrawOperation* dop) {
		//
		DrawOperation::ID id = dop->getID();
		mDrawOperations[id] = NULL;
		// recycle the id for reuse
		mFreeIDs.push(id);
		delete dop;
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
