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

#ifndef __VALUECHANGEREQUEST_H
#define __VALUECHANGEREQUEST_H

namespace Opde {

	/** A value + bool pair used to store requests of particular value (request for setting change).
	 * A typical usage is a within value change method that modifies a value that should be persistent
	 * for a defined period of time (one cycle iteration).
	 */
	template<typename T> class ValueChangeRequest {
		public:
			/// Constructor
			ValueChangeRequest() : mValue() { mRequested = false; };

			/// A request application method. Call this to set the new desired value
			void set(T newVal) { mValue = mValue; mRequested = true; };

			/** Sets the reference target with the new value if request happened
			 * @param tgt The target for the value change
			 * @return true uf value change happened
			 */
			bool getIfReq(T& tgt) { if (mRequested) { tgt = mValue; mRequested = false; return true; }; return false; };

			/// getter for the requested value
			T requestedVal() { return mValue; };

		protected:
			T mValue;
			bool mRequested;
	};

}

#endif
