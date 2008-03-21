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

#include "bindings.h"
#include "LinkQueryResultBinder.h"
#include "LinkServiceBinder.h"

namespace Opde {

	namespace Python {

		// -------------------- Link Query result --------------------
		char* LinkQueryResultBinder::msName = "LinkQueryResult";

		// ------------------------------------------
		PyTypeObject LinkQueryResultBinder::msType = {
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			msName,                   /* char *tp_name; */
			sizeof(LinkQueryResultBinder::Object),      /* int tp_basicsize; */
			0,                        /* int tp_itemsize;       /* not used much */
			LinkQueryResultBinder::dealloc,   /* destructor tp_dealloc; */
			0,			              /* printfunc  tp_print;   */
			LinkQueryResultBinder::getattr,  /* getattrfunc  tp_getattr; /* __getattr__ */
			0,   					  /* setattrfunc  tp_setattr;  /* __setattr__ */
			0,				          /* cmpfunc  tp_compare;  /* __cmp__ */
			repr,			          /* reprfunc  tp_repr;    /* __repr__ */
			0,				          /* PyNumberMethods *tp_as_number; */
			0,                        /* PySequenceMethods *tp_as_sequence; */
			0,                        /* PyMappingMethods *tp_as_mapping; */
			0,			              /* hashfunc tp_hash;     /* __hash__ */
			0,                        /* ternaryfunc tp_call;  /* __call__ */
			0,			              /* reprfunc tp_str;      /* __str__ */
			0,			              /* getattrofunc tp_getattro; */
			0,			              /* setattrofunc tp_setattro; */
			0,			              /* PyBufferProcs *tp_as_buffer; */
			Py_TPFLAGS_DEFAULT,       /* long tp_flags; */
			0,			              /* char *tp_doc;  */
			0,			              /* traverseproc tp_traverse; */
			0,			              /* inquiry tp_clear; */
			0,			              /* richcmpfunc tp_richcompare; */
			0,			              /* long tp_weaklistoffset; */
			getIterObject,            /* getiterfunc tp_iter; */
			getNext,	              /* iternextfunc tp_iternext; */
			msMethods,	              /* struct PyMethodDef *tp_methods; */
			0,			              /* struct memberlist *tp_members; */
			0,			              /* struct getsetlist *tp_getset; */
		};

		// ------------------------------------------
		PyMethodDef LinkQueryResultBinder::msMethods[] = {
			{NULL, NULL},
		};
		
		
		// ------------------------------------------
		PyObject* LinkQueryResultBinder::getIterObject(PyObject* self) {
			Py_INCREF(self);
			return self;
		}
		
		// ------------------------------------------
		PyObject* LinkQueryResultBinder::getNext(PyObject* self) {
			Object* o = python_cast<Object*>(self, &msType);
			
			// Get returnable object, advance to next.
			PyObject* next = NULL;
			
			if ((!o->mInstance.isNull()) && !o->mInstance->end()) {
				LinkPtr l = o->mInstance->next();
				next = LinkBinder::create(l);
			}
			
			return next;
		}
		
		// ------------------------------------------
		PyObject* LinkQueryResultBinder::repr(PyObject *self) {
			return PyString_FromFormat("<LinkQueryResult at %p>", self);
		}
		
		
		// ------------------------------------------
		PyObject* LinkQueryResultBinder::getattr(PyObject *self, char *name) {
			return Py_FindMethod(msMethods, self, name);
		}
		
		// ------------------------------------------
		PyObject* LinkQueryResultBinder::create(LinkQueryResultPtr result) {
			Object* object = construct(&msType);

			if (object != NULL) {
				object->mInstance = result;
			}

			return (PyObject *)object;
		}
		
		// ------------------------------------------
		void LinkQueryResultBinder::init(PyObject* module) {
			publishType(module, &msType, msName);
		}

	}

} // namespace Opde

