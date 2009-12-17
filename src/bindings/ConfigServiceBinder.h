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

#ifndef __CONFIGSERVICEBINDER_H
#define __CONFIGSERVICEBINDER_H

#include  "ConfigService.h"

namespace Opde {

	namespace Python {

		/// Config service python binder
		class ConfigServiceBinder : public shared_ptr_binder<ConfigServicePtr> {
			public:
				static void init(PyObject* module);
				
				// --- Python type related methods ---
				static PyObject* getattr(PyObject *self, char *name);

				/// to string - reprfunc conversion
				static PyObject* repr(PyObject *self);

				static PyObject* create();

				// --- Methods ---

				static PyObject* setParam(PyObject* self, PyObject* args);
				static PyObject* getParam(PyObject* self, PyObject* args);
				static PyObject* hasParam(PyObject* self, PyObject* args);
				static PyObject* loadParams(PyObject* self, PyObject* args);
				static PyObject* setConfigPathOverride(PyObject* self, PyObject* args);

			protected:
				/// Static type definition for ConfigService
				static PyTypeObject msType;

				/// Name of the python type
				static const char* msName;

				/// Method list
				static PyMethodDef msMethods[];
		};
	}
}

#endif
