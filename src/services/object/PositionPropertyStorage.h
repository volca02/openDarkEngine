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
 *	  $Id$
 *
 *****************************************************************************/


#ifndef __POSITIONPROPERTYSTORAGE_H
#define __POSITIONPROPERTYSTORAGE_H

#include "StructDataStorage.h"

namespace Opde {

	typedef struct {
	    Vector3 position;
	    uint32_t cell;
	    Quaternion facing;
	} sPositionProp;

	/** Position property storage */
	class PositionPropertyStorage : public StructDataStorage<sPositionProp> {
		public:
			PositionPropertyStorage();
			virtual ~PositionPropertyStorage();
	};

	typedef shared_ptr<PositionPropertyStorage> PositionPropertyStoragePtr;
};

#endif


