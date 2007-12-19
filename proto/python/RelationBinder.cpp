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
#include "RelationBinder.h"
#include "DTypeBinder.h"

namespace Opde {

	namespace Python {

		// -------------------- Link Service --------------------
		char* RelationBinder::msName = "Relation";

		// ------------------------------------------
		PyTypeObject RelationBinder::msType = {
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			msName,                   /* char *tp_name; */
			sizeof(RelationBinder::Object),      /* int tp_basicsize; */
			0,                        /* int tp_itemsize;       /* not used much */
			RelationBinder::dealloc,   /* destructor tp_dealloc; */
			0,			              /* printfunc  tp_print;   */
			RelationBinder::getattr,  /* getattrfunc  tp_getattr; /* __getattr__ */
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
		PyMethodDef RelationBinder::msMethods[] = {
			{"getID", getID, METH_NOARGS},
			{"getName", getName, METH_NOARGS},
			{"remove", remove, METH_VARARGS},
			{"create", createLink, METH_VARARGS},
			{"getLinkField", getLinkField, METH_VARARGS},
			{"setLinkField", setLinkField, METH_VARARGS},
			{NULL, NULL},
		};
		
		// ------------------------------------------
		PyObject* RelationBinder::getID(PyObject* self, PyObject* args) {
			Object* o = python_cast<Object*>(self, &msType);
			
			// Get the flavor, construct a python string, return.
			return PyInt_FromLong(o->mInstance->getID());
		}
		
		// ------------------------------------------
		PyObject* RelationBinder::getName(PyObject* self, PyObject* args) {
			Object* o = python_cast<Object*>(self, &msType);
			
			// Get the name, construct a python string, return.
			return PyString_FromString(o->mInstance->getName().c_str());
		}

		// ------------------------------------------
		PyObject* RelationBinder::remove(PyObject* self, PyObject* args) {
			Object* o = python_cast<Object*>(self, &msType);
			
			int id;

			if (PyArg_ParseTuple(args, "i", &id)) {
				o->mInstance->remove(id);

				PyObject* result = Py_None;
				Py_INCREF(result);
				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected an integer argument!");
				return NULL;
			}
		}

		// ------------------------------------------
		PyObject* RelationBinder::createLink(PyObject* self, PyObject* args) {
			Object* o = python_cast<Object*>(self, &msType);
			
			int from, to;
			PyObject* object = NULL;

			if (PyArg_ParseTuple(args, "ii|O", &from, &to, &object)) {
				link_id_t id;
				
				if (object == NULL) // No data create
				{
					id = o->mInstance->create(from, to);
				} else {
					// Wrap the DType, call with it
					DTypePtr t = DTypeBinder::extractDType(object);
					
					id = o->mInstance->create(from, to, t);
				}
				
				return PyLong_FromUnsignedLong(id);
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected two integer arguments (from, to) and optional link data values (DType)!");
				return NULL;
			}
		}

		// ------------------------------------------
		PyObject* RelationBinder::getLinkField(PyObject* self, PyObject* args) {
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			
			int id;
			const char* field;

			if (PyArg_ParseTuple(args, "is", &id, &field)) 
			{
				DVariant value;
				value = o->mInstance->getLinkField(id, field);

				result = DVariantToPyObject(value);
				return result;
			} 
			else 
			{
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a string argument!");
				return NULL;
			}
		}
		
		// ------------------------------------------
		PyObject* RelationBinder::setLinkField(PyObject* self, PyObject* args) {
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			
			int id;
			const char* field;
			PyObject* Object = NULL;

			if (PyArg_ParseTuple(args, "isO", &id, &field, &Object)) 
			{
				DVariant value;
				value = PyObjectToDVariant(Object);
				o->mInstance->setLinkField(id, field, value);

				result = Py_None;
				Py_INCREF(result);
				return result;
			} 
			else 
			{
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a string and a value!");
				return NULL;
			}
		}
		
		// ------------------------------------------
		PyObject* RelationBinder::getattr(PyObject *self, char *name) {
			return Py_FindMethod(msMethods, self, name);
		}

		// ------------------------------------------
		PyObject* RelationBinder::create(RelationPtr relation) {
			Object* object = construct(&msType);

			if (object != NULL) {
				object->mInstance = relation;
			}

			return (PyObject *)object;
		}
	}

} // namespace Opde

