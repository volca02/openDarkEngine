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
#include "PropertyServiceBinder.h"
#include "StringIteratorBinder.h"
#include "DataFieldDescIteratorBinder.h"

namespace Opde
{

	namespace Python
	{

		// -------------------- Property Service --------------------
		const char* PropertyServiceBinder::msName = "PropertyService";

		const char* opde_PropertyService__doc__ = "PropertyService proxy. Service that manages properties of game objects";

		// ------------------------------------------
		PyTypeObject PropertyServiceBinder::msType =
		{
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			"opde.services.PropertyService",                   // char *tp_name; */
			sizeof(PropertyServiceBinder::Object),      /* int tp_basicsize; */
			0,                        // int tp_itemsize;       /* not used much */
			PropertyServiceBinder::dealloc,   // destructor tp_dealloc; */
			0,			              // printfunc  tp_print;   */
			PropertyServiceBinder::getattr,  // getattrfunc  tp_getattr; /* __getattr__ */
			0,   					  // setattrfunc  tp_setattr;  /* __setattr__ */
			0,				          // cmpfunc  tp_compare;  /* __cmp__ */
			0,			              // reprfunc  tp_repr;    /* __repr__ */
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
			opde_PropertyService__doc__,            // char *tp_doc;  */
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
		const char* opde_PropertyService_has__doc__ = "has(id, name)\n"
				"Detects if the given object has certain property\n"
				"@type id: integer\n"
				"@param id: Object ID\n"
				"@type name: string\n"
				"@param name: Property name\n"
				"@rtype: bool\n"
				"@return: True if the object has the specified property";

		const char* opde_PropertyService_owns__doc__ = "owns(id, name)\n"
				"Detects if the given object owns certain property (not through inheritance but directly)\n"
				"@type id: integer\n"
				"@param id: Object ID\n"
				"@type name: string\n"
				"@param name: Property name\n"
				"@rtype: bool\n"
				"@return: True if the object has the specified property";

		const char* opde_PropertyService_set__doc__ = "set(id, name, field, value)\n"
				"Sets a new value of the given objects property field\n"
				"@type id: integer\n"
				"@param id: Object ID\n"
				"@type name: string\n"
				"@param name: Property name\n"
				"@type field: string\n"
				"@param field: Property field name\n"
				"@type value: object\n"
				"@param value: New value object\n";

		const char* opde_PropertyService_get__doc__ = "set(id, name, field, value)\n"
				"Gets property field value for the specified object\n"
				"@type id: integer\n"
				"@param id: Object ID\n"
				"@type name: string\n"
				"@param name: Property name\n"
				"@type field: string\n"
				"@param field: Property field name\n"
				"@rtype: object\n"
				"@return: Property field value, or None on error\n";

		const char* opde_PropertyService_getAllPropertyNames__doc__ = "getAllPropertyNames()\n"
				"@rtype: iterable object\n"
				"@return: Returns iterable object containing all property names as strings\n";

		const char* opde_PropertyService_getPropertyFieldsDesc__doc__ = "getPropertyFieldsDesc()\n"
				"@type name: string\n"
				"@param name: Property name\n"
				"@rtype: iterable object\n"
				"@return: Returns iterable object containing all properties field names as data field description iterator\n";

		// ------------------------------------------
		PyMethodDef PropertyServiceBinder::msMethods[] =
		{
			{"has",  has, METH_VARARGS, opde_PropertyService_has__doc__},
			{"owns", owns, METH_VARARGS, opde_PropertyService_owns__doc__},
			{"set",  set, METH_VARARGS, opde_PropertyService_set__doc__},
			{"get",  get, METH_VARARGS, opde_PropertyService_get__doc__},
			{"getAllPropertyNames", getAllPropertyNames, METH_NOARGS, opde_PropertyService_getAllPropertyNames__doc__},
			{"getPropertyFieldsDesc", getPropertyFieldsDesc, METH_VARARGS, opde_PropertyService_getPropertyFieldsDesc__doc__},
			{NULL, NULL},
		};

		// ------------------------------------------
		PyObject* PropertyServiceBinder::has(PyObject* self, PyObject* args)
		{
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			PropertyServicePtr o;
			
			if (!python_cast<PropertyServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			int obj_id;
			const char* propName;

			if (PyArg_ParseTuple(args, "is", &obj_id, &propName))
				return PyBool_FromLong(o->has(obj_id, propName));
			else
			{
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected an integer and a string!");
				return NULL;
			}
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* PropertyServiceBinder::owns(PyObject* self, PyObject* args)
		{
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			PropertyServicePtr o;
			
			if (!python_cast<PropertyServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			int obj_id;
			const char* propName;

			if (PyArg_ParseTuple(args, "is", &obj_id, &propName))
				return PyBool_FromLong(o->owns(obj_id, propName));
			else
			{
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected an integer and a string!");
				return NULL;
			}
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* PropertyServiceBinder::set(PyObject* self, PyObject* args)
		{
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			PropertyServicePtr o;
			
			if (!python_cast<PropertyServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			int obj_id;
			const char* propName;
			const char* propField;
			PyObject* Object;
			DVariant value;

			if (PyArg_ParseTuple(args, "issO", &obj_id, &propName, &propField, &Object))
			{
				value = PyObjectToDVariant(Object);
				o->set(obj_id, propName, propField, value);
				
				// TODO: should indicate by Py_True/Py_False here
				__PY_NONE_RET;
			}
			else
			{
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected an integer, two strings and a DVariant!");
				return NULL;
			}
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* PropertyServiceBinder::get(PyObject* self, PyObject* args)
		{
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			PropertyServicePtr o;
			
			if (!python_cast<PropertyServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;


			int obj_id;
			const char* propName;
			const char* propField;

			if (PyArg_ParseTuple(args, "iss", &obj_id, &propName, &propField)) {
				DVariant ret;

				if (o->get(obj_id, propName, propField, ret)) {
					return DVariantToPyObject(ret);
				} else {
					PyObject* result = Py_None;
					Py_INCREF(result);
					return result;
				}

			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected an integer and two strings!");
				return NULL;
			}
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* PropertyServiceBinder::getAllPropertyNames(PyObject* self, PyObject* args)
		{
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			PropertyServicePtr o;
			
			if (!python_cast<PropertyServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;


			// wrap the returned StringIterator into StringIteratorBinder, return
			StringIteratorPtr res = o->getAllPropertyNames();

			return StringIteratorBinder::create(res);
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* PropertyServiceBinder::getPropertyFieldsDesc(PyObject* self, PyObject* args)
		{
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			PropertyServicePtr o;
			
			if (!python_cast<PropertyServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			const char* propName;

			if (PyArg_ParseTuple(args, "s", &propName))
			{
				// wrap the returned StringIterator into StringIteratorBinder, return
				DataFieldDescIteratorPtr res = o->getFieldDescIterator(propName);
				return DataFieldDescIteratorBinder::create(res);
			}

			// Invalid parameters
			PyErr_SetString(PyExc_TypeError, "Expected a string argument!");
			return NULL;
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* PropertyServiceBinder::getattr(PyObject *self, char *name)
		{
			return Py_FindMethod(msMethods, self, name);
		}

		// ------------------------------------------
		void PropertyServiceBinder::init(PyObject* module)
		{
			publishType(module, &msType, msName);

			DataFieldDescIteratorBinder::init(module);
		}

		// ------------------------------------------
		PyObject* PropertyServiceBinder::create()
		{
			Object* object = construct(&msType);

			if (object != NULL)
			{
				object->mInstance = static_pointer_cast<PropertyService>(ServiceManager::getSingleton().getService(msName));
			}
			return (PyObject *)object;
		}
	}

} // namespace Opde

