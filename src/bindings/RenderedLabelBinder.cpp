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
#include "RenderedLabelBinder.h"
#include "RenderedLabel.h"
#include "DrawServiceBinder.h"
#include "DrawSourceBinder.h"

namespace Opde {

	namespace Python {

		// -------------------- RenderedLabel --------------------
		const char* RenderedLabelBinder::msName = "RenderedLabel";

		// ------------------------------------------
		PyTypeObject RenderedLabelBinder::msType = {
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			"opde.services.RenderedLabel",                   // char *tp_name; */
			sizeof(RenderedLabelBinder::Object),  // int tp_basicsize; */
			0,                        // int tp_itemsize;       /* not used much */
			RenderedLabelBinder::dealloc,   // destructor tp_dealloc; */
			0,			              // printfunc  tp_print;   */
			RenderedLabelBinder::getattr,  // getattrfunc  tp_getattr; /* __getattr__ */
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
			// for inheritance searches to work we need this
			Py_TPFLAGS_HAVE_CLASS,	              // long tp_flags; */
			0,			              // char *tp_doc;  */
			0,			              // traverseproc tp_traverse; */
			0,			              // inquiry tp_clear; */
			0,			              // richcmpfunc tp_richcompare; */
			0,			              // long tp_weaklistoffset; */
			0,			              // getiterfunc tp_iter; */
			0,			              // iternextfunc tp_iternext; */
			msMethods,	                      // struct PyMethodDef *tp_methods; */
			0,                                    // struct memberlist /*  *tp_members; */
            		0,                                    // struct getsetlist /* *tp_getset; */
            		// Base object type - needed for inheritance checks. Here, it is the DrawOperationBinder stub.
			&DrawOperationBinder::msType          // struct _typeobject *tp_base; 
		};

		// ------------------------------------------
		PyMethodDef RenderedLabelBinder::msMethods[] = {
			{"setLabel", setLabel, METH_VARARGS},
			{"addText", addText, METH_VARARGS},
			{"clearText", clearText, METH_NOARGS},
			{NULL, NULL}
		};
		
		// ------------------------------------------		
		PyObject* RenderedLabelBinder::setLabel(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			RenderedLabel* o;
			
			if (!python_cast<RenderedLabel*>(self, &msType, &o))
				__PY_CONVERR_RET;
			
			char *text;
			if (!PyArg_ParseTuple(args, "s", &text))
				__PY_BADPARMS_RET;
			
			o->setLabel(text);
			
			__PY_NONE_RET;
			__PYTHON_EXCEPTION_GUARD_END_;
		}
		
		// ------------------------------------------		
		PyObject* RenderedLabelBinder::addText(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			RenderedLabel* o;
			
			PyObject *col;
			char *text;
			
			if (!python_cast<RenderedLabel*>(self, &msType, &o))
				__PY_CONVERR_RET;
			
			if (!PyArg_ParseTuple(args, "sO", &text, &col))
				__PY_BADPARMS_RET;
			
			Ogre::ColourValue cv;
			
			if (!TypeInfo<Ogre::ColourValue>::fromPyObject(col, cv))
				__PY_BADPARM_RET("color");
			
			o->addText(text, cv);
			
			__PY_NONE_RET;
			__PYTHON_EXCEPTION_GUARD_END_;
		}
		
		// ------------------------------------------		
		PyObject* RenderedLabelBinder::clearText(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			RenderedLabel* o;
			
			if (!python_cast<RenderedLabel*>(self, &msType, &o))
				__PY_CONVERR_RET;
			
			o->clearText();
			
			__PY_NONE_RET;
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* RenderedLabelBinder::getattr(PyObject *self, char *name) {
			return Py_FindMethod(msMethods, self, name);
		}
				
		// ------------------------------------------
		PyObject* RenderedLabelBinder::repr(PyObject *self) {
			return PyString_FromFormat("<RenderedLabel at %p>", self);
		}
		
		// ------------------------------------------
		bool RenderedLabelBinder::extract(PyObject *obj, RenderedLabel*& tgt) {
			return python_cast<RenderedLabel*>(obj, &msType, &tgt);
		}
		
		// ------------------------------------------
		PyObject* RenderedLabelBinder::create(RenderedLabel *sh) {
			Object* object = construct(&msType);

			if (object != NULL) {
				object->mInstance = sh;
			}

			return (PyObject *)object;
		}

		// ------------------------------------------
		void RenderedLabelBinder::init(PyObject* module) {
			publishType(module, &msType, msName);
		}


  	} // namespace Python
} // namespace Opde

