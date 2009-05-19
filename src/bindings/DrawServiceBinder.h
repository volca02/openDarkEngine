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
 *
 *	   $Id$
 *
 *****************************************************************************/

#ifndef __DRAWSERVICEBINDER_H
#define __DRAWSERVICEBINDER_H

#include "BinaryService.h"
#include "DrawService.h"

namespace Opde {

	namespace Python {

		/// Draw service python binder
		class DrawServiceBinder : public shared_ptr_binder<DrawServicePtr> {
			public:
				static void init(PyObject* module);

				// --- Python type related methods ---
				static PyObject* getattr(PyObject *self, char *name);
				
				/// to string - reprfunc conversion
				static PyObject* repr(PyObject *self);
				
				// --- Methods ---
				// Sheet				
				static PyObject* createSheet(PyObject* self, PyObject* args);
				static PyObject* destroySheet(PyObject* self, PyObject* args);
				static PyObject* getSheet(PyObject* self, PyObject* args);
				static PyObject* setActiveSheet(PyObject* self, PyObject* args);
				
				// various draw sources and rendered objects
				static PyObject* createDrawSource(PyObject* self, PyObject* args);
				static PyObject* createRenderedImage(PyObject* self, PyObject* args);
				static PyObject* createRenderedLabel(PyObject* self, PyObject* args);
				static PyObject* destroyDrawOperation(PyObject* self, PyObject* args);
				
				static PyObject* createAtlas(PyObject* self, PyObject* args);
				static PyObject* destroyAtlas(PyObject* self, PyObject* args);
				
				static PyObject* loadFont(PyObject* self, PyObject* args);
				static PyObject* setFontPalette(PyObject* self, PyObject* args);
				
				static PyObject* create();

			protected:
				/// Static type definition
				static PyTypeObject msType;

				/// Name of the python type
				static const char* msName;

				/// Method list
				static PyMethodDef msMethods[];
		};
		

		// ClipRect type info
                template<> struct TypeInfo<ClipRect> : TypeInfoBase<ClipRect> {
                        TypeInfo() : TypeInfoBase<ClipRect>("ClipRect", VT_CUSTOM_TYPE) {};

                        PyObject* toPyObject(const ClipRect& cr) const {
				// tuple it is
				PyObject *crp = PyTuple_New(4);
				PyTuple_SetItem(crp, 0, PyFloat_FromDouble(cr.left)); // Steals reference
				PyTuple_SetItem(crp, 1, PyFloat_FromDouble(cr.right)); // Steals reference
				PyTuple_SetItem(crp, 2, PyFloat_FromDouble(cr.top)); // Steals reference
				PyTuple_SetItem(crp, 3, PyFloat_FromDouble(cr.bottom)); // Steals reference
				
				return crp;
                        }
			
			bool fromPyObject(PyObject* o, ClipRect& cr) {
				if (!PyTuple_Check(o))
					return false;
					
				if (PyTuple_GET_SIZE(o) != 4)
					return false;
				
				cr.left   = PyFloat_AsDouble(PyTuple_GetItem(o, 0));
				cr.right  = PyFloat_AsDouble(PyTuple_GetItem(o, 1));
				cr.top    = PyFloat_AsDouble(PyTuple_GetItem(o, 2));
				cr.bottom = PyFloat_AsDouble(PyTuple_GetItem(o, 3));
				
				return true;
			}
                };
                   
		
	}
}

#endif
