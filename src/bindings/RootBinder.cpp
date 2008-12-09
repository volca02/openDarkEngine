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
#include "RootBinder.h"

namespace Opde {

	namespace Python {

		// -------------------- Link Service --------------------
		char* RootBinder::msName = "Root";

		const char* opde_Root__doc__ = "The root object of the openDarkEngine API. Exposes some common objects/methods.\n";

		// ------------------------------------------
		PyTypeObject RootBinder::msType = {
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			"Opde.Root",                   /* char *tp_name; */
			sizeof(RootBinder::Object),      /* int tp_basicsize; */
			0,                        // int tp_itemsize;       /* not used much */
			RootBinder::dealloc,   /* destructor tp_dealloc; */
			0,			              /* printfunc  tp_print;   */
			RootBinder::getattr,  // getattrfunc  tp_getattr; /* __getattr__ */
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
			Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,       // long tp_flags; */
			opde_Root__doc__,			              // char *tp_doc;  */
			0,			              // traverseproc tp_traverse; */
			0,			              // inquiry tp_clear; */
			0,			              // richcmpfunc tp_richcompare; */
			0,			              // long tp_weaklistoffset; */
			0,				          // getiterfunc tp_iter; */
			0,	            		  // iternextfunc tp_iternext; */
			msMethods,	              // struct PyMethodDef *tp_methods; */
			0,			              // struct memberlist *tp_members; */
			0,			              // struct getsetlist *tp_getset; */
			0,						  // struct _typeobject *tp_base; */
			0,						  // PyObject *tp_dict; */
			0,						  // descrgetfunc tp_descr_get; */
			0,						  // descrsetfunc tp_descr_set; */
			0,						  // long tp_dictoffset; */
			0,						  // initproc tp_init; */
			0						  // allocfunc tp_alloc; */
		};

		// ------ Doc Strings -----------------------
		const char* opde_Root_loadResourceConfig__doc__ = "loadResourceConfig(path)\n"
						"Loads a Ogre's resource config file (resources.cfg type) from the specified path\n"
						"@type path: string\n"
						"@param path: Path to the resource config file to be loaded\n";

		const char* opde_Root_loadDTypeScript__doc__ = "loadDTypeScript(path, group)\n"
						"Loads a DType (Dynamic type specifications) file from the specified path\n"
						"@type path: string\n"
						"@param path: Path to the dtype file to be loaded\n"
						"@type group: string\n"
						"@param group: Resource group from which to load\n";

		const char* opde_Root_loadPLDefScript__doc__ = "loadPLDefScript(path)\n"
						"Loads a PLDef (Property/Link definitions) file from the specified path\n"
						"@type path: string\n"
						"@param path: Path to the pldef file to be loaded\n"
						"@type group: string\n"
						"@param group: Resource group from which to load\n";

		const char* opde_Root_loadConfigFile__doc__ = "loadConfigFile(path)\n"
						"Loads a Opde's config file (opde.cfg type) from the specified path\n"
						"@type path: string\n"
						"@param path: Path to the Opde's config file to be loaded\n";

		const char* opde_Root_addResourceLocation__doc__ = "addResourceLocation(name,type,section[,recursive])\n"
						"Adds a resource location to the specified section (group)\n"
						"@type name: string\n"
						"@param name: Resource location name\n"
						"@type type: string\n"
						"@param type: Resource location type (FileSystem, Zip, Crf, Dir,...)\n"
						"@type section: string\n"
						"@param section: The section (resource group) to add to\n"
						"@type recursive: bool\n"
						"@param recursive: Optional parameter specifying if subdirectories should be searched\n;";

		const char* opde_Root_removeResourceLocation__doc__ = "removeResourceLocation(name,section)\n"
						"Removes resource location from the specified section (group)\n"
						"@type name: string\n"
						"@param name: Resource location name\n"
						"@type section: string\n"
						"@param section: The section (resource group) to remove from\n";

		const char* opde_Root_bootstrapFinished__doc__ = "bootstrapFinished()\n"
						"Informs Opde that the resource setup has been done and opde can start.";

		const char* opde_Root_logToFile__doc__ = "logToFile(filename)\n"
						"Removes resource location from the specified section (group)\n"
						"@type filename: string\n"
						"@param filename: Resource location name\n";

		const char* opde_Root_setLogLevel__doc__ = "setLogLevel(level)\n"
						"Removes resource location from the specified section (group)\n"
						"@type level: integer\n"
						"@param level: The desired log level\n";

		// ------------------------------------------
		PyMethodDef RootBinder::msMethods[] = {
			{"loadResourceConfig", RootBinder::loadResourceConfig, METH_VARARGS, opde_Root_loadResourceConfig__doc__},
			{"loadDTypeScript", RootBinder::loadDTypeScript, METH_VARARGS, opde_Root_loadDTypeScript__doc__},
			{"loadPLDefScript", RootBinder::loadPLDefScript, METH_VARARGS, opde_Root_loadPLDefScript__doc__},
			{"loadConfigFile", RootBinder::loadConfigFile, METH_VARARGS, opde_Root_loadConfigFile__doc__},
			{"addResourceLocation", RootBinder::addResourceLocation, METH_VARARGS, opde_Root_addResourceLocation__doc__},
			{"removeResourceLocation", RootBinder::removeResourceLocation, METH_VARARGS, opde_Root_removeResourceLocation__doc__},
			{"bootstrapFinished", RootBinder::bootstrapFinished, METH_NOARGS, opde_Root_bootstrapFinished__doc__},
			{"logToFile", RootBinder::logToFile, METH_VARARGS, opde_Root_logToFile__doc__},
			{"setLogLevel", RootBinder::setLogLevel, METH_VARARGS, opde_Root_setLogLevel__doc__},
			{NULL, NULL},
		};

		// ------------------------------------------
		PyObject* RootBinder::loadResourceConfig(PyObject *self, PyObject* args) {
			// ARGS: string
			Object* o = python_cast<Object*>(self, &msType);

			const char* fname;

			if (PyArg_ParseTuple(args, "s", &fname)) {
			    try {
                    o->mInstance->loadResourceConfig(fname);
			    } catch (BasicException& e) {
			        PyErr_Format(PyExc_IOError, "Exception catched while trying to load resource config : %s", e.getDetails().c_str());
			        return NULL;
			    } catch (...) {
			    	PyErr_Format(PyExc_IOError, "Exception catched while trying to load resource config");
			        return NULL;
			    }

				PyObject *result = Py_None;
				Py_INCREF(result);
				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a string argument!");
				return NULL;
			}
		}

		// ------------------------------------------
		PyObject* RootBinder::loadDTypeScript(PyObject *self, PyObject* args) {
			// ARGS: string, string
			Object* o = python_cast<Object*>(self, &msType);

			const char *fname, *gname;

			if (PyArg_ParseTuple(args, "ss", &fname, &gname)) {
			    try {
                    o->mInstance->loadDTypeScript(fname, gname);
			    } catch (BasicException& e) {
			        PyErr_Format(PyExc_IOError, "Exception catched while trying to load DType script : %s", e.getDetails().c_str());
			        return NULL;
			    } catch (...) {
			    	PyErr_Format(PyExc_IOError, "Exception catched while trying to load DType script");
			        return NULL;
			    }

				PyObject *result = Py_None;
				Py_INCREF(result);
				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected two string arguments!");
				return NULL;
			}
		}

		// ------------------------------------------
		PyObject* RootBinder::loadPLDefScript(PyObject *self, PyObject* args) {
			// ARGS: string, string
			Object* o = python_cast<Object*>(self, &msType);

			const char  *fname, *gname;

			if (PyArg_ParseTuple(args, "ss", &fname, &gname)) {
			    try {
                    o->mInstance->loadPLDefScript(fname, gname);
			    } catch (BasicException& e) {
			        PyErr_Format(PyExc_IOError, "Exception catched while trying to load PLDef script : %s", e.getDetails().c_str());
			        return NULL;
			    } catch (...) {
			    	PyErr_Format(PyExc_IOError, "Exception catched while trying to load PLDef script");
			        return NULL;
			    }

				PyObject *result = Py_None;
				Py_INCREF(result);
				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected two string arguments!");
				return NULL;
			}
		}

		// ------------------------------------------
		PyObject* RootBinder::loadConfigFile(PyObject *self, PyObject* args) {
			// ARGS: string
			Object* o = python_cast<Object*>(self, &msType);

			const char *fname;

			if (PyArg_ParseTuple(args, "s", &fname)) {
			    try {
                    o->mInstance->loadConfigFile(fname);
			    } catch (BasicException& e) {
			        PyErr_Format(PyExc_IOError, "Exception catched while trying to load config : %s", e.getDetails().c_str());
			        return NULL;
			    } catch (...) {
			    	PyErr_Format(PyExc_IOError, "Exception catched while trying to load config");
			        return NULL;
			    }

				PyObject *result = Py_None;
				Py_INCREF(result);
				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a string argument!");
				return NULL;
			}
		}

		// ------------------------------------------
		PyObject* RootBinder::addResourceLocation(PyObject *self, PyObject* args) {
			// ARGS: string, string, string, bool
			Object *o = python_cast<Object*>(self, &msType);

			const char *name, *type, *section;
			PyObject *recursive = Py_False;

			if (PyArg_ParseTuple(args, "sss|O", &name, &type, &section, &recursive)) {
				try {
					bool recb = false;
					if (PyBool_Check(recursive)) {
						recb = (recursive == Py_True);
					} else {
						PyErr_Format(PyExc_TypeError, "The optional argument is expected to be Bool");
						return NULL;
					}

					o->mInstance->addResourceLocation(name, type, section, recb);
				} catch (BasicException& e) {
					PyErr_Format(PyExc_IOError, "Exception catched while trying to add resource location : %s", e.getDetails().c_str());
					return NULL;
				} catch (...) {
					PyErr_Format(PyExc_IOError, "Exception catched while trying to add resource location");
					return NULL;
				}

				PyObject *result = Py_None;
				Py_INCREF(result);
				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected three string arguments and one optional bool!");
				return NULL;
			}
		}

		// ------------------------------------------
		PyObject* RootBinder::removeResourceLocation(PyObject *self, PyObject* args) {
			// ARGS: string, string
			Object* o = python_cast<Object*>(self, &msType);

			const char *name, *section;

			if (PyArg_ParseTuple(args, "ss", &name, &section)) {
			    try {
                    o->mInstance->removeResourceLocation(name, section);
			    } catch (BasicException& e) {
			        PyErr_Format(PyExc_IOError, "Exception catched while trying to remove a resource location : %s", e.getDetails().c_str());
			        return NULL;
			    } catch (...) {
			    	PyErr_Format(PyExc_IOError, "Exception catched while trying to remove resource location");
			        return NULL;
			    }

				PyObject *result = Py_None;
				Py_INCREF(result);
				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected two string arguments!");
				return NULL;
			}
		}

		// ------------------------------------------
		PyObject* RootBinder::bootstrapFinished(PyObject *self, PyObject* args) {
			// ARGS: No arguments!
			Object* o = python_cast<Object*>(self, &msType);

			try {
				o->mInstance->bootstrapFinished();
			} catch (BasicException& e) {
				PyErr_Format(PyExc_IOError, "Exception catched while finishing bootstrap : %s", e.getDetails().c_str());
				return NULL;
			} catch (...) {
				PyErr_Format(PyExc_IOError, "Exception catched while finishing bootstrap");
				return NULL;
			}

			PyObject *result = Py_None;
			Py_INCREF(result);
			return result;
		}

		// ------------------------------------------
		PyObject* RootBinder::logToFile(PyObject *self, PyObject* args) {
			// ARGS: string
			Object* o = python_cast<Object*>(self, &msType);

			const char* fname;

			if (PyArg_ParseTuple(args, "s", &fname)) {
			    try {
                    o->mInstance->logToFile(fname);
			    } catch (BasicException& e) {
			        PyErr_Format(PyExc_IOError, "Exception catched while trying to setup file logging : %s", e.getDetails().c_str());
			        return NULL;
			    } catch (...) {
			    	PyErr_Format(PyExc_IOError, "Exception catched while trying to setup file logging");
			        return NULL;
			    }

				PyObject *result = Py_None;
				Py_INCREF(result);
				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a string argument!");
				return NULL;
			}
		}

		// ------------------------------------------
		PyObject* RootBinder::setLogLevel(PyObject *self, PyObject* args) {
			// ARGS: string
			Object* o = python_cast<Object*>(self, &msType);

			int level;

			if (PyArg_ParseTuple(args, "i", &level)) {
                o->mInstance->setLogLevel(level); // does not throw

				PyObject *result = Py_None;
				Py_INCREF(result);
				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected an integer argument!");
				return NULL;
			}
		}

		// ------------------------------------------
		PyObject* RootBinder::repr(PyObject *self) {
			return PyString_FromFormat("<Opde::Root at %p>", self);
		}


		// ------------------------------------------
		PyObject* RootBinder::getattr(PyObject *self, char *name) {
			return Py_FindMethod(msMethods, self, name);
		}

		// ------------------------------------------
		PyObject* RootBinder::create(Root* root) {
			Object* object = construct(&msType);

			if (object != NULL) {
				object->mInstance = root;
			}

			return (PyObject *)object;
		}


		// ------------------------------------------
		void RootBinder::init(PyObject* module) {
			// Register root in the Opde module
			publishType(module, &msType, msName);
		}
	}

} // namespace Opde

