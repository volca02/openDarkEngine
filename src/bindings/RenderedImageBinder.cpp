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
#include "RenderedImageBinder.h"
#include "RenderedImage.h"

namespace Opde {

	namespace Python {

		// -------------------- Draw Source --------------------
		const char* RenderedImageBinder::msName = "RenderedImage";

		// ------------------------------------------
		PyTypeObject RenderedImageBinder::msType = {
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			"opde.services.RenderedImage",                   // char *tp_name; */
			sizeof(RenderedImageBinder::Object),  // int tp_basicsize; */
			0,                        // int tp_itemsize;       /* not used much */
			RenderedImageBinder::dealloc,   // destructor tp_dealloc; */
			0,			              // printfunc  tp_print;   */
			RenderedImageBinder::getattr,  // getattrfunc  tp_getattr; /* __getattr__ */
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
		PyMethodDef RenderedImageBinder::msMethods[] = {
			{"setPosition", setPosition, METH_VARARGS},
			{"setZOrder", setZOrder, METH_VARARGS},
			{"setClipRect", setClipRect, METH_VARARGS},			
			{NULL, NULL}
		};
		
		
		// ------------------------------------------
		PyObject* RenderedImageBinder::setPosition(PyObject* self, PyObject* args)  {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
                
                        PyObject *result = NULL;
                        Object* o = python_cast<Object*>(self, &msType);
                  
                        int x, y;
                        
                        if (PyArg_ParseTuple(args, "ii", &x, &y)) {
                       		o->mInstance->setPosition(x, y);
                        
				result = Py_None;
                        	Py_INCREF(result);
                                return result;
                        } else {
                                // Invalid parameters
                                PyErr_SetString(PyExc_TypeError, "Expected two integer arguments!");
                                return NULL;
                        }
                                
                        __PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* RenderedImageBinder::setZOrder(PyObject* self, PyObject* args)  {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
                
                        PyObject *result = NULL;
                        Object* o = python_cast<Object*>(self, &msType);
                  
                        int z;
                        
                        if (PyArg_ParseTuple(args, "i", &z)) {
                       		o->mInstance->setZOrder(z);
                        
				result = Py_None;
                        	Py_INCREF(result);
                                return result;
                        } else {
                                // Invalid parameters
                                PyErr_SetString(PyExc_TypeError, "Expected an integer argument!");
                                return NULL;
                        }
                                
                        __PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* RenderedImageBinder::setClipRect(PyObject* self, PyObject* args)  {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
                
                        PyObject *result = NULL;
                        Object* o = python_cast<Object*>(self, &msType);
                  
			PyObject *cr;
                        
                        if (PyArg_ParseTuple(args, "o", &cr)) {
				// if it's a tuple, it should contain four floats
				ClipRect rect;
				if (!msRectTypeInfo.fromPyObject(cr, rect)) {
					PyErr_SetString(PyExc_TypeError, "Expected 4 float tuple!");
	                                return NULL;
				};
				
				o->mInstance->setClipRect(rect);
                        
				result = Py_None;
                        	Py_INCREF(result);
                                return result;
                        } else {
                                // Invalid parameters
                                PyErr_SetString(PyExc_TypeError, "Expected an integer argument!");
                                return NULL;
                        }
                                
                        __PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* RenderedImageBinder::getattr(PyObject *self, char *name) {
			// TODO: Image/Group name getters?
			return Py_FindMethod(msMethods, self, name);
		}
				
		// ------------------------------------------
		PyObject* RenderedImageBinder::repr(PyObject *self) {
			return PyString_FromFormat("<RenderedImage at %p>", self);
		}
		
		// ------------------------------------------
		RenderedImage* RenderedImageBinder::extract(PyObject *obj) {
			return python_cast<Object*>(obj, &msType)->mInstance;
		}
		
		// ------------------------------------------
		PyObject* RenderedImageBinder::create(RenderedImage *sh) {
			Object* object = construct(&msType);

			if (object != NULL) {
				object->mInstance = sh;
			}

			return (PyObject *)object;
		}

		// ------------------------------------------
		void RenderedImageBinder::init(PyObject* module) {
			publishType(module, &msType, msName);
		}


  	} // namespace Python
} // namespace Opde

