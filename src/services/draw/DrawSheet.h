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


#ifndef __DRAWSHEET_H
#define __DRAWSHEET_H

#include "DrawBuffer.h"
#include "DrawOperation.h"

namespace Opde {

	/** A 2D rendering sheet. Represents one visible screen.
	 * Stores rendering operations, can queue itself for rendering to ogre.
	 * Uses DrawBuffer for render op. storage */
	class DrawSheet {
		public:
			/// Constructor
			DrawSheet();

			/// Destructor
			~DrawSheet();

			/// Activates the sheet, prepares it for rendering
			void activate();

			/// Deactivates the sheet
			void deactivate();

			/// Adds a draw operation to be rendered on this sheet
			void addDrawOperation(DrawOperation* drawOp);

			/// Removes the draw operation from this sheet.
			void removeDrawOperation(DrawOperation* toRemove);

			/// Internal: Marks a certain draw operation dirty in this sheet. Called internally on DrawOp. change
			void _markDirty(DrawOperation* drawOp);

			/// Will remove all Buffers with zero Draw operations
			void purge();

		protected:
			/// Called to ensure all the DrawBuffers are current and reflect the requested state
			void rebuildBuffers();

			DrawBuffer* getBufferForOperation(DrawOperation* drawOp, bool autoCreate = false);

			/// All draw buffers for the sheet as map
			DrawBufferMap mDrawBufferMap;

			/// True if this sheet is rendered
			bool mActive;

			/// All draw operations on this sheet
			DrawOperationMap mDrawOpMap;
	};
}

#endif
