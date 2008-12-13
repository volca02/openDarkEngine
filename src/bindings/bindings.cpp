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
#include "ServiceBinder.h"
#include "logger.h"
#include "RootBinder.h"
#include "Root.h"
#include "StringIteratorBinder.h"
#include "DataFieldDescIteratorBinder.h"
#include "DTypeBinder.h"

namespace Opde {
	Opde::Root* Opde::PythonLanguage::msRoot = NULL;

	namespace Python {

		PyObject* DVariantToPyObject(const DVariant& inst) {
			// Do a conversion to python object from variant
			// Look for the type
			switch (inst.type()) {
				case DVariant::DV_INVALID:
					Py_INCREF(Py_None);
					return Py_None;

				case DVariant::DV_BOOL: {
					PyObject* ret = inst.toBool() ? Py_True : Py_False;
					Py_INCREF(ret);
					return ret;
				}

				case DVariant::DV_FLOAT:
					return PyFloat_FromDouble(inst.toFloat());

				case DVariant::DV_INT:
					return PyInt_FromLong(inst.toInt());

				case DVariant::DV_UINT:
					return PyInt_FromLong(inst.toUInt());

				case DVariant::DV_STRING:
					return PyString_FromString(inst.toString().c_str());

				case DVariant::DV_VECTOR: {
					// Build a touple
					const Ogre::Vector3& v = inst.toVector();
					return Py_BuildValue("[fff]", v.x, v.y, v.z);
				}

				case DVariant::DV_QUATERNION: {
					// Build a touple
					const Ogre::Quaternion& q = inst.toQuaternion();
					return Py_BuildValue("[ffff]", q.x, q.y, q.z, q.w);
				}

				default:	//All possible paths must return a value
                    PyErr_SetString(PyExc_TypeError, "Invalid DVariant type");
                    return NULL;
			}
		}

		DVariant PyObjectToDVariant(PyObject* obj) {
			// Do a conversion from python object to DVariant instance
			// Look for the type of the python object

			if (PyInt_Check(obj))
				return DVariant(static_cast<int>(PyInt_AsLong(obj)));
			else if (PyBool_Check(obj))
			{
				if(obj == Py_True)
					return DVariant((bool)true);
				else
					return DVariant((bool)false);
			}
			else if (PyFloat_Check(obj))
				return DVariant((float)PyFloat_AsDouble(obj));
			else if (PyString_Check(obj))
				return DVariant(PyString_AsString(obj));
			else if (PyModule_Check(obj))
			{
				float X, Y, Z;
				PyArg_Parse(obj, "[fff]", &X, &Y, &Z);
				return DVariant(X, Y, Z);
			}

			return DVariant(DVariant::DV_INVALID); //Py_None, or a non-handled type
		}

		// Logging methods
		// LEVELS: fatal error info debug verbose
		PyObject* Py_Log(PyObject *self, Logger::LogLevel level, PyObject* args) {
			const char* text;
			PyObject* result = NULL;

			if (PyArg_ParseTuple(args, "s", &text)) {
				Logger::getSingleton().log(level, "Python: %s", text);

				result = Py_None;
				Py_INCREF(result);
			} else {
				PyErr_SetString(PyExc_TypeError, "Expected string argument!");
			}

			return result;
		}

		PyObject* Py_Log_Fatal(PyObject *self, PyObject* args) {
			return Py_Log(self, Logger::LOG_LEVEL_FATAL, args);
		}

		PyObject* Py_Log_Error(PyObject *self, PyObject* args) {
			return Py_Log(self, Logger::LOG_LEVEL_ERROR, args);
		}

		PyObject* Py_Log_Info(PyObject *self, PyObject* args) {
			return Py_Log(self, Logger::LOG_LEVEL_INFO, args);
		}

		PyObject* Py_Log_Debug(PyObject *self, PyObject* args) {
			return Py_Log(self, Logger::LOG_LEVEL_DEBUG, args);
		}

		PyObject* Py_Log_Verbose(PyObject *self, PyObject* args) {
			return Py_Log(self, Logger::LOG_LEVEL_VERBOSE, args);
		}


		// ------------------------------------------------------------------------------------------------
		void PythonPublishedType::publishType(PyObject* containter, PyTypeObject* type, const char* name) {
			PyType_Ready(type);
			Py_INCREF(type);

			// VC does not like const to plain C calls. So we have to const_cast here
			PyModule_AddObject(containter, const_cast<char*>(name), (PyObject *)type);
		}
	} // namespace Python

	// ---- Doc Strings ----
	char* opde_log_fatal__doc__ = "log_fatal(msg)\n"
		"\tLogs a message with FATAL log level.\n"
		"@type msg: string\n"
		"@param msg: The logged string\n";

	char* opde_log_error__doc__ = "log_error(msg)\n"
		"\tLogs a message with ERROR log level.\n"
		"@type msg: string\n"
		"@param msg: The logged string\n";;

	char* opde_log_info__doc__ = "log_info(msg)\n"
		"\tLogs a message with INFO log level.\n"
		"@type msg: string\n"
		"@param msg: The logged string\n";

	char* opde_log_debug__doc__ = "log_debug(msg)\n"
		"\tLogs a message with DEBUG log level.\n"
		"@type msg: string\n"
		"@param msg: The logged string\n";

	char* opde_log_verbose__doc__ = "log_verbose(msg)\n"
		"\tLogs a message with VERBOSE (=ALL) log level.\n"
		"@type msg: string\n"
		"@param msg: The logged string\n";

	char* opde_createRoot__doc__ = "createRoot(mask)\n"
		"Creates the Opde.Root object with the specified service mask (See L{Opde.Services<Opde.Services>}).\n"
		"@type mask: number\n"
		"@param mask: Service creation mask\n"
		"@rtype: Root\n"
		"@return: A new Opde.Root object reference";

	char* opde_getRoot__doc__ = "getRoot()\n"
		"Retrieves the previously created Opde.Root object.\n"
		"@rtype: Root\n"
		"@return: A new Opde.Root object reference";

	PyMethodDef sOpdeMethods[] = {
		{"log_fatal", Python::Py_Log_Fatal, METH_VARARGS, opde_log_fatal__doc__},
		{"log_error", Python::Py_Log_Error, METH_VARARGS, opde_log_error__doc__},
		{"log_info", Python::Py_Log_Info, METH_VARARGS, opde_log_info__doc__},
		{"log_debug", Python::Py_Log_Debug, METH_VARARGS, opde_log_debug__doc__},
		{"log_verbose", Python::Py_Log_Verbose, METH_VARARGS, opde_log_verbose__doc__},
		{"createRoot", PythonLanguage::createRoot, METH_VARARGS, opde_createRoot__doc__},
		{"getRoot", PythonLanguage::getRoot, METH_NOARGS, opde_getRoot__doc__},
		{NULL, NULL},
	};

//	Ogre::Root* PythonLanguage::msRoot = NULL;

	void PythonLanguage::init(int argc, char **argv) {
		Py_Initialize();

		msRoot = NULL;

		// Create an Opde module
		PyObject* module = Py_InitModule("Opde", sOpdeMethods);

		// Call all the binders here. The result is initialized Python VM
		PyObject *servicemod = Python::ServiceBinder::init(module);
		Python::RootBinder::init(module);

		Python::StringIteratorBinder::init(module);
		Python::DataFieldDescIteratorBinder::init(module);
		Python::DTypeBinder::init(module);

		if (PyErr_Occurred()) {
			// TODO: Do something useful here, or forget it
			PyErr_Print();
			PyErr_Clear();
		}

		PySys_SetArgv(argc, argv);
	}

	void PythonLanguage::term() {
		Py_Finalize();

		// If we constructed the root, destroy it now...
		if (msRoot)
			delete msRoot;
	}

	void PythonLanguage::runScriptPtr(const char* ptr) {
	    // Is this the right way?
        PyRun_SimpleString(ptr);

        if (PyErr_Occurred()) {
			// TODO: Do something useful here, or forget it
			// TODO: PythonException(BasicException) with the PyErr string probably. Same in the init
			PyErr_Print();
			PyErr_Clear();
		}
	}

	bool PythonLanguage::runScript(const char* fname) {
		FILE *fp = fopen (fname, "rb");

		if (fp != NULL) {
			/* A short explanation:
			1. The PyRun_SimpleFile has issues with FILE* type. Results in Access Violations on Windows
			2. The PyRun_SimpleString does not handle DOS line-endings.
			*/

			std::ifstream pyfile(fname);

			if (!pyfile)
				return false;

			std::string ftxt, line;

			while (!pyfile.eof()) {
				getline(pyfile, line);
				ftxt += line;
				ftxt += '\n';
			}
			pyfile.close();

			PyRun_SimpleStringFlags(ftxt.c_str(), NULL);
		}

		if (PyErr_Occurred()) {
			PyErr_Print();
			PyErr_Clear();
			return false;
		}

		return true;
	}

	PyObject* PythonLanguage::createRoot(PyObject *self, PyObject* args) {
		// args: Module mask - unsigned long long
		PyObject *result = NULL;
		unsigned long mask;

		if (PyArg_ParseTuple(args, "l", &mask)) {
			// Create new root, wrap, return
			msRoot = new Opde::Root(mask);

			// Todo: We could also share a single object instance here PyObject - mPyRoot PyIncRef on it, return
			result = Python::RootBinder::create(msRoot);
			return result;
		} else {
			// Invalid parameters
			PyErr_SetString(PyExc_TypeError, "Expected one integer argument!");
			return NULL;
		}
	}

	PyObject* PythonLanguage::getRoot(PyObject *self, PyObject* args) {
		// args: Module mask - unsigned long long
		PyObject *result = NULL;

		if (msRoot == NULL) {
			// Invalid parameters
			// TODO: Choose a propper exception type
			PyErr_SetString(PyExc_TypeError, "Root was not yet created!");
			return NULL;
		}

		result = Python::RootBinder::create(msRoot);
		return result;
	}

} // namespace Opde
