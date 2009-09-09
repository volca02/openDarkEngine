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
 
#include "darkdb.h"
#include "File.h"

using namespace std;

namespace Opde {
	File& operator<<(File& st, const DarkDBHeader& h) {
		st << h.inv_offset << h.zero << h.one;
		st.write(&h.zeros, sizeof(h.zeros));
		st << h.dead_beef;
		
		return st;
	}
	
        File& operator>>(File& st, DarkDBHeader& h) {
		st >> h.inv_offset >> h.zero >> h.one;
		st.read(&h.zeros, sizeof(h.zeros));
		st >> h.dead_beef;
		
		return st;
	}

	File& operator<<(File& st, const DarkDBInvItem& h) {
		st.write(&h.name, sizeof(h.name));
		st << h.offset << h.length;
		
		return st;
	}
	
	File& operator>>(File& st, DarkDBInvItem& h) {
		st.read(&h.name, sizeof(h.name));
		st >> h.offset >> h.length;
		
		return st;
	}

	File& operator<<(File& st, const DarkDBChunkHeader& h) {
		st.write(&h.name, sizeof(h.name));
		st << h.version_high << h.version_low << h.zero;
		
		return st;
	}
	
	File& operator>>(File& st, DarkDBChunkHeader& h) {
		st.read(&h.name, sizeof(h.name));
		st >> h.version_high >> h.version_low >> h.zero;
		
		return st;
	}
	
}
