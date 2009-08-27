/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
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


#include "RoomService.h"
#include "OpdeException.h"
#include "ServiceCommon.h"
#include "logger.h"

using namespace std;

namespace Opde {

	/*----------------------------------------------------*/
	/*-------------------- RoomService -------------------*/
	/*----------------------------------------------------*/
	template<> const size_t ServiceImpl<RoomService>::SID = __SERVICE_ID_ROOM;
	
	RoomService::RoomService(ServiceManager *manager, const std::string& name) : ServiceImpl< Opde::RoomService >(manager, name) {
	}

	//------------------------------------------------------
	RoomService::~RoomService() {
	}


	//------------------------------------------------------
	bool RoomService::init() {
		return true;
	}
	
	
	//------------------------------------------------------
	void RoomService::bootstrapFinished() {
		mDbService = GET_SERVICE(DatabaseService);
		mDbService->registerListener(this, DBP_ROOM);
	}
	
	//------------------------------------------------------
	void RoomService::shutdown() {
		mDbService->unregisterListener(this);
		mDbService.setNull();
	}
	
	//------------------------------------------------------
	void RoomService::onDBLoad(const FileGroupPtr& db, uint32_t curmask) {
		LOG_INFO("RoomService::onDBLoad called.");
		// STUB
	}
	
	//------------------------------------------------------
	void RoomService::onDBSave(const FileGroupPtr& db, uint32_t tgtmask) {
		LOG_INFO("RoomService::onDBSave called.");
		// STUB
	}
	
	//------------------------------------------------------
	void RoomService::onDBDrop(uint32_t dropmask) {
		LOG_INFO("RoomService::onDBDrop called.");
		// STUB
	}

	//-------------------------- Factory implementation
	std::string RoomServiceFactory::mName = "RoomService";

	RoomServiceFactory::RoomServiceFactory() : ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
	};

	const std::string& RoomServiceFactory::getName() {
		return mName;
	}

	const uint RoomServiceFactory::getMask() {
		return SERVICE_ENGINE;
	}

	const size_t RoomServiceFactory::getSID() {
		return RoomService::SID;
	}

	Service* RoomServiceFactory::createInstance(ServiceManager* manager) {
		return new RoomService(manager, mName);
	}

}
