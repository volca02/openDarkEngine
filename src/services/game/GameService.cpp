/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
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
 *****************************************************************************/

 
#include "GameService.h"
#include "WorldRepService.h"
#include "BinaryService.h"
#include "LinkService.h"
#include "OpdeException.h"

using namespace std;

namespace Opde {
	
	/*----------------------------------------------------*/
	/*-------------------- GameService -------------------*/
	/*----------------------------------------------------*/
	GameService::GameService(ServiceManager *manager) : Service(manager), mCurDB(NULL) {
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
		mCurDB = getDBFileNamed(filename);
		
		_loadMissionDB(mCurDB);
	}
	
	//------------------------------------------------------
	DarkFileGroup* GameService::getDBFileNamed(const std::string& filename) {
		FilePtr fp = FilePtr(new StdFile(filename, File::FILE_R));
		
		return new DarkFileGroup(fp);
	}
	
	//------------------------------------------------------
	void GameService::unload() {
		// TODO: Just submit a message to all services having LOADER flag, once the flag system is implemented
		WorldRepServicePtr wr_s = mServiceManager->getService("WorldRepService").as<WorldRepService>();
		
		wr_s->unload();
		
		/// Wipe out the files we used
		if (mCurDB)
			mCurDB->release();
		
		mCurDB = NULL;
	}
	
	//------------------------------------------------------
	void GameService::_loadMissionDB(DarkFileGroup* db) {
		/* Game Save:
		// DARKMISS chunk contains the mission name (without the .mis extension)
		// This is better handled with binary service
		
		BinaryService* bs = static_cast<BinaryService*>(mServiceManager->getService("BinaryService"));
		
		DTypeDef* dm = bs->getType("chunks", "DARKMISS");
		
		FilePtr fdm = db->getFile("DARKMISS");
		
		char* data = new char[dm->size()];
		
		DVariant val = dm->get(data, "path");
		
		DarkFileGroup* mis = getDBFileNamed(val.toString() + ".mis");
		*/
		
		_loadGameSysDB(db);
		
		// Load the Mission
		
		
		// Get the link service
		LinkServicePtr ls = mServiceManager->getService("LinkService").as<LinkService>();
		
		// Get the world geometry service
		WorldRepServicePtr wr_s = mServiceManager->getService("WorldRepService").as<WorldRepService>();
			
		// Now call all the services to load
		wr_s->loadFromDarkDatabase(db);
		ls->load(db);
	}
	
	//------------------------------------------------------
	void GameService::_loadGameSysDB(DarkFileGroup* db) {
		// GAM_FILE
		FilePtr fdm = db->getFile("GAM_FILE");
		
		char* data = new char[fdm->size()];
		
		data[0] = 0x0;
		
		fdm->read(data, fdm->size());
		
		DarkFileGroup* gs = getDBFileNamed(data);
		
		// load the gamesys
		// Get the link service
		LinkServicePtr ls = mServiceManager->getService("LinkService").as<LinkService>();
		
		// Now call all the services to load
		ls->load(gs);
		
		gs->release();
		
		/*
		// Debug test. Save the Link DB
		DarkFileGroup* tdb = new DarkFileGroup();
		ls->save(tdb, 1); // only gamesys data
		
		FilePtr tf = new StdFile("dbgtest.vbr", File::FILE_W);
		
		tdb->write(tf);
		delete tdb;

		
		delete data;
		*/
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
