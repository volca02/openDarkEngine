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
			PyVarObject_HEAD_INIT(&PyType_Type, 0)
			"opde.services.DatabaseService",                   // char *tp_name; */
			sizeof(DatabaseServiceBinder::Object),      // int tp_basicsize; */
			0,                        // int tp_itemsize;       /* not used much */
			DatabaseServiceBinder::dealloc,   // destructor tp_dealloc; */
			0,						  // printfunc  tp_print;   */
			0,						  // getattrfunc  tp_getattr; /* __getattr__ */
			0,						  // setattrfunc  tp_setattr;  /* __setattr__ */
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
		const char* opde_DatabaseService_load__doc__ = "load(filename, mask)\n"
				"Loads a database file and the parent databases it refers to from the specified location. Clears all the previous loaded data\n"
				"@type filename: string\n"
				"@type mask: integer\n"
				"@param filename: The filename of the mission file\n"
				"@param mask: The mask to use while loading (see the DBM_ constants for more info)\n"
				"@raise IOError: if something bad happened while loading";

		const char* opde_DatabaseService_mergeLoad__doc__ = "mergeLoad(filename, mask)\n"
				"Loads a database file from the specified location, not destroying any previously loaded data (can be dangerous)\n"
				"@type filename: string\n"
				"@type mask: integer\n"
				"@param filename: The filename of the mission file\n"
				"@param mask: The mask to use while loading (see the DBM_ constants for more info)\n"
				"@raise IOError: if something bad happened while loading";

		const char* opde_DatabaseService_recursiveMergeLoad__doc__ = "recursiveMergeLoad(filename, mask)\n"
				"Loads a database file and the parent databases it refers to from the specified location, not destroying any previously loaded data (can be dangerous)\n"
				"@type filename: string\n"
				"@type mask: integer\n"
				"@param filename: The filename of the mission file\n"
				"@param mask: The mask to use while loading (see the DBM_ constants for more info)\n"
				"@raise IOError: if something bad happened while loading";

		const char* opde_DatabaseService_save__doc__ = "save(filename, mask)\n"
				"Saves a tag database file of the especified type to a specified location.\n"
				"@type filename: string\n"
				"@type mask: integer\n"
				"@param filename: The filename of the file to save\n"
				"@param mask: The mask to use while loading (see the DBM_ constants for more info)\n"
				"@raise IOError: if something bad happened while loading";

		const char* opde_DatabaseService_unload__doc__ = "unload(mask)\n"
				"unloads all data. Clears specified subset of the engine data\n"
				"@type mask: integer\n"
				"@param mask: The mask to use while clearing (see the DBM_ constants for more info)";

		const char* opde_DatabaseService_setProgressListener__doc__ = "setProgressListener(listener)\n"
				"Registers a new database listener to be called every loading step. The callable has to have one parameter, which will receive L{DatabaseProgressMsg<opde.services.DatabaseProgressMsg>}\n"
				"@type listener: callable\n"
				"@param listener: The function to call on every DB change\n";

		const char* opde_DatabaseService_unsetProgressListener__doc__ = "unsetProgressListener()\n"
				"Unsets the previously specified DB loading listner.";

		// ------------------------------------------
		PyMethodDef DatabaseServiceBinder::msMethods[] = {
			{"load", load, METH_VARARGS, opde_DatabaseService_load__doc__},
			{"mergeLoad", mergeLoad, METH_VARARGS, opde_DatabaseService_mergeLoad__doc__},
			{"recursiveMergeLoad", recursiveMergeLoad, METH_VARARGS, opde_DatabaseService_recursiveMergeLoad__doc__},
			{"save", save, METH_VARARGS, opde_DatabaseService_save__doc__},
			{"unload", unload, METH_NOARGS, opde_DatabaseService_unload__doc__},
			{"setProgressListener", setProgressListener, METH_VARARGS, opde_DatabaseService_setProgressListener__doc__},
			{"unsetProgressListener", unsetProgressListener, METH_NOARGS, opde_DatabaseService_unsetProgressListener__doc__},
			{NULL, NULL},
		};

		// ------------------------------------------
		PyObject* DatabaseServiceBinder::load(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;

			DatabaseServicePtr o;

			if (!python_cast<DatabaseServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			const char* fname;
			uint32_t mask;

			if (PyArg_ParseTuple(args, "si", &fname, &mask)) {
				o->load(fname, mask);

				__PY_NONE_RET;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a string and integer arguments!");
				return NULL;
			}

			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* DatabaseServiceBinder::mergeLoad(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;

			DatabaseServicePtr o;

			if (!python_cast<DatabaseServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			const char* fname;
			uint32_t mask;

			if (PyArg_ParseTuple(args, "si", &fname, &mask)) {
				o->mergeLoad(fname, mask);

				__PY_NONE_RET;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a string and integer arguments!");
				return NULL;
			}

			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* DatabaseServiceBinder::recursiveMergeLoad(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;

			DatabaseServicePtr o;

			if (!python_cast<DatabaseServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			const char* fname;
			uint32_t mask;

			if (PyArg_ParseTuple(args, "si", &fname, &mask)) {
				o->recursiveMergeLoad(fname, mask);

				__PY_NONE_RET;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a string and integer arguments!");
				return NULL;
			}

			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* DatabaseServiceBinder::save(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;

			DatabaseServicePtr o;

			if (!python_cast<DatabaseServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			const char* fname;
			uint32_t mask;

			if (PyArg_ParseTuple(args, "si", &fname, &mask)) {
				o->save(fname, mask);

				__PY_NONE_RET;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a string and integer arguments!");
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

			uint32_t mask;

			if (PyArg_ParseTuple(args, "i", &mask)) {
				o->unload(mask);

				__PY_NONE_RET;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected an integer argument!");
				return NULL;
			}

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

			PyModule_AddIntConstant(module, "DBM_CONCRETE", DBM_CONCRETE);
			PyModule_AddIntConstant(module, "DBM_ABSTRACT", DBM_ABSTRACT);
			PyModule_AddIntConstant(module, "DBM_MBRUSHES", DBM_MBRUSHES);
			PyModule_AddIntConstant(module, "DBM_UNKNOWN1", DBM_UNKNOWN1);
			PyModule_AddIntConstant(module, "DBM_CONCRETE_OLP", DBM_CONCRETE_OLP);
			PyModule_AddIntConstant(module, "DBM_OBJTREE_CONCRETE", DBM_OBJTREE_CONCRETE);
			PyModule_AddIntConstant(module, "DBM_MIS_DATA", DBM_MIS_DATA);
			PyModule_AddIntConstant(module, "DBM_OBJTREE_GAMESYS", DBM_OBJTREE_GAMESYS);
			PyModule_AddIntConstant(module, "DBM_COMPLETE", DBM_COMPLETE);
			PyModule_AddIntConstant(module, "DBM_FILETYPE_GAM", DBM_FILETYPE_GAM);
			PyModule_AddIntConstant(module, "DBM_FILETYPE_VBR", DBM_FILETYPE_VBR);
			PyModule_AddIntConstant(module, "DBM_FILETYPE_SAV", DBM_FILETYPE_SAV);
			PyModule_AddIntConstant(module, "DBM_FILETYPE_MIS", DBM_FILETYPE_MIS);
			PyModule_AddIntConstant(module, "DBM_FILETYPE_COW", DBM_FILETYPE_COW);
		}

	} // namespace Python

} // namespace Opde
