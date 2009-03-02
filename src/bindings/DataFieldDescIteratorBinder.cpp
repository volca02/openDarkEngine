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
#include "DTypeDef.h"
#include "PythonStruct.h"
#include "DataFieldDescIteratorBinder.h"


namespace Opde {

	namespace Python {
		class DataFieldDescBinder : public PythonStruct<DataFieldDesc> {
				public:
					static void init(PyObject* module) {
						// TODO: publishType(module, &msType, msName);

						// name, label, size, type, enumerator
						field("name", &DataFieldDesc::name);
						field("label", &DataFieldDesc::label);
						field("size", &DataFieldDesc::size);
						field("type", &DataFieldDesc::type);
						field("enumerator", &DataFieldDesc::enumerator);
					}
		};

		template<> const char* PythonStruct<DataFieldDesc>::msName = "DataFieldDesc";



		// -------------------- Data field desc iterator --------------------
		const char* DataFieldDescIteratorBinder::msName = "DataFieldDescIterator";

		// ------------------------------------------
		PyTypeObject DataFieldDescIteratorBinder::msType = {
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			"Opde.DataFieldDescIterator",  // char *tp_name; */
			sizeof(DataFieldDescIteratorBinder::Object),      /* int tp_basicsize; */
			0,                        // int tp_itemsize;       /* not used much */
			DataFieldDescIteratorBinder::dealloc,   // destructor tp_dealloc; */
			0,			              // printfunc  tp_print;   */
			DataFieldDescIteratorBinder::getattr,  // getattrfunc  tp_getattr; /* __getattr__ */
			0,   					  // setattrfunc  tp_setattr;  /* __setattr__ */
			0,				          // cmpfunc  tp_compare;  /* __cmp__ */
			repr,			          // reprfunc  tp_repr;    /* __repr__ */
			0,				          // PyNumberMethods *tp_as_number; */
			0,                        // PySequenceMethods *tp_as_sequence; */
			0,                        // PyMappingMethods *tp_as_mapping; */
			0,			              // hashfunc tp_hash;     /* __hash__ */
			0,                        // ternaryfunc tp_call;  /* __call__ */
			0,			              // reprfunc tp_str;      /* __str__ */
			0,			              // getattrofunc tp_getattro; */
			0,			              // setattrofunc tp_setattro; */
			0,			              // PyBufferProcs *tp_as_buffer; */
			Py_TPFLAGS_DEFAULT,       // long tp_flags; */
			0,			              // char *tp_doc;  */
			0,			              // traverseproc tp_traverse; */
			0,			              // inquiry tp_clear; */
			0,			              // richcmpfunc tp_richcompare; */
			0,			              // long tp_weaklistoffset; */
			getIterObject,            // getiterfunc tp_iter; */
			getNext,	              // iternextfunc tp_iternext; */
			msMethods,	              // struct PyMethodDef *tp_methods; */
			0,			              // struct memberlist *tp_members; */
			0,			              // struct getsetlist *tp_getset; */
		};

		// ------------------------------------------
		PyMethodDef DataFieldDescIteratorBinder::msMethods[] = {
			{NULL, NULL},
		};


		// ------------------------------------------
		PyObject* DataFieldDescIteratorBinder::getIterObject(PyObject* self) {
			Py_INCREF(self);
			return self;
		}

		// ------------------------------------------
		PyObject* DataFieldDescIteratorBinder::getNext(PyObject* self) {
			Object* o = python_cast<Object*>(self, &msType);

			// Get returnable object, advance to next.
			PyObject* next = NULL;

			if ((!o->mInstance.isNull()) && !o->mInstance->end()) {
				const DataFieldDesc& d = o->mInstance->next();
				next = DataFieldDescBinder::create(d);
			}

			return next;
		}

		// ------------------------------------------
		PyObject* DataFieldDescIteratorBinder::repr(PyObject *self) {
			return PyString_FromFormat("<DataFieldDescIterator at %p>", self);
		}


		// ------------------------------------------
		PyObject* DataFieldDescIteratorBinder::getattr(PyObject *self, char *name) {
			return Py_FindMethod(msMethods, self, name);
		}

		// ------------------------------------------
		PyObject* DataFieldDescIteratorBinder::create(const DataFieldDescIteratorPtr& result) {
			Object* object = construct(&msType);

			if (object != NULL) {
				object->mInstance = result;
			}

			return (PyObject *)object;
		}

		// ------------------------------------------
		void DataFieldDescIteratorBinder::init(PyObject* module) {
			publishType(module, &msType, msName);
			DataFieldDescBinder::init(module);
		}

	}

} // namespace Opde

