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

#ifndef __PHYSCOMMON_H
#define __PHYSCOMMON_H

#include "compat.h"
#include "integers.h"
#include "Vector3.h"
#include "Quaternion.h"

namespace Opde {
	// Forward decls.
	class PhysModel;
	class File;
	
	/// Phys model flags. Incomplete & may be wrong
	typedef enum {
		PHYS_MDL_INACTIVE    = 0x000000004,
		PHYS_MDL_MOVABLE     = 0x000000200,
		PHYS_MDL_PLAYER      = 0x000000800,
		PHYS_MDL_SLEEPING    = 0x000000080,
		PHYS_MDL_PPLATE      = 0x000008000,
		PHYS_MDL_ROPE        = 0x000001000,
		/// Deployed rope
		PHYS_MDL_DEPL_ROPE   = 0x000004000,
		PHYS_MDL_FACE_VEL    = 0x000040000,
		PHYS_MDL_MTERRAIN    = 0x000100000,
		PHYS_MDL_DOOR        = 0x000200000,
		PHYS_MDL_AI_COLLIDE  = 0x000400000,
		PHYS_MDL_PROJECTILE  = 0x000800000,
		/// Flag indicating the refs should be updated - ignored here
		PHYS_MDL_UPDATEREFS  = 0x002000000
	} PhysModelFlags;
	
	struct PhysLocation {
		Vector3 location;
		int16_t state1; // unknown - some kind of manipulation is done on every location setting
		int16_t state2;
		Quaternion facing;
	};
	
	struct SubModel {
		PhysLocation location;
		PhysLocation end_location;
		PhysLocation target_location;
		
		uint16_t unknown; // Maybe cell id?
		PhysModel* owner;
	};

	File& operator<<(File& st, const PhysLocation& val);
	File& operator>>(File& st, PhysLocation& val);
	
	File& operator<<(File& st, const SubModel& val);
	File& operator>>(File& st, SubModel& val);
};

#endif