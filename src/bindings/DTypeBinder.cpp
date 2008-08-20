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
#include "DTypeBinder.h"

namespace Opde {

	namespace Python {

		// -------------------- Link Service --------------------
		char* DTypeBinder::msName = "DType";

		// ------------------------------------------
		PyTypeObject DTypeBinder::msType = {
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			msName,                   /* char *tp_name; */
			sizeof(DTypeBinder::Object),      /* int tp_basicsize; */
			0,                        /* int tp_itemsize;       /* not used much */
			DTypeBinder::dealloc,   /* destructor tp_dealloc; */
			0,			              /* printfunc  tp_print;   */
			DTypeBinder::getattr,  /* getattrfunc  tp_getattr; /* __getattr__ */
			0,   					  /* setattrfunc  tp_setattr;  /* __setattr__ */
			0,				          /* cmpfunc  tp_compare;  /* __cmp__ */
			0,			              /* reprfunc  tp_repr;    /* __repr__ */
			0,				          /* PyNumberMethods *tp_as_number; */
			0,                        /* PySequenceMethods *tp_as_sequence; */
			0,                        /* PyMappingMethods *tp_as_mapping; */
			0,			              /* hashfunc tp_hash;     /* __hash__ */
			0,                        /* ternaryfunc tp_call;  /* __call__ */
			0,			              /* reprfunc tp_str;      /* __str__ */
			0,			              /* getattrofunc tp_getattro; */
			0,			              /* setattrofunc tp_setattro; */
			0,			              /* PyBufferProcs *tp_as_buffer; */
			0,			              /* long tp_flags; */
			0,			              /* char *tp_doc;  */
			0,			              /* traverseproc tp_traverse; */
			0,			              /* inquiry tp_clear; */
			0,			              /* richcmpfunc tp_richcompare; */
			0,			              /* long tp_weaklistoffset; */
			0,			              /* getiterfunc tp_iter; */
			0,			              /* iternextfunc tp_iternext; */
			msMethods,	              /* struct PyMethodDef *tp_methods; */
			0,			              /* struct memberlist *tp_members; */
			0,			              /* struct getsetlist *tp_getset; */
		};

		// ------------------------------------------
		PyMethodDef DTypeBinder::msMethods[] = {
			// {"set", set, METH_VARARGS}, // Volca: Commented setter. Would lead to inappropriate usage (no data change broadcast)
			{"get", get, METH_VARARGS},
			{NULL, NULL},
		};

		// ------------------------------------------
		PyObject* DTypeBinder::set(PyObject* self, PyObject* args) {
			// Simmilar to PropertyServiceBinder::set
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			
			const char* field;
			PyObject* Object;

			if (PyArg_ParseTuple(args, "sO", &field, &Object)) 
			{
				DVariant value;
				value = PyObjectToDVariant(Object);
				o->mInstance->set(field, value);

				result = Py_None;
				Py_INCREF(result);
				return result;
			} 
			else 
			{
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a string and a DVariant!");
				return NULL;
			}
		}
		
		// ------------------------------------------
		PyObject* DTypeBinder::get(PyObject* self, PyObject* args) {
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			
			const char* field;

			if (PyArg_ParseTuple(args, "s", &field)) 
			{
				DVariant value;
				value = o->mInstance->get(field);

				result = DVariantToPyObject(value);
				return result;
			} 
			else 
			{
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a string argument!");
				return NULL;
			}
		}

		// ------------------------------------------
		PyObject* DTypeBinder::getattr(PyObject *self, char *name) {
			return Py_FindMethod(msMethods, self, name);
		}

		// ------------------------------------------
		DTypePtr DTypeBinder::extractDType(PyObject* object) {
			// Get Object from PyObject, return mInstance
			Object* o = python_cast<Object*>(object, &msType);
			
			// It is always good to remember that it is not wise to return shared_ptr directly
			DTypePtr t = o->mInstance;
			return t;
		}

		// ------------------------------------------
		PyObject* DTypeBinder::create(const DTypePtr& type) {
			Object* object = construct(&msType);

			if (object != NULL) {
				object->mInstance = type;
			}

			return (PyObject *)object;
		}
	}

} // namespace Opde

