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

/**
 @file DarkCommon.h
 @brief Common data types used throughout the entire engine. Here, these are mainly used for disk access.
 */


#ifndef __DARKCOMMON_H
#define __DARKCOMMON_H

#include "config.h"
#include "integers.h"
#include "File.h"

namespace Opde {
	/// A 3D coordinate
	struct DVector3 { // SIZE: 12
		float	 x;
		float    y;
		float    z;
		
		void read(const FilePtr& f) {
			f->read(&x, sizeof(float));
			f->read(&y, sizeof(float));
			f->read(&z, sizeof(float));
		}
		
		void write(const FilePtr& f) {
			f->write(&x, sizeof(float));
			f->write(&y, sizeof(float));
			f->write(&z, sizeof(float));
		}
	};
	
	/// A 2D coordinate
	struct DVector2 { // SIZE: 8
		float	x;
		float   y;
		
		void read(const FilePtr& f) {
			f->read(&x, sizeof(float));
			f->read(&y, sizeof(float));
		}
		
		void write(const FilePtr& f) {
			f->write(&x, sizeof(float));
			f->write(&y, sizeof(float));
		}
	};

	/// A plane (normal + distance)
	struct DPlane { // SIZE: 16
		DVector3	normal;
		float		d;
		
		void read(const FilePtr& f) {
			normal.read(f);
			f->read(&d, sizeof(float));
		}
		
		void write(const FilePtr& f) {
			normal.write(f);
			f->write(&d, sizeof(float));
		}

	};
}

#endif
