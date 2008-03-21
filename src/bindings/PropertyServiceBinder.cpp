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
#include "PropertyServiceBinder.h"
#include "StringIteratorBinder.h"

namespace Opde 
{

	namespace Python 
	{

		// -------------------- Property Service --------------------
		char* PropertyServiceBinder::msName = "PropertyService";

		// ------------------------------------------
		PyTypeObject PropertyServiceBinder::msType = 
		{
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			msName,                   /* char *tp_name; */
			sizeof(PropertyServiceBinder::Object),      /* int tp_basicsize; */
			0,                        /* int tp_itemsize;       /* not used much */
			PropertyServiceBinder::dealloc,   /* destructor tp_dealloc; */
			0,			              /* printfunc  tp_print;   */
			PropertyServiceBinder::getattr,  /* getattrfunc  tp_getattr; /* __getattr__ */
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
		PyMethodDef PropertyServiceBinder::msMethods[] = 
		{
			{"has",  has, METH_VARARGS},
			{"owns", owns, METH_VARARGS},
			{"set",  set, METH_VARARGS},
			{"get",  get, METH_VARARGS},
			{"getAllPropertyNames", getAllPropertyNames, METH_NOARGS},
			{NULL, NULL},
		};

		// ------------------------------------------
		PyObject* PropertyServiceBinder::has(PyObject* self, PyObject* args) 
		{
			Object* o = python_cast<Object*>(self, &msType);
			
			int obj_id;
			const char* propName;

			if (PyArg_ParseTuple(args, "is", &obj_id, &propName))
				return PyBool_FromLong(o->mInstance->has(obj_id, propName));
			else 
			{
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected an integer and a string!");
				return NULL;
			}
		}

		// ------------------------------------------
		PyObject* PropertyServiceBinder::owns(PyObject* self, PyObject* args) 
		{
			Object* o = python_cast<Object*>(self, &msType);
			
			int obj_id;
			const char* propName;

			if (PyArg_ParseTuple(args, "is", &obj_id, &propName))
				return PyBool_FromLong(o->mInstance->owns(obj_id, propName));
			else 
			{
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected an integer and a string!");
				return NULL;
			}
		}

		// ------------------------------------------
		PyObject* PropertyServiceBinder::set(PyObject* self, PyObject* args) 
		{
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			
			int obj_id;
			const char* propName;
			const char* propField;
			PyObject* Object;
			DVariant value;

			if (PyArg_ParseTuple(args, "issO", &obj_id, &propName, &propField, &Object)) 
			{
				value = PyObjectToDVariant(Object);
				o->mInstance->set(obj_id, propName, propField, value);

				result = Py_None;
				Py_INCREF(result);
				return result;
			} 
			else 
			{
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected an integer, two strings and a DVariant!");
				return NULL;
			}
		}

		// ------------------------------------------
		PyObject* PropertyServiceBinder::get(PyObject* self, PyObject* args) 
		{
			Object* o = python_cast<Object*>(self, &msType);
			
			int obj_id;
			const char* propName;
			const char* propField;

			if (PyArg_ParseTuple(args, "iss", &obj_id, &propName, &propField)) 
				return DVariantToPyObject(o->mInstance->get(obj_id, propName, propField));
			else 
			{
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected an integer and two strings!");
				return NULL;
			}
		}
		
		// ------------------------------------------
		PyObject* PropertyServiceBinder::getAllPropertyNames(PyObject* self, PyObject* args) 
		{
			Object* o = python_cast<Object*>(self, &msType);
			
			// wrap the returned StringIterator into StringIteratorBinder, return
			StringIteratorPtr res = o->mInstance->getAllPropertyNames();
				
			return StringIteratorBinder::create(res);
		}

		// ------------------------------------------
		PyObject* PropertyServiceBinder::getattr(PyObject *self, char *name) 
		{
			return Py_FindMethod(msMethods, self, name);
		}

		// ------------------------------------------
		void PropertyServiceBinder::init(PyObject* module) 
		{
			publishType(module, &msType, msName);
			
			StringIteratorBinder::init(module);
		}
		
		// ------------------------------------------
		PyObject* PropertyServiceBinder::create() 
		{
			Object* object = construct(&msType);

			if (object != NULL) 
			{
				object->mInstance = ServiceManager::getSingleton().getService(msName).as<PropertyService>();
			}
			return (PyObject *)object;
		}
	}

} // namespace Opde

