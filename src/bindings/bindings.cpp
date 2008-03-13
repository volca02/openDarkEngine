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
		
		
	} // namespace Python

	PyMethodDef sOpdeMethods[] = {
		{"log_fatal", Python::Py_Log_Fatal, METH_VARARGS},
		{"log_error", Python::Py_Log_Error, METH_VARARGS},
		{"log_info", Python::Py_Log_Info, METH_VARARGS},
		{"log_debug", Python::Py_Log_Debug, METH_VARARGS},
		{"log_verbose", Python::Py_Log_Verbose, METH_VARARGS},
		{"createRoot", PythonLanguage::createRoot, METH_VARARGS},
		{"getRoot", PythonLanguage::getRoot, METH_NOARGS},
		{NULL, NULL},
	};

//	Ogre::Root* PythonLanguage::msRoot = NULL;

	void PythonLanguage::init() {
		Py_Initialize();

		msRoot = NULL;
		
		// Create an Opde module
		PyObject* module = Py_InitModule("Opde", sOpdeMethods);

		// Call all the binders here. The result is initialized Python VM
		PyObject *servicemod = Python::ServiceBinder::init(module);
		Python::RootBinder::init(module);

		if (PyErr_Occurred()) {
			// TODO: Do something useful here, or forget it
			PyErr_Print();
			PyErr_Clear();
		}

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
	
	void PythonLanguage::runScript(const char* fname) {
		FILE *fp = fopen (fname, "r+");
		PyRun_SimpleFile(fp, fname);
		fclose(fp);
		
		if (PyErr_Occurred()) {
			// TODO: Do something useful here, or forget it
			// TODO: PythonException(BasicException) with the PyErr string probably. Same in the init
			PyErr_Print();
			PyErr_Clear();
		}
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
		unsigned long mask;
		
		if (msRoot = NULL) {
			// Invalid parameters
			// TODO: Choose a propper exception type
			PyErr_SetString(PyExc_TypeError, "Root was not yet created!");
			return NULL;
		}

		result = Python::RootBinder::create(msRoot);
		return result;
	}

} // namespace Opde
