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

#include "Win32Platform.h"

using namespace std;

namespace Opde {

	/*----------------------------------------------------*/
	/*--------------------- Win32Platform ----------------*/
	/*----------------------------------------------------*/
	Win32Platform::Win32Platform(PlatformService* owner) : Platform(owner) {
	}

	//------------------------------------------------------
	Win32Platform::~Win32Platform() {
	}

	//------------------------------------------------------
	std::string Win32Platform::getGlobalConfigPath() const {
		// TODO: CODE
		return "\\Invalid\\";
	}

	//------------------------------------------------------
	std::string Win32Platform::getUserConfigPath() const {
		// TODO: CODE
		return "\\Invalid\\";
	}

	//------------------------------------------------------
	std::string Win32Patform::getDirectorySeparator() const {
		return "\\";
	}
}
