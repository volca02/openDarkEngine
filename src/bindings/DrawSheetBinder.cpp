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

#include "bindings.h"
#include "DrawSheetBinder.h"
#include "DrawSheet.h"

namespace Opde {

	namespace Python {

		// -------------------- Draw Sheet --------------------
		const char* DrawSheetBinder::msName = "DrawSheet";

		// ------------------------------------------
		PyTypeObject DrawSheetBinder::msType = {
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			"opde.services.DrawSheet",                   // char *tp_name; */
			sizeof(DrawSheetBinder::Object),  // int tp_basicsize; */
			0,                        // int tp_itemsize;       /* not used much */
			DrawSheetBinder::dealloc,   // destructor tp_dealloc; */
			0,			              // printfunc  tp_print;   */
			DrawSheetBinder::getattr,  // getattrfunc  tp_getattr; /* __getattr__ */
			0,   					  // setattrfunc  tp_setattr;  /* __setattr__ */
			0,				          // cmpfunc  tp_compare;  /* __cmp__ */
			repr,			              // reprfunc  tp_repr;    /* __repr__ */
			0,				          // PyNumberMethods *tp_as_number; */
			0,                        // PySequenceMethods *tp_as_sequence; */
			0,                        // PyMappingMethods *tp_as_mapping; */
			0,			              // hashfunc tp_hash;     /* __hash__ */
			0,                        // ternaryfunc tp_call;  /* __call__ */
			0,			              // reprfunc tp_str;      /* __str__ */
			0,			              // getattrofunc tp_getattro; */
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
		PyMethodDef DrawSheetBinder::msMethods[] = {
				{"activate", activate, METH_NOARGS},
				{"deactivate", deactivate, METH_NOARGS},
				{"addDrawOperation", addDrawOperation, METH_VARARGS},
				{"removeDrawOperation", removeDrawOperation, METH_VARARGS},
				{"purge", purge, METH_NOARGS},
				{"setResolutionOverride", setResolutionOverride, METH_VARARGS},
				{"getClipRect", getClipRect, METH_VARARGS},
				{NULL, NULL}
		};

		// ------------------------------------------
		PyObject* DrawSheetBinder::activate(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			
			o->mInstance->activate();
			
			result = Py_None;
			Py_INCREF(result);
			return result;
			
			__PYTHON_EXCEPTION_GUARD_END_;
		}
		
		// ------------------------------------------
		PyObject* DrawSheetBinder::deactivate(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			
			o->mInstance->deactivate();
			
			result = Py_None;
			Py_INCREF(result);
			return result;
			
			/// TODO: Stub. Stupid return fix both
			return result;

			__PYTHON_EXCEPTION_GUARD_END_;
		}
		
		// ------------------------------------------
		PyObject* DrawSheetBinder::addDrawOperation(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			/*
			if (PyArg_ParseTuple(args, "o", &dop)) {
				// Either it is RenderedImage or RenderedLabel. Anyway:
								DrawOperation* dopc = ;

                                result = o;

                                return result;
                        } else {
                                // Invalid parameters
                                PyErr_SetString(PyExc_TypeError, "Expected a string argument!");
                                return NULL;
                        }

			*/
			/// TODO: Stub. Stupid return fix both
			return result;
				
			__PYTHON_EXCEPTION_GUARD_END_;
		}
		
		// ------------------------------------------
		PyObject* DrawSheetBinder::removeDrawOperation(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			
			/// TODO: Stub. Stupid return fix both
			return result;

			__PYTHON_EXCEPTION_GUARD_END_;
		}
		
		// ------------------------------------------
		PyObject* DrawSheetBinder::purge(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);

			/// TODO: Stub. Stupid return fix both
			return result;
			
			__PYTHON_EXCEPTION_GUARD_END_;
		}
		
		// ------------------------------------------
		PyObject* DrawSheetBinder::setResolutionOverride(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);

			/// TODO: Stub. Stupid return fix both
			return result;
			
			__PYTHON_EXCEPTION_GUARD_END_;
		}
		
		// ------------------------------------------
		PyObject* DrawSheetBinder::getClipRect(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			
			/// TODO: Stub. Stupid return fix both
			return result;
			
			__PYTHON_EXCEPTION_GUARD_END_;
		}
		
		
		
		// ------------------------------------------
		PyObject* DrawSheetBinder::getattr(PyObject *self, char *name) {
			return Py_FindMethod(msMethods, self, name);
		}
				
		// ------------------------------------------
		PyObject* DrawSheetBinder::repr(PyObject *self) {
			return PyString_FromFormat("<DrawSheet at %p>", self);
		}
		
		// ------------------------------------------
		DrawSheet* DrawSheetBinder::extract(PyObject *obj) {
			return python_cast<Object*>(obj, &msType)->mInstance;
		}
		
		// ------------------------------------------
		PyObject* DrawSheetBinder::create(DrawSheet *sh) {
			Object* object = construct(&msType);

			if (object != NULL) {
				object->mInstance = sh;
			}

			return (PyObject *)object;
		}

		// ------------------------------------------
		void DrawSheetBinder::init(PyObject* module) {
			publishType(module, &msType, msName);
		}


  	} // namespace Python
} // namespace Opde

