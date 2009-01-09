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


#ifndef __DRAWSERVICE_H
#define __DRAWSERVICE_H

#include "config.h"

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "SharedPtr.h"

namespace Opde {

	/** @brief Draw Service - 2D rendering service.
	*/
	class OPDELIB_EXPORT DrawService : public Service {
		public:
			DrawService(ServiceManager *manager, const std::string& name);
			virtual ~DrawService();

		protected:
			bool init();
	};

	/// Shared pointer to the draw service
	typedef shared_ptr<DrawService> DrawServicePtr;


	/// Factory for the DrawService objects
	class OPDELIB_EXPORT DrawServiceFactory : public ServiceFactory {
		public:
			DrawServiceFactory();
			~DrawServiceFactory() {};

			/** Creates a GameService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

			virtual const uint getMask(); 
		private:
			static std::string mName;
	};
}


#endif
