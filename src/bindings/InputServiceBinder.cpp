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
#include "InputServiceBinder.h"
#include "PythonCallback.h"

namespace Opde 
{

	namespace Python 
	{

		class PythonInputMessageConverter {
			public:
				PyObject* operator()(const InputEventMsg& ev) {
					PyObject* result = PyDict_New();
					
					// 1. Event type. Integer. TODO: Could use a python-side enum 
					PyDict_SetItemString(result, "event", PyInt_FromLong(ev.event));
					// 2. Command string
					PyDict_SetItemString(result, "command", PyString_FromString(ev.command.c_str()));
					// 3. Parameters to the command
					PyDict_SetItemString(result, "params", DVariantToPyObject(ev.params));
					
					return result;
				}
		};
		
		
		typedef PythonCallback<InputEventMsg, PythonInputMessageConverter> PythonInputCallback;
		typedef shared_ptr<PythonInputCallback> PythonInputCallbackPtr;

		// -------------------- Property Service --------------------
		char* InputServiceBinder::msName = "InputService";

		// ------------------------------------------
		PyTypeObject InputServiceBinder::msType = 
		{
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			msName,								/* char *tp_name; */
			sizeof(InputServiceBinder::Object), /* int tp_basicsize; */
			0,									/* int tp_itemsize;       /* not used much */
			InputServiceBinder::dealloc,		/* destructor tp_dealloc; */
			0,									/* printfunc  tp_print;   */
			InputServiceBinder::getattr,		/* getattrfunc  tp_getattr; /* __getattr__ */
			0,   								/* setattrfunc  tp_setattr;  /* __setattr__ */
			0,									/* cmpfunc  tp_compare;  /* __cmp__ */
			0,									/* reprfunc  tp_repr;    /* __repr__ */
			0,									/* PyNumberMethods *tp_as_number; */
			0,									/* PySequenceMethods *tp_as_sequence; */
			0,									/* PyMappingMethods *tp_as_mapping; */
			0,									/* hashfunc tp_hash;     /* __hash__ */
			0,									/* ternaryfunc tp_call;  /* __call__ */
			0,									/* reprfunc tp_str;      /* __str__ */
			0,									/* getattrofunc tp_getattro; */
			0,									/* setattrofunc tp_setattro; */
			0,									/* PyBufferProcs *tp_as_buffer; */
			0,									/* long tp_flags; */
			0,									/* char *tp_doc;  */
			0,									/* traverseproc tp_traverse; */
			0,									/* inquiry tp_clear; */
			0,									/* richcmpfunc tp_richcompare; */
			0,									/* long tp_weaklistoffset; */
			0,									/* getiterfunc tp_iter; */
			0,									/* iternextfunc tp_iternext; */
			msMethods,							/* struct PyMethodDef *tp_methods; */
			0,									/* struct memberlist *tp_members; */
			0,									/* struct getsetlist *tp_getset; */
		};

		// ------------------------------------------
		PyMethodDef InputServiceBinder::msMethods[] = 
		{
		    {"createBindContext", createBindContext, METH_VARARGS},
		    {"setBindContext", setBindContext, METH_VARARGS},
		    {"command", command, METH_VARARGS},
		    {"registerCommandTrap", registerCommandTrap, METH_VARARGS},
		    {"unregisterCommandTrap", unregisterCommandTrap, METH_VARARGS},
		    {NULL, NULL},
		};


		// ------------------------------------------
		PyObject* InputServiceBinder::createBindContext(PyObject* self, PyObject* args) 
		{
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			
			char* name;

			if (PyArg_ParseTuple(args, "s", &name)) {
			    o->mInstance->createBindContext(name);
				    
			    result = Py_None;
			    Py_INCREF(result);
			    return result;
			} else {
				PyErr_SetString(PyExc_TypeError, "Expected a string parameter!");
			}
				
			return result;
		}
		
		// ------------------------------------------
		PyObject* InputServiceBinder::setBindContext(PyObject* self, PyObject* args) 
		{
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			
			char* name;

			if (PyArg_ParseTuple(args, "s", &name)) {
				o->mInstance->setBindContext(name);
					
				result = Py_None;
				Py_INCREF(result);
				return result;
			} else {
				PyErr_SetString(PyExc_TypeError, "Expected a string parameter!");
			}
			
			return result;
		}

		// ------------------------------------------
		PyObject* InputServiceBinder::command(PyObject* self, PyObject* args) 
		{
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			
			char* command;

			if (PyArg_ParseTuple(args, "s", &command)) {
			    o->mInstance->command(command);
				    
			    result = Py_None;
			    Py_INCREF(result);
			    return result;
			} else {
				PyErr_SetString(PyExc_TypeError, "Expected a string parameter!");
			}
                
			return result;
		}

		// ------------------------------------------
		PyObject* InputServiceBinder::registerCommandTrap(PyObject* self, PyObject* args) 
		{
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			
			char* name;
			PyObject* callable;

			if (PyArg_ParseTuple(args, "sO", &name, &callable)) {
				try {        
					PythonInputCallbackPtr pcp = new PythonInputCallback(callable);
					
					// call the is to register the command trap
					o->mInstance->registerCommandTrap(name, pcp);
		
					result = Py_None;
					Py_INCREF(result);
					return result;
				} catch (BasicException& ex) {
					PyErr_SetString(PyExc_TypeError, "Error setting a callback, is the given argument a callable?");
					return NULL;
				}
			} else {
					PyErr_SetString(PyExc_TypeError, "Expected a string and callable parameters!");
			}
			
			return result;
		}
		
		// ------------------------------------------
		PyObject* InputServiceBinder::unregisterCommandTrap(PyObject* self, PyObject* args) 
		{
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);
			
			char* name;

			if (PyArg_ParseTuple(args, "s", &name)) {
				o->mInstance->unregisterCommandTrap(name);
		
				result = Py_None;
				Py_INCREF(result);
				return result;
			} else {
					PyErr_SetString(PyExc_TypeError, "Expected a string parameter!");
			}
			
			return result;
		}

		// ------------------------------------------
		PyObject* InputServiceBinder::getattr(PyObject *self, char *name) 
		{
			return Py_FindMethod(msMethods, self, name);
		}

		// ------------------------------------------
		PyObject* InputServiceBinder::create() 
		{
			Object* object = construct(&msType);

			if (object != NULL) 
			{
				object->mInstance = ServiceManager::getSingleton().getService(msName).as<InputService>();
			}
			return (PyObject *)object;
		}
	}

} // namespace Opde

