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
#include "DrawServiceBinder.h"
#include "DrawSheet.h"

namespace Opde {

	namespace Python {

		// -------------------- Draw Service --------------------
		const char* DrawServiceBinder::msName = "DrawService";

		// ------------------------------------------
		PyTypeObject DrawServiceBinder::msType = {
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			"opde.services.DrawService",                   // char *tp_name; */
			sizeof(DrawServiceBinder::Object),  // int tp_basicsize; */
			0,                        // int tp_itemsize;       /* not used much */
			DrawServiceBinder::dealloc,   // destructor tp_dealloc; */
			0,			              // printfunc  tp_print;   */
			DrawServiceBinder::getattr,  // getattrfunc  tp_getattr; /* __getattr__ */
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
		PyMethodDef DrawServiceBinder::msMethods[] = {
				{"createSheet", createSheet, METH_VARARGS},
				{"destroySheet", destroySheet, METH_VARARGS},
				{"getSheet", getSheet, METH_VARARGS},
				{"setActiveSheet", setActiveSheet, METH_VARARGS},
				{"createDrawSource", createDrawSource, METH_VARARGS},
				{"createRenderedImage", createRenderedImage, METH_VARARGS},
				{"createRenderedLabel", createRenderedLabel, METH_VARARGS},
				{"destroyDrawOperation", destroyDrawOperation, METH_VARARGS},
				{"createAtlas", createAtlas, METH_VARARGS},
				{"destroyAtlas", destroyAtlas, METH_VARARGS},
				{"loadFont", loadFont, METH_VARARGS},
				{"setFontPalette", setFontPalette, METH_VARARGS},
				{NULL, NULL},
		};

		// ------------------------------------------
		PyObject* DrawServiceBinder::createSheet(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			
			// argument - the name of the sheet to create, string
			const char* sname;

			if (PyArg_ParseTuple(args, "s", &sname)) {
			    DrawSheet* i = o->mInstance->createSheet(sname);
			    
			    // TODO: Sheet binding...
			    
				result = Py_None;
				Py_INCREF(result);
				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a string argument!");
				return NULL;
			}
			
			__PYTHON_EXCEPTION_GUARD_END_;
		}
		
		// ------------------------------------------
		PyObject* DrawServiceBinder::destroySheet(PyObject* self, PyObject* args) {
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			return result;	//Temporary return to fix the VC build
		}
		
		// ------------------------------------------
		PyObject* DrawServiceBinder::getSheet(PyObject* self, PyObject* args) {
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			return result;	//Temporary return to fix the VC build
		}
		
		// ------------------------------------------
		PyObject* DrawServiceBinder::setActiveSheet(PyObject* self, PyObject* args) {
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			return result;	//Temporary return to fix the VC build
		}
		// ------------------------------------------
		PyObject* DrawServiceBinder::createDrawSource(PyObject* self, PyObject* args) {
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			return result;	//Temporary return to fix the VC build
		}
		// ------------------------------------------
		PyObject* DrawServiceBinder::createRenderedImage(PyObject* self, PyObject* args) {
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			return result;	//Temporary return to fix the VC build
		}
		// ------------------------------------------
		PyObject* DrawServiceBinder::createRenderedLabel(PyObject* self, PyObject* args) {
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			return result;	//Temporary return to fix the VC build
		}
		// ------------------------------------------
		PyObject* DrawServiceBinder::destroyDrawOperation(PyObject* self, PyObject* args) {
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			return result;	//Temporary return to fix the VC build
		}
		// ------------------------------------------		
		PyObject* DrawServiceBinder::createAtlas(PyObject* self, PyObject* args) {
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			return result;	//Temporary return to fix the VC build
		}
		// ------------------------------------------
		PyObject* DrawServiceBinder::destroyAtlas(PyObject* self, PyObject* args) {
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			return result;	//Temporary return to fix the VC build
		}
		// ------------------------------------------		
		PyObject* DrawServiceBinder::loadFont(PyObject* self, PyObject* args) {
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			return result;	//Temporary return to fix the VC build
		}
		// ------------------------------------------
		PyObject* DrawServiceBinder::setFontPalette(PyObject* self, PyObject* args) {
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			return result;	//Temporary return to fix the VC build
		}
		
		// ------------------------------------------
		PyObject* DrawServiceBinder::getattr(PyObject *self, char *name) {
			return Py_FindMethod(msMethods, self, name);
		}
				
		// ------------------------------------------
		PyObject* DrawServiceBinder::repr(PyObject *self) {
			return PyString_FromFormat("<DrawService at %p>", self);
		}
		
		// ------------------------------------------
		PyObject* DrawServiceBinder::create() {
			Object* object = construct(&msType);

			if (object != NULL) {
				object->mInstance = static_pointer_cast<DrawService>(ServiceManager::getSingleton().getService(msName));
			}

			return (PyObject *)object;
		}

		// ------------------------------------------
		void DrawServiceBinder::init(PyObject* module) {
			publishType(module, &msType, msName);
		}


  	} // namespace Python
} // namespace Opde

