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
 *
 *	   $Id$
 *
 *****************************************************************************/

#ifndef __STRINGITERATORBINDER_H
#define __STRINGITERATORBINDER_H

#include "Iterator.h"

namespace Opde {

	namespace Python {

		/// String iterator python binder. Iterable
		class StringIteratorBinder : public shared_ptr_binder<StringIteratorPtr> {
			public:
				static void init(PyObject* module);
			
				// --- Python type related methods ---
				static PyObject* getattr(PyObject *self, char *name);

				/// to string - reprfunc conversion
				static PyObject* repr(PyObject *self);

				/// creates a python object representation of the string iterator
				static PyObject* create(StringIteratorPtr& strit);

			protected:
				/// Return self as iterator with a increased ref count.
				static PyObject* getIterObject(PyObject* self);
				
				/// Returns current object, advances to next object (or returns NULL if at end)
				static PyObject* getNext(PyObject* self);
			
				/// Static type definition for String Iterator
				static PyTypeObject msType;

				/// Name of the python type
				static const char* msName;

				/// Method list
				static PyMethodDef msMethods[];
		};
	}
}

#endif
