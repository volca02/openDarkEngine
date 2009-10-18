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
 *
 *		$Id$
 *
 *****************************************************************************/


#ifndef __CAMERASERVICE_H
#define __CAMERASERVICE_H

#include "config.h"

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "RenderService.h"
#include "SharedPtr.h"

namespace Opde {

	/** @brief camera service. Service that handles in-game camera
	*/
	class OPDELIB_EXPORT CameraService : public ServiceImpl<CameraService> {
		public:
			CameraService(ServiceManager *manager, const std::string& name);
			virtual ~CameraService();
			
			/** Attaches camera to an object. Does not allow freelook
			 * @return true on success, false on failure */
			bool staticAttach(int objID);
			
			/** Attaches camera to an object with freelook allowed 
			  * @return true on success, false on failure */
			bool dynamicAttach(int objID);
			
			/** Returns (conditionally) the camera to the player object. 
			 * Camera is only returned if it is currently attached to the object specified by the parameter 
			 * @param curObjID The current object to which camera is attached 
			 * @return true on success, false on failure */
			bool cameraReturn(int objID);
			
			/** Returns (unconditionally) the camera to the player object. */
			void forceCameraReturn();
			
		protected:
			bool init();
	};

	/// Shared pointer to Camera service
	typedef shared_ptr<CameraService> CameraServicePtr;


	/// Factory for the CameraService objects
	class OPDELIB_EXPORT CameraServiceFactory : public ServiceFactory {
		public:
			CameraServiceFactory();
			~CameraServiceFactory() {};

			/** Creates a CameraService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

			virtual const uint getMask();
			
			virtual const size_t getSID();
		private:
			static std::string mName;
	};
}


#endif
