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

#include "PhysCommon.h"
#include "File.h"
#include "FileCompat.h"
#include "integers.h"

namespace Opde {
	/*----------------------------------------------------*/
	/*----------------- PhysCompat helpers ---------------*/
	/*----------------------------------------------------*/
	
	File& operator<<(File& st, const PhysLocation& val) {
		st 	<< val.location 
			<< val.state1 
			<< val.state2 
			<< val.facing;
			
		return st;
	}
	
	File& operator>>(File& st, PhysLocation& val) {
		st >> val.location >> val.state1 >> val.state2 >> val.facing;
		
		return st;
	}
	
	File& operator<<(File& st, const SubModel& val) {
		st	<< val.location 
			<< val.end_location 
			<< val.target_location 
			<< val.unknown; 

		// 32bit pointer. skipped
		st << uint32_t(0);
		
		return st;
	}
	
	File& operator>>(File& st, SubModel& val) {
		st	>> val.location 
			>> val.end_location 
			>> val.target_location 
			>> val.unknown;
		
		// value of the owner is not read, ignored
		val.owner = NULL;
			
		// this is just to skip the owner pointer
		uint32_t vl = 0;
		st >> vl;
		
		return st;
	}
}