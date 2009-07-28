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
 *	  $Id$
 *
 *****************************************************************************/

#include "bindings.h"
#include "DatabaseServiceBinder.h"
#include "PythonCallback.h"
#include "PythonStruct.h"

namespace Opde {

	namespace Python {

		const char* opde_PythonDatabaseProgressMessage__doc__ = "Database Progress Message structure. Structure informing about loading/saving progress.\n"
			"@ivar completed: Overall 0.0-1.0 progress\n"
			"@ivar totalCoarse: Coarse steps total\n"
			"@ivar currentCoarse: Coarse steps current\n"
			"@ivar overallFine: Current count of the fine steps\n";

		class PythonDatabaseProgressMessage : public PythonStruct<DatabaseProgressMsg> {
			public:
				static void init(PyObject* container) {
					field("completed", &DatabaseProgressMsg::completed);
					field("totalCoarse", &DatabaseProgressMsg::totalCoarse);
					field("currentCoarse", &DatabaseProgressMsg::currentCoarse);
					field("overallFine", &DatabaseProgressMsg::overallFine);

					publish(opde_PythonDatabaseProgressMessage__doc__, "opde.services.DatabaseProgressMsg", container);
				}
		};

		template<> const char* PythonStruct<DatabaseProgressMsg>::msName = "DatabaseProgressMsg";

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
		const char* DatabaseServiceBinder::msName = "DatabaseService";

		const char* opde_DatabaseService__doc__ = "Database service. Used to load/save game state";

		// ------------------------------------------
		PyTypeObject DatabaseServiceBinder::msType = {
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			"opde.services.DatabaseService",                   // char *tp_name; */
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
			PyObject_GenericGetAttr,  // getattrofunc tp_getattro; */
			0,			              // setattrofunc tp_setattro; */
			0,			              // PyBufferProcs *tp_as_buffer; */
			0,			              // long tp_flags; */
			opde_DatabaseService__doc__,  // char *tp_doc;  */
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
		const char* opde_DatabaseService_load__doc__ = "load(filename)\n"
				"Loads a mission file (.mis) from the specified location. Clears all the previous loaded data\n"
				"@type filename: string\n"
				"@param filename: The filename of the mission file\n"
				"@raise IOError: if something bad happened while loading";

		const char* opde_DatabaseService_loadGameSys__doc__ = "loadGameSys(filename)\n"
				"Loads a game database file (.gam) from the specified location. Clears all the previous loaded data\n"
				"@type filename: string\n"
				"@param filename: The filename of the mission file\n"
				"@raise IOError: if something bad happened while loading";

		const char* opde_DatabaseService_unload__doc__ = "unload()\n"
				"unloads all data. Clears everything\n";

		const char* opde_DatabaseService_setProgressListener__doc__ = "setProgressListener(listener)\n"
				"Registers a new database listener to be called every loading step. The callable has to have one parameter, which will receive L{DatabaseProgressMsg<opde.services.DatabaseProgressMsg>}\n"
				"@type listener: callable\n"
				"@param listener: The function to call on every DB change\n";

		const char* opde_DatabaseService_unsetProgressListener__doc__ = "unsetProgressListener()\n"
				"Unsets the previously specified DB loading listner.";

		// ------------------------------------------
		PyMethodDef DatabaseServiceBinder::msMethods[] = {
			{"load", load, METH_VARARGS, opde_DatabaseService_load__doc__},
			{"loadGameSys", loadGameSys, METH_VARARGS, opde_DatabaseService_loadGameSys__doc__},
			{"unload", unload, METH_NOARGS, opde_DatabaseService_unload__doc__},
			{"setProgressListener", setProgressListener, METH_VARARGS, opde_DatabaseService_setProgressListener__doc__},
			{"unsetProgressListener", unsetProgressListener, METH_NOARGS, opde_DatabaseService_unsetProgressListener__doc__},
			{NULL, NULL},
		};

		// ------------------------------------------
		PyObject* DatabaseServiceBinder::load(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			
			PyObject *result = NULL;
			DatabaseServicePtr o;
			
			if (!python_cast<DatabaseServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			const char* fname;

			if (PyArg_ParseTuple(args, "s", &fname)) {
				o->load(fname);
				
				result = Py_None;
				Py_INCREF(result);
				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a string argument!");
				return NULL;
			}
			
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* DatabaseServiceBinder::loadGameSys(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			
			DatabaseServicePtr o;
			
			if (!python_cast<DatabaseServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			const char* fname;

			if (PyArg_ParseTuple(args, "s", &fname)) {
				o->loadGameSys(fname);
				
				__PY_NONE_RET;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a string argument!");
				return NULL;
			}
			
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* DatabaseServiceBinder::unload(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;        	
			DatabaseServicePtr o;
			
			if (!python_cast<DatabaseServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			o->unload();

			__PY_NONE_RET;
			
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* DatabaseServiceBinder::setProgressListener(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			
			PyObject *result = NULL;
			DatabaseServicePtr o;
			
			if (!python_cast<DatabaseServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			PyObject* callable;

			if (PyArg_ParseTuple(args, "O", &callable)) {
				DatabaseService::ProgressListenerPtr pcp(new PythonDatabaseProgressCallback(callable));

				// call the is to register the command trap
				o->setProgressListener(pcp);

				__PY_NONE_RET;
			} else {
					PyErr_SetString(PyExc_TypeError, "Expected a callable parameter!");
			}

			return result;
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* DatabaseServiceBinder::unsetProgressListener(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			
			DatabaseServicePtr o;
			
			if (!python_cast<DatabaseServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			o->unsetProgressListener();

			__PY_NONE_RET;
			
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* DatabaseServiceBinder::getattr(PyObject *self, char *name) {
			return Py_FindMethod(msMethods, self, name);
		}

		// ------------------------------------------
		PyObject* DatabaseServiceBinder::create() {
			Object* object = construct(&msType);

			if (object != NULL) {
				object->mInstance = GET_SERVICE(DatabaseService);
			}

			return (PyObject *)object;
		}

		// ------------------------------------------
		void DatabaseServiceBinder::init(PyObject* module) {
			PythonDatabaseProgressMessage::init(module);

			publishType(module, &msType, msName);
		}

	} // namespace Python

} // namespace Opde

