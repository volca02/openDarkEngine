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
#include "ConfigServiceBinder.h"

namespace Opde {

	namespace Python {

		// -------------------- Config Service --------------------
		const char* ConfigServiceBinder::msName = "ConfigService";

		const char* opde_ConfigService__doc__ = "ConfigService proxy. Service that manages engine configuration";

		// ------------------------------------------
		PyTypeObject ConfigServiceBinder::msType = {
			PyVarObject_HEAD_INIT(&PyType_Type, 0)
			const_cast<char*>("opde.services.ConfigService"),                   // char *tp_name; */
			sizeof(ConfigServiceBinder::Object),      // int tp_basicsize; */
			0,                        // int tp_itemsize;       /* not used much */
			ConfigServiceBinder::dealloc,   // destructor tp_dealloc; */
			0,			              // printfunc  tp_print;   */
			0,  // getattrfunc  tp_getattr; /* __getattr__ */
			0,   					  // setattrfunc  tp_setattr;  /* __setattr__ */
			0,				          // cmpfunc  tp_compare;  /* __cmp__ */
			repr,		              // reprfunc  tp_repr;    /* __repr__ */
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
			const_cast<char*>(opde_ConfigService__doc__),// char *tp_doc;  */
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
		const char* opde_ConfigService_setParam__doc__ = "setParam(key, value)\n"
				"Sets a new configuration parameter value\n"
				"@type key: string\n"
				"@param key: The configuration key name\n"
				"@type value: object\n"
				"@param value: the new value for the configuration key\n";

		const char* opde_ConfigService_getParam__doc__ = "getParam(key)\n"
				"Gets the value associated with the given configuration key\n"
				"@type key: string\n"
				"@param key: The configuration key name\n"
				"@rtype: object\n"
				"@return: the value associated with the key\n";

		const char* opde_ConfigService_hasParam__doc__ = "hasParam(key)\n"
				"Detects if the given configuration key is defined\n"
				"@type key: string\n"
				"@param key: The configuration key name\n"
				"@rtype: boolean\n"
				"@return: true if key defined, false otherwise\n";

		const char* opde_ConfigService_loadParams__doc__ = "loadParams(path)\n"
				"Detects if the given configuration key is defined\n"
				"@type path: string\n"
				"@param path: The configuration file name\n";

		const char* opde_ConfigService_setConfigPathOverride__doc__ = "setConfigPathOverride(path)\n"
				"Sets a path that overrides the normal process of config loading - that means the path given will be the only one config files are loaded from.\n"
				"@type path: string\n"
				"@param path: The configuration directory\n";

		// ------------------------------------------
		PyMethodDef ConfigServiceBinder::msMethods[] = {
			{"setParam", setParam, METH_VARARGS, const_cast<char*>(opde_ConfigService_setParam__doc__)},
			{"getParam", getParam, METH_VARARGS, const_cast<char*>(opde_ConfigService_getParam__doc__)},
			{"hasParam", hasParam, METH_VARARGS, const_cast<char*>(opde_ConfigService_hasParam__doc__)},
			{"loadParams", loadParams, METH_VARARGS, const_cast<char*>(opde_ConfigService_loadParams__doc__)},
			{"setConfigPathOverride", setConfigPathOverride, METH_VARARGS, const_cast<char*>(opde_ConfigService_setConfigPathOverride__doc__)},
			{NULL, NULL},
		};

		// ------------------------------------------
		PyObject* ConfigServiceBinder::create() {
			Object* object = construct(&msType);

			if (object != NULL) {
				object->mInstance = GET_SERVICE(ConfigService);
			}

			return (PyObject *)object;
		}

		// ------------------------------------------
		PyObject* ConfigServiceBinder::setParam(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;

			ConfigServicePtr o;

			if (!python_cast<ConfigServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			const char* name;
			const char* value;

			if (PyArg_ParseTuple(args, "ss", &name, &value)) {
				o->setParam(name, value);
				__PY_NONE_RET;
			} else {
				PyErr_SetString(PyExc_TypeError, "Expected two string parameters!");
				return NULL;
			}

			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* ConfigServiceBinder::getParam(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;

			ConfigServicePtr o;

			if (!python_cast<ConfigServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			const char* name;

			if (PyArg_ParseTuple(args, "s", &name)) {
				DVariant rv = o->getParam(name);

				return DVariantToPyObject(rv);
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a string parameter!");
				return NULL;
			}
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* ConfigServiceBinder::hasParam(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			ConfigServicePtr o;

			if (!python_cast<ConfigServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			const char* name;

			if (PyArg_ParseTuple(args, "s", &name)) {
				bool has = o->hasParam(name);

				PyObject* ret = has ? Py_True : Py_False;
				Py_INCREF(ret);
				return ret;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a string parameter!");
				return NULL;
			}
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* ConfigServiceBinder::loadParams(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			ConfigServicePtr o;

			if (!python_cast<ConfigServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			const char* fname;

			if (PyArg_ParseTuple(args, "s", &fname)) {
				o->loadParams(fname);

				__PY_NONE_RET;
			} else {
				PyErr_SetString(PyExc_TypeError, "Expected a string parameter!");
				return NULL;
			}
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* ConfigServiceBinder::setConfigPathOverride(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			ConfigServicePtr o;

			if (!python_cast<ConfigServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			const char* path;

			if (PyArg_ParseTuple(args, "s", &path)) {
				o->setConfigPathOverride(path);

				__PY_NONE_RET;
			} else {
				PyErr_SetString(PyExc_TypeError, "Expected a string parameter!");
				return NULL;
			}
			__PYTHON_EXCEPTION_GUARD_END_;
		}


		// ------------------------------------------
		void ConfigServiceBinder::init(PyObject* module) {
			publishType(module, &msType, msName);
		}

		// ------------------------------------------
		PyObject* ConfigServiceBinder::repr(PyObject* self) {
#ifdef IS_PY3K
			return PyBytes_FromFormat("<ConfigService at %p>", self);
#else
			return PyString_FromFormat("<ConfigService at %p>", self);
#endif
		}
	}

} // namespace Opde
