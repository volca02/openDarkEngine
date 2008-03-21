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
 *	   $Id$
 *
 *****************************************************************************/

#ifndef __PROPERTYSERVICEBINDER_H
#define __PROPERTYSERVICEBINDER_H

#include  "DTypeDef.h"
#include  "PropertyService.h"

namespace Opde 
{
	namespace Python 
	{

		/// Property service python binder
		class PropertyServiceBinder : public shared_ptr_binder<PropertyServicePtr> {
			public:
				static void init(PyObject* module);
			
				// --- Python type related methods ---
				static PyObject* getattr(PyObject *self, char *name);

				/// creates a python object representation of the relation
				static PyObject* create();

				// --- Methods ---
				static PyObject* has(PyObject* self, PyObject* args);
				static PyObject* owns(PyObject* self, PyObject* args);
				static PyObject* set(PyObject* self, PyObject* args);
				static PyObject* get(PyObject* self, PyObject* args);
				static PyObject* getAllPropertyNames(PyObject* self, PyObject* args);
								
			protected:
				/// Static type definition for PropertyService
				static PyTypeObject msType;

				/// Name of the python type
				static char* msName;

				/// Method list
				static PyMethodDef msMethods[];
		};
	}
}

#endif	// __PROPERTYSERVICEBINDER_H
