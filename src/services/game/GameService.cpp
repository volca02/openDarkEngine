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
#include "BinaryService.h"
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
		if (mCurDB != NULL) // TODO: some consistent indicator would be better here
			unload();

		// TODO: Here, a query to mission database would be nice instead of direct file opening
		mCurDB = getDBFileNamed(filename);
		
		// Also, assign parents for the file group (MIS+GAM file for Savegames, GAM for mission files)
		assignDBParents(mCurDB);
		
		WorldRepService* wr_s = static_cast<WorldRepService*>(mServiceManager->getService("WorldRepService"));
			
		// Now call all the services to load
		wr_s->loadFromDarkDatabase(mCurDB);
	}
	
	//------------------------------------------------------
	DarkFileGroup* GameService::getDBFileNamed(const std::string& filename) {
		return new DarkFileGroup(new StdFile(filename, File::FILE_R));
	}
	
	//------------------------------------------------------
	void GameService::unload() {
		WorldRepService* wr_s = static_cast<WorldRepService*>(mServiceManager->getService("WorldRepService"));
		
		wr_s->unload();
		
		/// Wipe out the files we used
		if (mCurDB)
			mCurDB->release();
		
		mCurDB = NULL;
	}
	
	//------------------------------------------------------
	void GameService::assignDBParents(DarkFileGroup* db) {
		// detect the DB type
		uint32 ftype;
		
		File* ft = db->getFile("FILE_TYPE");
		
		ft->readElem(&ftype, sizeof(uint32));
		
		ft->release();
		
		db->addRef();
		
		bool loadGameSys = false;
		bool loadMission = false;
		
		// now, we detect the file type by looking at the uint32 we read
		switch (ftype) {
			// TODO: Find a suitable solution for this. Do not include those numbers directly
			// The ftype seems to be a bitmap of stored info in the file
			case 0x00031900: // MIS
				db->setType(DFG_MISSION);
				_loadGameSysDB(db);
				break;
			
			case 0x00011900: // SAV
				db->setType(DFG_SAVEGAME);
				_loadMissionDB(db);
				break;
			
			default:
				db->release();
				OPDE_EXCEPT("Invalid game database type by FILE_TYPE", "GameService::assignDBParents");
		}
		
		// Okay, all DBs should be loaded
	}
	
	//------------------------------------------------------
	void GameService::_loadMissionDB(DarkFileGroup* db) {
		// TODO: One of the game specific things, SS2 seems not to handle savegames this way
		// Actually, this code is rather a quick hack to let me load all databases I need
		// FileGroup should get a getParent/setParent and getType/setType rather
		
		// DARKMISS chunk contains the mission name (without the .mis extension)
		// This is better handled with binary service
		BinaryService* bs = static_cast<BinaryService*>(mServiceManager->getService("BinaryService"));
		
		DTypeDef* dm = bs->getType("chunks", "DARKMISS");
		
		File* fdm = db->getFile("DARKMISS");
		
		char* data = new char[dm->size()];
		
		DVariant val = dm->get(data, "path");
		
		DarkFileGroup* mis = getDBFileNamed(val.toString() + ".mis");
		
		mis->setType(DFG_MISSION);
		// now read the mission file for the savegame
		db->setParent(mis);
		
		_loadGameSysDB(mis);
		
		delete data;
		
		
		mis->release();
		dm->release();
		fdm->release();
	}
	
	//------------------------------------------------------
	void GameService::_loadGameSysDB(DarkFileGroup* db) {
		// GAM_FILE
		File* fdm = db->getFile("GAM_FILE");
		
		char* data = new char[fdm->size()];
		
		data[0] = 0x0;
		
		fdm->read(data, fdm->size());
		
		DarkFileGroup* gs = getDBFileNamed(data);
		
		gs->setType(DFG_GAMESYS);
		
		// now read the mission file for the savegame
		db->setParent(gs);
		
		gs->release();
		
		delete data;
		
		fdm->release();	
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
