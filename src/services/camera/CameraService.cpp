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

#include "CameraService.h"
#include "logger.h"
#include "ServiceCommon.h"

using namespace std;
using namespace Ogre;

namespace Opde {

	/*----------------------------------------------------*/
	/*-------------------- CameraService -----------------*/
	/*----------------------------------------------------*/
	template<> const size_t ServiceImpl<CameraService>::SID = __SERVICE_ID_CAMERA;
	
	CameraService::CameraService(ServiceManager *manager, const std::string& name) : ServiceImpl<CameraService>(manager, name) {
	}
	
	//------------------------------------------------------
	CameraService::~CameraService() {
	}
	
	//------------------------------------------------------
	bool CameraService::staticAttach(int objID) {
		// TODO: Code
		return false;
	}
	
	//------------------------------------------------------
	bool CameraService::dynamicAttach(int objID) {
		// TODO: Code
		return false;
	}
	
	//------------------------------------------------------
	bool CameraService::cameraReturn(int objID) {
		// TODO: Code
		return false;
	}
	
	//------------------------------------------------------
	void CameraService::forceCameraReturn() {
		// TODO: Code
	}
	
	//------------------------------------------------------
	bool CameraService::init() {
		return true;
	}
	
	//-------------------------- Factory implementation
	std::string CameraServiceFactory::mName = "CameraService";

	CameraServiceFactory::CameraServiceFactory() : ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
	};

	const std::string& CameraServiceFactory::getName() {
		return mName;
	}

	const uint CameraServiceFactory::getMask() {
		return SERVICE_RENDERER;
	}
	
	const size_t CameraServiceFactory::getSID() {
		return CameraService::SID;
	}

	Service* CameraServiceFactory::createInstance(ServiceManager* manager) {
		return new CameraService(manager, mName);
	}

}
