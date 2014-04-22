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

#include "bindings.h"
#include "GUIServiceBinder.h"
#include "RelationBinder.h"

namespace Opde {

	namespace Python {

		// -------------------- GUI Service --------------------
		const char* GUIServiceBinder::msName = "GUIService";

		// ------------------------------------------
		PyTypeObject GUIServiceBinder::msType = {
			PyVarObject_HEAD_INIT(&PyType_Type, 0)
			msName,                   /* char *tp_name; */
			sizeof(GUIServiceBinder::Object),      /* int tp_basicsize; */
			0,                        // int tp_itemsize;       /* not used much */
			GUIServiceBinder::dealloc,   /* destructor tp_dealloc; */
			0,			              /* printfunc  tp_print;   */
			0,  // getattrfunc  tp_getattr; /* __getattr__ */
			0,   					  // setattrfunc  tp_setattr;  /* __setattr__ */
			0,				          // cmpfunc  tp_compare;  /* __cmp__ */
			0,			              // reprfunc  tp_repr;    /* __repr__ */
			0,				          // PyNumberMethods *tp_as_number; */
			0,                        // PySequenceMethods *tp_as_sequence; */
			0,                        // PyMappingMethods *tp_as_mapping; */
			0,			              // hashfunc tp_hash;     /* __hash__ */
			0,                        // ternaryfunc tp_call;  /* __call__ */
			0,			              // reprfunc tp_str;      /* __str__ */
			PyObject_GenericGetAttr,  // getattrofunc tp_getattro; */
			0,			              // setattrofunc tp_setattro; */
			0,			              // PyBufferProcs *tp_as_buffer; */
			0,			              // long tp_flags; */
			0,			              // char *tp_doc;  */
			0,			              // traverseproc tp_traverse; */
			0,			              // inquiry tp_clear; */
			0,			              // richcmpfunc tp_richcompare; */
			0,			              // long tp_weaklistoffset; */
			0,			              // getiterfunc tp_iter; */
			0,			              // iternextfunc tp_iternext; */
			msMethods,	              // struct PyMethodDef *tp_methods; */
			0,			              // struct memberlist *tp_members; */
			0,			              // struct getsetlist *tp_getset; */
		};

		// ------------------------------------------
		PyMethodDef GUIServiceBinder::msMethods[] = {
			{"setActive", setActive, METH_VARARGS},
			{"setVisible", setVisible, METH_VARARGS},
			{"getActiveSheet", getActiveSheet, METH_VARARGS},
			{"setActiveSheet", setActiveSheet, METH_VARARGS},
			{"createSheet", createSheet, METH_VARARGS},
			{"destroySheet", destroySheet, METH_VARARGS},
			{NULL, NULL},
		};


		// ------------------------------------------
		PyObject* GUIServiceBinder::setActive(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;

			PyObject *result = NULL;
			GUIServicePtr o;

			if (!python_cast<GUIServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			PyObject* active;

			if (PyArg_ParseTuple(args, "O", &active)) {
				o->setActive(PyObject_IsTrue(active)  == 1);

				result = Py_None;
				Py_INCREF(result);
			} else {
				PyErr_SetString(PyExc_TypeError, "Expected a bool parameter!");
			}

			return result;
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* GUIServiceBinder::setVisible(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			PyObject *result = NULL;
			GUIServicePtr o;

			if (!python_cast<GUIServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			PyObject* visible;

			if (PyArg_ParseTuple(args, "O", &visible)) {
				o->setVisible(PyObject_IsTrue(visible)  == 1);

				result = Py_None;
				Py_INCREF(result);
			} else {
				PyErr_SetString(PyExc_TypeError, "Expected a bool parameter!");
			}

			return result;
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* GUIServiceBinder::getActiveSheet(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			// TODO: Code
			PyErr_SetString(PyExc_TypeError, "NOT IMPLEMENTED YET");

			return NULL;
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* GUIServiceBinder::setActiveSheet(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			// TODO: Code
			PyErr_SetString(PyExc_TypeError, "NOT IMPLEMENTED YET");
			return NULL;
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* GUIServiceBinder::createSheet(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			// TODO: Code
			PyErr_SetString(PyExc_TypeError, "NOT IMPLEMENTED YET");
			return NULL;
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* GUIServiceBinder::destroySheet(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			// TODO: Code
			PyErr_SetString(PyExc_TypeError, "NOT IMPLEMENTED YET");
			return NULL;
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* GUIServiceBinder::create() {
			Object* object = construct(&msType);

			if (object != NULL) {
				object->mInstance = GET_SERVICE(GUIService);
			}

			return (PyObject *)object;
		}


		// ------------------------------------------
		void GUIServiceBinder::init(PyObject* module) {
			publishType(module, &msType, msName);
		}
	}

} // namespace Opde
