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

#ifndef __ROOTBINDER_H
#define __ROOTBINDER_H

#include  "Root.h"

namespace Opde {

	namespace Python {

		/// Opde::Root python binder
		class RootBinder : public class_ptr_binder<Opde::Root> {
			public:
				// --- Python type related methods ---
				/// to string - reprfunc conversion
				static PyObject* repr(PyObject *self);

				/// creates a python object representation of the Opde::Root object
				static PyObject* create(Root* root);

				/// Initializes the opde.Root object (type)
				static void init(PyObject* module);

				// methods:
				static PyObject* loadResourceConfig(PyObject *self, PyObject* args);
				static PyObject* loadDTypeScript(PyObject *self, PyObject* args);
				static PyObject* loadPLDefScript(PyObject *self, PyObject* args);
				static PyObject* loadConfigFile(PyObject *self, PyObject* args);
				static PyObject* addResourceLocation(PyObject *self, PyObject* args);
				static PyObject* removeResourceLocation(PyObject *self, PyObject* args);

				static PyObject* bootstrapFinished(PyObject *self, PyObject* args);

				static PyObject* logToFile(PyObject *self, PyObject* args);
				static PyObject* setLogLevel(PyObject *self, PyObject* args);

				static PyObject* registerCustomScriptLoaders(PyObject *self, PyObject* args);


				/* TODO: Need bindings written first
				static PyObject* getLogger(PyObject *self, PyObject* args);
				static PyObject* getServiceManager(PyObject *self, PyObject* args);
				*/

			protected:
				static PyTypeObject msType;

				/// Name of the python type
				static const char* msName;

				/// Method list
				static PyMethodDef msMethods[];
		};
	}
}

#endif
