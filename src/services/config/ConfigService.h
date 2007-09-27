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


#ifndef __CONFIGSERVICE_H
#define __CONFIGSERVICE_H

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "DatabaseService.h"
#include "FileGroup.h"
#include "SharedPtr.h"
#include "DVariant.h"

namespace Opde {

	/** @brief config service
	*/
	class ConfigService : public Service {
		public:
			ConfigService(ServiceManager *manager);
			virtual ~ConfigService();

			/** Set a parameter */
			void setParam(const std::string& param, const std::string& value);

            /** get a parameter */
			DVariant getParam(const std::string& param);

            /** determine an existence of a parameter */
			bool hasParam(const std::string& param);

			/** Injects the settings from a ogre's cfg file */
			void loadParams(const std::string& cfgfile);

		protected:
      typedef std::map< std::string, std::string > Parameters;
      Parameters mParameters;
	};

	/// Shared pointer to game service
	typedef shared_ptr<ConfigService> ConfigServicePtr;


	/// Factory for the GameService objects
	class ConfigServiceFactory : public ServiceFactory {
		public:
			ConfigServiceFactory();
			~ConfigServiceFactory() {};

			/** Creates a GameService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

		private:
			static std::string mName;
	};
}


#endif
