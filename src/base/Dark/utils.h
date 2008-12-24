/******************************************************************************
 *    utils.h
 *
 *    This file is part of DarkUtils
 *    Copyright (C) 2004 Tom N Harris <telliamed@whoopdedo.cjb.net>
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

#ifndef DARK_UTILS_H
#define DARK_UTILS_H

#define UNUSED(v)	/* v */

#include "integers.h"

namespace Dark {
	
typedef uint64_t uint64;
typedef int64_t sint64;
typedef uint32_t uint32;
typedef int32_t sint32;
typedef uint16_t uint16;
typedef int16_t sint16;
typedef uint8_t uint8;
typedef int8_t sint8;

/* Isn't it great how everyone can invent their own boolean data type? */
typedef unsigned long	Boolean;

#ifdef __cplusplus
inline uint64 make_ll(uint32 h, uint32 l)
{
	return (((uint64)h<<32)&((uint64)0xFFFFFFFF<<32))|((uint64)l&0xFFFFFFFF);
}
#else
#define make_ll(__h,__l)	\
		((((uint64)(__h)<<32)&((uint64)0xFFFFFFFF<<32))|((uint64)(__l)&0xFFFFFFFF))
#endif

// Till we get rid of the headers - Stop warnings about the zero sized arrays
// Dark headers are not used anymore - // #pragma warning (disable : 4200)

}

#endif
