/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2009 openDarkEngine team
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

/** @file FileCompat.cpp
 * @brief A various utility methods for File class usage - implementation
 */


#include "config.h"
#include "FileCompat.h"

namespace Opde {
	// Vector2
	File& operator<<(File& st, const Ogre::Vector2& val) {
		st << val.x << val.y;
		return st;
	}
	
	File& operator>>(File& st, Ogre::Vector2& val) {
		st >> val.x >> val.y;
		return st;
	}
	
	// Vector3
	File& operator<<(File& st, const Ogre::Vector3& val) {
		st << val.x << val.y << val.z;
		return st;
	}
	
	File& operator>>(File& st, Ogre::Vector3& val) {
		st >> val.x >> val.y >> val.z;
		return st;
	}
	
	// Plane
	File& operator<<(File& st, const Ogre::Plane& val) {
		st << val.normal << val.d;
		return st;
	}
	
	File& operator>>(File& st, Ogre::Plane& val) {
		st >> val.normal >> val.d;
		return st;
	}

}

