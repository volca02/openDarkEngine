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


#include "DrawService.h"
#include "OpdeException.h"
#include "ServiceCommon.h"

using namespace std;

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
