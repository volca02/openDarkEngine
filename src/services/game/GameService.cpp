/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2007 openDarkEngine team
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 * http://www.gnu.org/copyleft/lesser.txt.
 *****************************************************************************/
 
#include "GameService.h"
#include "WorldRepService.h"
#include "OpdeException.h"

using namespace std;

namespace Opde {
	
	/*----------------------------------------------------*/
	/*-------------------- GameService -------------------*/
	/*----------------------------------------------------*/
	GameService::GameService(ServiceManager *manager) : Service(manager), mCurDB(NULL) {
		mServiceMgr = ServiceManager::getSingletonPtr();
		
	}
	
	//------------------------------------------------------
	GameService::~GameService() {
		
	}
	
	//------------------------------------------------------
	void GameService::load(const std::string& filename) {
		// if there is another game in progress, release it now
		if (mCurDB != NULL)
			unload();

		// TODO: Here, a query to mission database would be nice instead of direct file opening
		mCurDB = new DarkFileGroup(new StdFile(filename, File::FILE_R));
		
		// Also, assign parents for the file group (MIS file for Savegames, GAM for mission files)
		assignDBParents(mCurDB);
		
		WorldRepService* wr_s = static_cast<WorldRepService*>(mServiceMgr->getService("WorldRepService"));
			
		// Now call all the services to load
		wr_s->loadFromDarkDatabase(mCurDB);
	}
	
	//------------------------------------------------------
	void GameService::unload() {
		WorldRepService* wr_s = static_cast<WorldRepService*>(mServiceMgr->getService("WorldRepService"));
		
		wr_s->unload();
	}
	
	//------------------------------------------------------
	void GameService::assignDBParents(DarkFileGroup* db) {
	}
	
	//-------------------------- Factory implementation
	std::string GameServiceFactory::mName = "GameService";
	
	GameServiceFactory::GameServiceFactory() : ServiceFactory() { 
		ServiceManager::getSingleton().addServiceFactory(this);
	};
	
	const std::string& GameServiceFactory::getName() {
		return mName;
	}
	
	Service* GameServiceFactory::createInstance(ServiceManager* manager) {
		return new GameService(manager);
	}
	
}
