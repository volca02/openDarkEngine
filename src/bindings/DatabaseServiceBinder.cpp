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
 *	  $Id$
 *
 *****************************************************************************/

#include "bindings.h"
#include "DatabaseServiceBinder.h"
#include "PythonCallback.h"
#include "PythonStruct.h"

namespace Opde {

	namespace Python {

		class PythonDatabaseProgressMessage : public PythonStruct<DatabaseProgressMsg> {
			public:
				static void init() {
					field("completed", &DatabaseProgressMsg::completed);
					field("totalCoarse", &DatabaseProgressMsg::totalCoarse);
					field("currentCoarse", &DatabaseProgressMsg::currentCoarse);
					field("overallFine", &DatabaseProgressMsg::overallFine);
				}
		};

		template<> char* PythonStruct<DatabaseProgressMsg>::msName = "DatabaseProgressMsg";

        // -------------------- Database Progress message --------------------
        class PythonDatabaseProgressMessageConverter {
			public:
				PyObject* operator()(const DatabaseProgressMsg& ev) {
					PyObject* result = PythonDatabaseProgressMessage::create(ev);
					return result;
				}
		};


		typedef PythonCallback<DatabaseProgressMsg, PythonDatabaseProgressMessageConverter> PythonDatabaseProgressCallback;
		typedef shared_ptr<PythonDatabaseProgressCallback> PythonDatabaseProgressCallbackPtr;

		// -------------------- Database Service --------------------
		char* DatabaseServiceBinder::msName = "DatabaseService";

		// ------------------------------------------
		PyTypeObject DatabaseServiceBinder::msType = {
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			"Opde.Services.DatabaseService",                   // char *tp_name; */
			sizeof(DatabaseServiceBinder::Object),      // int tp_basicsize; */
			0,                        // int tp_itemsize;       /* not used much */
			DatabaseServiceBinder::dealloc,   // destructor tp_dealloc; */
			0,			              // printfunc  tp_print;   */
			DatabaseServiceBinder::getattr,  // getattrfunc  tp_getattr; /* __getattr__ */
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
		PyMethodDef DatabaseServiceBinder::msMethods[] = {
			{"load", load, METH_VARARGS},
			{"loadGameSys", loadGameSys, METH_VARARGS},
			{"unload", unload, METH_NOARGS},
			{"setProgressListener", setProgressListener, METH_VARARGS},
			{"unsetProgressListener", unsetProgressListener, METH_NOARGS},
			{NULL, NULL},
		};

		// ------------------------------------------
		PyObject* DatabaseServiceBinder::load(PyObject* self, PyObject* args) {
		    PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);

			const char* fname;

			if (PyArg_ParseTuple(args, "s", &fname)) {
			    try {
                    o->mInstance->load(fname);
			    } catch (BasicException& e) {
			        PyErr_Format(PyExc_IOError, "Exception catched while trying to load mission database : %s", e.getDetails().c_str());
			        return NULL;
			    }

				result = Py_None;
				Py_INCREF(result);
				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a string argument!");
				return NULL;
			}
		}

		// ------------------------------------------
		PyObject* DatabaseServiceBinder::loadGameSys(PyObject* self, PyObject* args) {
		    PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);

			const char* fname;

			if (PyArg_ParseTuple(args, "s", &fname)) {
			    try {
                    o->mInstance->loadGameSys(fname);
			    } catch (BasicException& e) {
			        PyErr_Format(PyExc_IOError, "Exception catched while trying to load gamesys database : %s", e.getDetails().c_str());
			        return NULL;
			    }

				result = Py_None;
				Py_INCREF(result);
				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a string argument!");
				return NULL;
			}
		}

		// ------------------------------------------
        PyObject* DatabaseServiceBinder::unload(PyObject* self, PyObject* args) {
            PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);

			o->mInstance->unload();

			result = Py_None;
            Py_INCREF(result);
            return result;
		}

        // ------------------------------------------
        PyObject* DatabaseServiceBinder::setProgressListener(PyObject* self, PyObject* args) {
            PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);

			PyObject* callable;

			if (PyArg_ParseTuple(args, "O", &callable)) {
				try {
					DatabaseService::ProgressListenerPtr pcp = new PythonDatabaseProgressCallback(callable);

					// call the is to register the command trap
					o->mInstance->setProgressListener(pcp);

					result = Py_None;
					Py_INCREF(result);
					return result;
				} catch (BasicException) {
					PyErr_SetString(PyExc_TypeError, "Error setting a callback, is the given argument a callable?");
					return NULL;
				}
			} else {
					PyErr_SetString(PyExc_TypeError, "Expected a callable parameter!");
			}

			return result;
		}

        // ------------------------------------------
        PyObject* DatabaseServiceBinder::unsetProgressListener(PyObject* self, PyObject* args) {
            PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);

			o->mInstance->unsetProgressListener();

			result = Py_None;
            Py_INCREF(result);
            return result;
		}

		// ------------------------------------------
		PyObject* DatabaseServiceBinder::getattr(PyObject *self, char *name) {
			return Py_FindMethod(msMethods, self, name);
		}

		// ------------------------------------------
		PyObject* DatabaseServiceBinder::create() {
			Object* object = construct(&msType);

			if (object != NULL) {
				object->mInstance = static_pointer_cast<DatabaseService>(ServiceManager::getSingleton().getService(msName));
			}

			return (PyObject *)object;
		}

		// ------------------------------------------
		void DatabaseServiceBinder::init(PyObject* module) {
			PythonDatabaseProgressMessage::init();

			publishType(module, &msType, msName);
		}

	}

} // namespace Opde

