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
 
#ifndef __LINKSERVICE_H
#define __LINKSERVICE_H

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "FileGroup.h"

namespace Opde {
	
	/** @brief Link service - service managing in-game object links
	*/
	class LinkService : public Service {
		public:
			LinkService(ServiceManager *manager);
			virtual ~LinkService();
			
		protected:
			
	};
	
	
	/// Factory for the GameService objects
	class LinkServiceFactory : public ServiceFactory {
		public:
			LinkServiceFactory();
			~LinkServiceFactory() {};
			
			/** Creates a GameService instance */
			Service* createInstance(ServiceManager* manager);
			
			virtual const std::string& getName();
		
		private:
			static std::string mName;
	};
}
 
 
#endif
