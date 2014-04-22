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

#ifndef __DATABASESERVICEBINDER_H
#define __DATABASESERVICEBINDER_H

#include  "DatabaseService.h"

namespace Opde {

	namespace Python {

		/// Database python binder
		class DatabaseServiceBinder : public shared_ptr_binder<DatabaseServicePtr> {
			public:
				static void init(PyObject* module);

				// --- Python type related methods ---
				static PyObject* create();

				// --- Methods ---
				static PyObject* load(PyObject* self, PyObject* args);
				static PyObject* mergeLoad(PyObject* self, PyObject* args);
				static PyObject* recursiveMergeLoad(PyObject* self, PyObject* args);
				static PyObject* save(PyObject* self, PyObject* args);
				static PyObject* unload(PyObject* self, PyObject* args);

				static PyObject* setProgressListener(PyObject* self, PyObject* args);
				static PyObject* unsetProgressListener(PyObject* self, PyObject* args);

			protected:
				/// Static type definition for DatabaseService
				static PyTypeObject msType;

				/// Name of the python type
				static const char* msName;

				/// Method list
				static PyMethodDef msMethods[];
		};
	}
}

#endif
