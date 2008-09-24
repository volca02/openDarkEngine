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
 *
 *		$Id$
 *
 *****************************************************************************/
 
#ifndef __REFCOUNTED_H
#define __REFCOUNTED_H

#include "config.h"

namespace Opde {
		
	// TODO: Make this atomic
	/** Reference counter. A base class for all classes using reference counting approach. 
	* The addRef() method should be called every time a copy of the pointer to ref. counted class is made.
	* a release() should be called when throwing the pointer away. 
	* @note : the counter is intialized to 0, because some classes can allow changes of a certain type only on non-referenced instances, do not forget to call the addRef as soon as finished
	* @todo make the addref and release atomic operations (couldn't hurt, and is a must if we're gonna do multithreaded app) 
	* 
	* <b>Usage example</b><br>
	* <code>
	*  // First, I create some class which is reference counted<br>
	*  RefCClass* test = new RefCClass();<br>
	*  // now use the field definition as you like<br>
	*  //....<br>
	*  // and release after being finished<br>
	*<br>
	*  test->release();<br>
	* </code>
	* @deprecated */
	class OPDELIB_EXPORT RefCounted {
		protected:
			int mReferences;
		public:
			/** Constructs the RefCounted instance with the reference count of 0 */
			RefCounted();
	
			/** Adds one reference to the instance */
			void addRef();
	
			/** Releases one reference to the instance. If the reference count is 0, deletes itself */
			void release();
	
			/** Returns true if the instance is referenced. */
			bool isRef() const;
			
			/** reference count getter */
			inline int getRefCount() { return mReferences; };
			
			/** Destructor. */
			virtual ~RefCounted();
	};

} // namespace Opde

#endif
