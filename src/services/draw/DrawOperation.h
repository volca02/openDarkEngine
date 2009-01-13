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


#ifndef __DRAWOPERATION_H
#define __DRAWOPERATION_H

#include <OgreString.h>

namespace Opde {
	// Forward decls.
	class DrawService;
	class DrawBuffer;
	class DrawSheet;

	/** A single 2D draw operation (Bitmap draw for example). Internally this explodes to N vertices stored in the VBO of choice (via DrawBuffer) - For building itself into a VBO, this produces N DrawQuads. */
	class DrawOperation {
		public:
			/// ID type of this operation
			typedef size_t ID;

			DrawOperation(DrawService* owner, ID id, size_t order, const Ogre::String& name);

			virtual ~DrawOperation();

			inline ID getID() const { return mID; };

			const Ogre::String& getMaterialName() const;

			/// Called by DrawBuffer to get the Quads queued for rendering. Fill this method to get the rendering done (via DrawBuffer::_queueDrawQuad())
			virtual void visitDrawBuffer(DrawBuffer* db);

			/// Called by sheet when the operation is added to a sheet. Default implementation adds the sheet to the sheet set.
			virtual void onSheetRegister(DrawSheet* sheet);

			/// Called by sheet when the operation is removed from a sheet. Default implementation adds the sheet to the sheet set.
			virtual void onSheetUnregister(DrawSheet* sheet);

		protected:
			/// On change updater - marks all using sheets as dirty
			virtual void onChange();

			ID mID;

			Ogre::String mImageName;
			DrawService* mOwner;

			typedef std::set<DrawSheet*> DrawSheetSet;

			/// Sheets using this draw op
			DrawSheetSet mUsingSheets;
	};

	/// Map of all draw operations by it's ID
	typedef std::map<DrawOperation::ID, DrawOperation*> DrawOperationMap;
}

#endif
