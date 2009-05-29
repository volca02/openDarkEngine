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
#include "InheritServiceBinder.h"
#include "InheritQueryResultBinder.h"

namespace Opde {

	namespace Python {

		// -------------------- Inherit Service --------------------
		const char* InheritServiceBinder::msName = "InheritService";

		// ------------------------------------------
		PyTypeObject InheritServiceBinder::msType = {
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			"opde.services.InheritService",                   /* char *tp_name; */
			sizeof(InheritServiceBinder::Object),      /* int tp_basicsize; */
			0,                        /* int tp_itemsize;       // not used much */
			InheritServiceBinder::dealloc,   /* destructor tp_dealloc; */
			0,			              /* printfunc  tp_print;   */
			InheritServiceBinder::getattr,  /* getattrfunc  tp_getattr; // __getattr__ */
			0,   					  /* setattrfunc  tp_setattr;  // __setattr__ */
			0,				          /* cmpfunc  tp_compare;  // __cmp__ */
			0,			              /* reprfunc  tp_repr;    // __repr__ */
			0,				          /* PyNumberMethods *tp_as_number; */
			0,                        /* PySequenceMethods *tp_as_sequence; */
			0,                        /* PyMappingMethods *tp_as_mapping; */
			0,			              /* hashfunc tp_hash;     // __hash__ */
			0,                        /* ternaryfunc tp_call;  // __call__ */
			0,			              /* reprfunc tp_str;      // __str__ */
			PyObject_GenericGetAttr,  // getattrofunc tp_getattro; */
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
		PyMethodDef InheritServiceBinder::msMethods[] = {
			{"getSources", getSources, METH_VARARGS},
			{"getTargets", getTargets, METH_VARARGS},
			{"hasTargets", hasTargets, METH_VARARGS},
			{"getArchetype", getArchetype, METH_VARARGS},
			{"setArchetype", setArchetype, METH_VARARGS},
			{"inheritsFrom", inheritsFrom, METH_VARARGS},
			{NULL, NULL},
		};

		// ------------------------------------------
		PyObject* InheritServiceBinder::getSources(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			PyObject *result = NULL;
			InheritServicePtr o;
			
			if (!python_cast<InheritServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			int objID;

			if (PyArg_ParseTuple(args, "i", &objID)) {
				InheritQueryResultPtr res = o->getSources(objID);

				result = InheritQueryResultBinder::create(res);

				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected one integer argument!");
				return NULL;
			}
			
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* InheritServiceBinder::getTargets(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			PyObject *result = NULL;
			InheritServicePtr o;
			
			if (!python_cast<InheritServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;
			
			int objID;

			if (PyArg_ParseTuple(args, "i", &objID)) {
				InheritQueryResultPtr res = o->getTargets(objID);

				result = InheritQueryResultBinder::create(res);

				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected one integer argument!");
				return NULL;
			}
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* InheritServiceBinder::hasTargets(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			InheritServicePtr o;
			
			if (!python_cast<InheritServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;
			
			int objID;

			if (PyArg_ParseTuple(args, "i", &objID)) {
				return TypeInfo<bool>::toPyObject(o->hasTargets(objID));
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected one integer argument!");
				return NULL;
			}
			__PYTHON_EXCEPTION_GUARD_END_;
		}


		// ------------------------------------------
		PyObject* InheritServiceBinder::getArchetype(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			PyObject *result = NULL;
			InheritServicePtr o;
			
			if (!python_cast<InheritServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			int id;

			if (PyArg_ParseTuple(args, "i", &id)) {
				int archID = o->getArchetype(id);

				result = PyLong_FromLong(archID);
				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected an integer argument!");
				return NULL;
			}
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* InheritServiceBinder::setArchetype(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			InheritServicePtr o;
			
			if (!python_cast<InheritServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			int id, srcid;

			if (PyArg_ParseTuple(args, "ii", &id, &srcid)) {
				o->setArchetype(id, srcid);

				__PY_NONE_RET;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected an integer argument!");
				return NULL;
			}
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* InheritServiceBinder::inheritsFrom(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			PyObject *result = NULL;
			InheritServicePtr o;
			
			if (!python_cast<InheritServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			int id, srcid;

			if (PyArg_ParseTuple(args, "ii", &id, &srcid)) {
				bool inh = o->inheritsFrom(id, srcid);

				result = inh ? Py_True : Py_False;
				Py_INCREF(result);
				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected an integer argument!");
				return NULL;
			}
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* InheritServiceBinder::getattr(PyObject *self, char *name) {
			return Py_FindMethod(msMethods, self, name);
		}

		// ------------------------------------------
		PyObject* InheritServiceBinder::create() {
			Object* object = construct(&msType);

			if (object != NULL) {
				object->mInstance = static_pointer_cast<InheritService>(ServiceManager::getSingleton().getService(msName));
			}

			return (PyObject *)object;
		}

		// ------------------------------------------
		void InheritServiceBinder::init(PyObject* module) {
			publishType(module, &msType, msName);

			InheritLinkBinder::init(module);
			InheritQueryResultBinder::init(module);
		}


        // -------------------- Link --------------------
		const char* InheritLinkBinder::msName = "InheritLink";

		// ------------------------------------------
		PyTypeObject InheritLinkBinder::msType = {
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			msName,                   /* char *tp_name; */
			sizeof(InheritLinkBinder::Object),      // int tp_basicsize; */
			0,                        /* int tp_itemsize;       // not used much */
			InheritLinkBinder::dealloc,   /* destructor tp_dealloc; */
			0,			              /* printfunc  tp_print;   */
			InheritLinkBinder::getattr,  /* getattrfunc  tp_getattr; // __getattr__ */
		};


        // ------------------------------------------
		PyObject* InheritLinkBinder::getattr(PyObject *self, char *name) {
			InheritLinkPtr o;
			if (!python_cast<InheritLinkPtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			if (o.isNull()) 
				// Just return PyNone
				__PY_NONE_RET;

			if (strcmp(name, "src") == 0) {
			    return PyLong_FromLong(o->srcID);
			} else if (strcmp(name, "dst") == 0) {
			    return PyLong_FromLong(o->dstID);
            } else if (strcmp(name, "priority") == 0) {
			    return PyLong_FromLong(o->priority);
            } else {
                PyErr_SetString(PyExc_TypeError, "Unknown attribute specified!");
            }

            return NULL;
		}

		// ------------------------------------------
		PyObject* InheritLinkBinder::create(InheritLinkPtr& link) {
			if (link.isNull()) {
			      PyErr_SetString(PyExc_TypeError, "Null link binding!");
			      return NULL;
			}

			Object* object = construct(&msType);

			if (object != NULL) {
				object->mInstance = link;
			}

			return (PyObject *)object;
		}

		// ------------------------------------------
		void InheritLinkBinder::init(PyObject* module) {
			publishType(module, &msType, msName);
		}
  	} // namespace Python
} // namespace Opde

