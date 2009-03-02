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

#ifndef __DTYPEBINDER_H
#define __DTYPEBINDER_H

#include  "DTypeDef.h"

namespace Opde {

	namespace Python {

		/// DType python binder
		class DTypeBinder : public shared_ptr_binder<DTypePtr> {
			public:
				// --- Python type related methods ---
				static PyObject* getattr(PyObject *self, char *name);

				static PyObject* create(const DTypePtr& type);

				/// Extracts a DTypePtr from the PyObject*, checking type
				static DTypePtr extractDType(PyObject* object);

				// --- Methods ---
				/// setter for values
				static PyObject* set(PyObject* self, PyObject* args);
				/// getter for values
				static PyObject* get(PyObject* self, PyObject* args);

				/// Initializes the type - exposes it in the module
				static void init(PyObject* module);

			protected:
				/// Static type definition for LinkService
				static PyTypeObject msType;

				/// Name of the python type
				static const char* msName;

				/// Method list
				static PyMethodDef msMethods[];
		};
	}
}

#endif
