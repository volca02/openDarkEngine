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

#ifndef __INTEGERS_H
#define __INTEGERS_H

#include "config.h"

#ifndef HAVE_INTTYPES_H
// Try to use VC++ types
	#define uint64_t unsigned __int64
	#define uint32_t unsigned __int32
	#define uint16_t unsigned __int16
	#define uint8_t  unsigned __int8
	#define int64_t  __int64	
	#define int32_t  __int32
	#define int16_t  __int16
	#define int8_t   __int8
#else
	#include <inttypes.h>
#endif

namespace Opde {
#ifndef uint
 typedef unsigned int uint;
#endif
}

#endif

