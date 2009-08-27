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


#ifndef __ROOMPORTAL_H
#define __ROOMPORTAL_H

#include "config.h"
#include "SharedPtr.h"

namespace Opde {
	/** @brief A room portal. Room portals connect two Room instances (doorways)
	*/
	class OPDELIB_EXPORT RoomPortal {
		public:
			RoomPortal();
			~RoomPortal();

		private:

	};

	/// Shared pointer to a room portal instance
	typedef shared_ptr<RoomPortal> RoomPortalPtr;
}


#endif
