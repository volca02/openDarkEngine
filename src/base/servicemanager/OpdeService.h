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
 
#ifndef __OPDESERVICE_H
#define __OPDESERVICE_H
 
#include "NonCopyable.h" 
#include "SharedPtr.h"

namespace Opde {
	 
	// Forward declaration
	class ServiceManager;
	
	/** Interface used for all services. Those must implement the here mentioned methods. */
	class Service : public NonCopyable {
			protected:
				ServiceManager* mServiceManager;
		
			public:
				Service(ServiceManager* manager);
				virtual ~Service();
	};
	
	/// A shared pointer to service
	typedef shared_ptr<Service> ServicePtr;
}
 
 
#endif
