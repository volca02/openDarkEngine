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
#include "FontDrawSourceBinder.h"

namespace Opde {

	namespace Python {

		// -------------------- Draw Source --------------------
		const char* FontDrawSourceBinder::msName = "FontDrawSource";

		// ------------------------------------------
		PyTypeObject FontDrawSourceBinder::msType = {
			PyVarObject_HEAD_INIT(&PyType_Type, 0)
			"opde.services.FontDrawSource",                   // char *tp_name; */
			sizeof(FontDrawSourceBinder::Object),  // int tp_basicsize; */
			0,                        // int tp_itemsize;       /* not used much */
			FontDrawSourceBinder::dealloc,   // destructor tp_dealloc; */
			0,			              // printfunc  tp_print;   */
			0,  // getattrfunc  tp_getattr; /* __getattr__ */
			0,   					  // setattrfunc  tp_setattr;  /* __setattr__ */
			0,				          // cmpfunc  tp_compare;  /* __cmp__ */
			repr,			              // reprfunc  tp_repr;    /* __repr__ */
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
		PyMethodDef FontDrawSourceBinder::msMethods[] = {
				{NULL, NULL}
		};

		// ------------------------------------------
		PyObject* FontDrawSourceBinder::repr(PyObject *self) {
#ifdef IS_PY3K
			return PyBytes_FromFormat("<FontDrawSource at %p>", self);
#else
			return PyString_FromFormat("<FontDrawSource at %p>", self);
#endif
		}

		// ------------------------------------------
		bool FontDrawSourceBinder::extract(PyObject *obj, FontDrawSourcePtr& tgt) {
			return python_cast<FontDrawSourcePtr>(obj, &msType, &tgt);
		}

		// ------------------------------------------
		PyObject* FontDrawSourceBinder::create(const FontDrawSourcePtr& sh) {
			Object* object = construct(&msType);

			if (object != NULL) {
				object->mInstance = sh;
			}

			return (PyObject *)object;
		}

		// ------------------------------------------
		void FontDrawSourceBinder::init(PyObject* module) {
			publishType(module, &msType, msName);
		}


  	} // namespace Python
} // namespace Opde
