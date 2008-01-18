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
#include "ConfigServiceBinder.h"

namespace Opde {

	namespace Python {

		// -------------------- Config Service --------------------
		char* ConfigServiceBinder::msName = "ConfigService";

		// ------------------------------------------
		PyTypeObject ConfigServiceBinder::msType = {
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			msName,                   /* char *tp_name; */
			sizeof(ConfigServiceBinder::Object),      /* int tp_basicsize; */
			0,                        /* int tp_itemsize;       /* not used much */
			ConfigServiceBinder::dealloc,   /* destructor tp_dealloc; */
			0,			              /* printfunc  tp_print;   */
			ConfigServiceBinder::getattr,  /* getattrfunc  tp_getattr; /* __getattr__ */
			0,   					  /* setattrfunc  tp_setattr;  /* __setattr__ */
			0,				          /* cmpfunc  tp_compare;  /* __cmp__ */
			repr,		              /* reprfunc  tp_repr;    /* __repr__ */
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
		PyMethodDef ConfigServiceBinder::msMethods[] = {
			{"setParam", setParam, METH_VARARGS},
			{"getParam", getParam, METH_VARARGS},
			{"hasParam", hasParam, METH_VARARGS},
			{"loadParams", loadParams, METH_VARARGS},
			{NULL, NULL},
		};

		// ------------------------------------------
		PyObject* ConfigServiceBinder::getattr(PyObject *self, char *name) {
			return Py_FindMethod(msMethods, self, name);
		}

		// ------------------------------------------
		PyObject* ConfigServiceBinder::create() {
			Object* object = construct(&msType);

			if (object != NULL) {
				object->mInstance = ServiceManager::getSingleton().getService(msName).as<ConfigService>();
			}

			return (PyObject *)object;
		}

		// ------------------------------------------
		PyObject* ConfigServiceBinder::setParam(PyObject* self, PyObject* args) {
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			const char* name;
			const char* value;

			if (PyArg_ParseTuple(args, "ss", &name, &value)) {
				o->mInstance->setParam(name, value);
				result = Py_None;
				Py_INCREF(result);
			} else {
				PyErr_SetString(PyExc_TypeError, "Expected two string parameters!");
			}

			return result;
		}

		// ------------------------------------------
		PyObject* ConfigServiceBinder::getParam(PyObject* self, PyObject* args) {
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			const char* name;

			if (PyArg_ParseTuple(args, "s", &name)) {
				DVariant rv = o->mInstance->getParam(name);

				return DVariantToPyObject(rv);
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a string parameter!");
				return NULL;
			}
		}

		// ------------------------------------------
		PyObject* ConfigServiceBinder::hasParam(PyObject* self, PyObject* args) {
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			const char* name;

			if (PyArg_ParseTuple(args, "s", &name)) {
				bool has = o->mInstance->hasParam(name);

				PyObject* ret = has ? Py_True : Py_False;
				Py_INCREF(ret);
				return ret;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a string parameter!");
				return NULL;
			}
		}

		// ------------------------------------------
		PyObject* ConfigServiceBinder::loadParams(PyObject* self, PyObject* args) {
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			const char* fname;

			if (PyArg_ParseTuple(args, "s", &fname)) {
				o->mInstance->loadParams(fname);

				result = Py_None;
				Py_INCREF(result);
			} else {
				PyErr_SetString(PyExc_TypeError, "Expected a string parameter!");
			}

			return result;
		}

		PyMethodDef sOpdeMethods[] = {
			{NULL, NULL},
		};



		// ------------------------------------------
		PyObject* ConfigServiceBinder::repr(PyObject* self) {
			return PyString_FromFormat("<ConfigService at %p>", self);
		}
	}

} // namespace Opde

