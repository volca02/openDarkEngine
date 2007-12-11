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
 *	   $Id:$
 *
 *****************************************************************************/

#ifndef __PYCONFIGSERVICE_H
#define __PYCONFIGSERVICE_H

#include  "ConfigService.h"

namespace Opde {

	namespace Python {

		/// Config service python binder
		class ConfigServiceBinder {
			public:
				static void init();

				// --- Python type releted methods ---

				static void dealloc(PyObject *self);
				static PyObject* getattr(PyObject *self, char *name);

				static PyObject* create();

				// --- Methods ---

				static PyObject* setParam(PyObject* self, PyObject* args);
				static PyObject* getParam(PyObject* self, PyObject* args);
				static PyObject* hasParam(PyObject* self, PyObject* args);
				static PyObject* loadParams(PyObject* self, PyObject* args);

			protected:
				/// Python object instance definition
				typedef ObjectBase<ConfigServicePtr> Object;

				/// Static type definition for ConfigService
				static PyTypeObject msType;

				/// Name of the python type
				static char* msName;

				/// Method list
				static PyMethodDef msMethods[];
		};
	}
}

#endif
