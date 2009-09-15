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
 *
 *		$Id$
 *
 *****************************************************************************/

#ifndef __OPDESERVICE_H
#define __OPDESERVICE_H

#include "config.h"

#include "NonCopyable.h"
#include "SharedPtr.h"

#include <string>

namespace Opde {

	// Forward declaration
	class ServiceManager;

	/** Interface used for all services. Those must implement the here mentioned methods. 
	* @warning Do not override. Override ServiceImpl instead to avoid SID manipulation problems (And others) */
	class OPDELIB_EXPORT Service : public NonCopyable {
		protected:
			friend class ServiceManager;
			
		public:
			/** Destructor. When using service dependencies, be aware that if an error happened, the fields can be un-initialized.
			*/
			virtual ~Service() {};

			/// Service name getter
			virtual const std::string& getName() const = 0;
			
			/// SID service id getter
			virtual const size_t getSID() const = 0;
		
		protected:
			/** Intialization of the service. Guaranteed to be called after construction (If constructor was sucessful).
			* Used to estabilish relations with other services. Only the dependencies that are fixed can be resolved here, otherwise use the bootstrapFinished.
			* @return true on success, false on a fatal error
			*/
			virtual bool init() = 0;

			/** Tells the service that bootstraping has finished. Bootstraping process initializes the default and needed values in the services.
			* This method is called after the bootstraping happened, so the services can use data that were initialized by the bootstraping.
			* For example: if a InheritService depended on Metaprop relation, the metaprop relation has to be constructed in advance.
			* This method can be thus used to initialize the InheritService (callback to the MetaProp relation)
			* Called by ServiceManager::bootstrapFinished()
			* This method  */
			virtual void bootstrapFinished() {};

			/** This will be called right before the service manager releases the service from it's evidence. All shared pointers to other services
			* should be released here to enable seamless shutdown.
			*/
			virtual void shutdown() {};
	};

	/// Implementation base for all services. Used to force-in the inclusion of service-specific version of the static variables
	template<class C> class OPDELIB_EXPORT ServiceImpl : public Service {
		public:
			/** Constructor. Do not implement inheritance resolution here! (Can cycle if you do so)
			* @param manager The constructing manager
			* @param name The name of the service (unique)
			* @param sid The service id (unique) */
			ServiceImpl(ServiceManager* manager, const std::string& name) : Service(), mServiceManager(manager), mName(name) {
			};
			
			/** Destructor */
			~ServiceImpl() {
			}
			
			/** Service ID of the service.
			* To be filled in the implementation of the service.
			*/
			static const size_t SID;
			
			/// Service ID getter
			virtual const size_t getSID() const {
				return SID;
			}
			
			/// Service Name getter
			virtual const std::string& getName() const { return mName; };
			
		protected:
			ServiceManager* mServiceManager;
			std::string mName;
	};

	/// A shared pointer to service
	typedef shared_ptr<Service> ServicePtr;
}


#endif
