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
 
#ifndef __OPDESERVICEFACTORY_H
#define __OPDESERVICEFACTORY_H

#include "compat.h"
#include <string>
#include "OpdeServiceManager.h"

namespace Opde {

	// Volca: I like the Ogre's approach on factories. This is quite simmilar
	
	// forward declaration
	class ServiceManager;
	class Service;
		
	/** Code base for the service factories. Implement the methods with the Service Factory you're implementing. */
	class ServiceFactory {
		public:
			ServiceFactory() {  };
			virtual ~ServiceFactory() {	};
		
			/** Creates and returns a new instance of the service. 
			* @todo Do I NEED named services? I think not. */
			virtual Service* createInstance(ServiceManager* manager) = 0;
			
			/** Get the name of the created objects. What object this factory creates? */
			virtual const std::string& getName() = 0;
			
			/** Get the mask of the service.
			* Masks are freely chosen bitmaps that describe what the requirements/capabilities of the service are 
			* @returns 0 in this case, to be overriden by the factory implemetation */
			virtual const uint getMask() { return 0; };
			
	};
}


#endif
