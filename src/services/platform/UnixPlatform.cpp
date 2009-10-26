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

#include "UnixPlatform.h"
#include <stdlib.h>

using namespace std;

namespace Opde {
	const std::string UnixPlatform::msOpdeHomeDirName = ".opde";

	/*----------------------------------------------------*/
	/*--------------------- UnixPlatform ----------------*/
	/*----------------------------------------------------*/
	UnixPlatform::UnixPlatform(PlatformService* owner) : Platform(owner) {
	}
	
	//------------------------------------------------------
	UnixPlatform::~UnixPlatform() {
	}

	//------------------------------------------------------
	std::string UnixPlatform::getGlobalConfigPath() const {
		// global config path is hardcoded at compile time
		// it is the location of share/opde directory...
		// for the share directory location, we have a config time define
		return OPDE_SHARE_DIR;
	}

	//------------------------------------------------------
	std::string UnixPlatform::getUserConfigPath() const {
		// local config path is based on the user's home directory
		// respectively the .opde directory inside it
		std::string home = getenv("HOME");

		return home + getDirectorySeparator() + msOpdeHomeDirName + getDirectorySeparator();
	}

	//------------------------------------------------------
	std::string UnixPlatform::getDirectorySeparator() const {
		return "/";
	}
}
