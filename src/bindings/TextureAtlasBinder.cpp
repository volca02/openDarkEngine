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
#include "TextureAtlasBinder.h"
#include "DrawSourceBinder.h"

namespace Opde {

	namespace Python {

		// -------------------- TextureAtlas --------------------
		const char* TextureAtlasBinder::msName = "TextureAtlas";

		// ------------------------------------------
		PyTypeObject TextureAtlasBinder::msType = {
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			"opde.services.TextureAtlas",                   // char *tp_name; */
			sizeof(TextureAtlasBinder::Object),  // int tp_basicsize; */
			0,                        // int tp_itemsize;       /* not used much */
			TextureAtlasBinder::dealloc,   // destructor tp_dealloc; */
			0,			              // printfunc  tp_print;   */
			TextureAtlasBinder::getattr,  // getattrfunc  tp_getattr; /* __getattr__ */
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
		PyMethodDef TextureAtlasBinder::msMethods[] = {
				{"createDrawSource", createDrawSource, METH_VARARGS},
				{"getAtlasID", getAtlasID, METH_NOARGS},
				{NULL, NULL}
		};

		// ------------------------------------------
		PyObject *TextureAtlasBinder::createDrawSource(PyObject *self, PyObject *args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			
			PyObject *result = NULL;
			TextureAtlas* o;
			
			if (!python_cast<TextureAtlas*>(self, &msType, &o))
				__PY_CONVERR_RET;

			
			// argument - the name of the sheet to create, string
			const char *name, *group;

			if (PyArg_ParseTuple(args, "ss", &name, &group)) {
			    DrawSourcePtr dsp = o->createDrawSource(name, group);
			    
			    PyObject *o = DrawSourceBinder::create(dsp);
			    
				return o;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected two string arguments!");
				return NULL;
			}
		
			__PYTHON_EXCEPTION_GUARD_END_;
		}
		
		// ------------------------------------------
		PyObject *TextureAtlasBinder::getAtlasID(PyObject *self, PyObject *args) {
		__PYTHON_EXCEPTION_GUARD_BEGIN_;
			
			PyObject *result = NULL;
			TextureAtlas* o;
			
			if (!python_cast<TextureAtlas*>(self, &msType, &o))
				__PY_CONVERR_RET;

			return TypeInfo<int>::toPyObject(o->getAtlasID());
			
			__PYTHON_EXCEPTION_GUARD_END_;
		}
		
		// ------------------------------------------
		PyObject* TextureAtlasBinder::getattr(PyObject *self, char *name) {
			return Py_FindMethod(msMethods, self, name);
		}
				
		// ------------------------------------------
		PyObject* TextureAtlasBinder::repr(PyObject *self) {
			return PyString_FromFormat("<TextureAtlas at %p>", self);
		}
		
		// ------------------------------------------
		bool TextureAtlasBinder::extract(PyObject *obj, TextureAtlas*& tgt) {
			return python_cast<TextureAtlas*>(obj, &msType, &tgt);
		}
		
		// ------------------------------------------
		PyObject* TextureAtlasBinder::create(TextureAtlas*& sh) {
			Object* object = construct(&msType);

			if (object != NULL) {
				object->mInstance = sh;
			}

			return (PyObject *)object;
		}

		// ------------------------------------------
		void TextureAtlasBinder::init(PyObject* module) {
			publishType(module, &msType, msName);
		}


  	} // namespace Python
} // namespace Opde

