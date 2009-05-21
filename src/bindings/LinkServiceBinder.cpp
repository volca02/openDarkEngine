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
#include "LinkServiceBinder.h"
#include "RelationBinder.h"
#include "LinkQueryResultBinder.h"
#include "StringIteratorBinder.h"
#include "DataFieldDescIteratorBinder.h"

namespace Opde {

	namespace Python {

		// -------------------- Link Service --------------------
		const char* LinkServiceBinder::msName = "LinkService";

		// ------------------------------------------
		PyTypeObject LinkServiceBinder::msType = {
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			"opde.services.LinkService",                   // char *tp_name; */
			sizeof(LinkServiceBinder::Object),  // int tp_basicsize; */
			0,                        // int tp_itemsize;       /* not used much */
			LinkServiceBinder::dealloc,   // destructor tp_dealloc; */
			0,			              // printfunc  tp_print;   */
			LinkServiceBinder::getattr,  // getattrfunc  tp_getattr; /* __getattr__ */
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
		PyMethodDef LinkServiceBinder::msMethods[] = {
			{"setChunkVersion", setChunkVersion, METH_VARARGS},
			{"nameToFlavor", nameToFlavor, METH_VARARGS},
			{"flavorToName", flavorToName, METH_VARARGS},
			{"getRelation", getRelation, METH_VARARGS},
			{"getAllLinks", getAllLinks, METH_VARARGS},
			{"getAllInherited", getAllInherited, METH_VARARGS},
			{"getOneLink", getOneLink, METH_VARARGS},
			{"getAllLinkNames", getAllLinkNames, METH_NOARGS},
			{"getFieldsDesc", getFieldsDesc, METH_VARARGS},
			{NULL, NULL},
		};

		// ------------------------------------------
		PyObject* LinkServiceBinder::setChunkVersion(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			PyObject *result = NULL;
			LinkServicePtr o;
			
			if (!python_cast<LinkServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			int major;
			int minor;

			if (PyArg_ParseTuple(args, "ii", &major, &minor)) {
				o->setChunkVersion(major, minor);

                result = Py_None;
				Py_INCREF(result);
				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected two integer arguments!");
				return NULL;
			}
			
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* LinkServiceBinder::nameToFlavor(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			PyObject *result = NULL;
			LinkServicePtr o;
			
			if (!python_cast<LinkServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;
			
			const char* name;

			if (PyArg_ParseTuple(args, "s", &name)) {
				const int res = o->nameToFlavor(name);

				result = PyInt_FromLong(res);
				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a string argument!");
				return NULL;
			}
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* LinkServiceBinder::flavorToName(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			PyObject *result = NULL;
			LinkServicePtr o;
			
			if (!python_cast<LinkServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			int id;

			if (PyArg_ParseTuple(args, "i", &id)) {
				const std::string& res = o->flavorToName(id);

				result = PyString_FromString(res.c_str());
				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected an integer argument!");
				return NULL;
			}
			__PYTHON_EXCEPTION_GUARD_END_;
		}
		

		// ------------------------------------------
		PyObject* LinkServiceBinder::getRelation(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			PyObject *result = NULL;
			LinkServicePtr o;
			
			if (!python_cast<LinkServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			PyObject* object;
			if (!PyArg_ParseTuple(args, "O", &object)) {
				PyErr_SetString(PyExc_TypeError, "Expected an integer or string argument!");
				return NULL;
			}

			// two possibilities here : name or flavor
			if (PyString_Check(object)) {
			    char* str = PyString_AsString(object);
				RelationPtr rel = o->getRelation(str);

                if (rel.isNull()) {
                    PyErr_Format(PyExc_ValueError, "Relation not found by name %s", str);
                } else
                    result = RelationBinder::create(rel);

				return result;
			} else if (PyInt_Check(object)) {
			    long id = PyInt_AsLong(object);
				RelationPtr rel = o->getRelation(static_cast<int>(id));

                if (rel.isNull()) {
                    PyErr_Format(PyExc_ValueError, "Relation not found by id %ld", id);
                } else
                    result = RelationBinder::create(rel);

				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected an integer or string argument!");
				return NULL;
			}
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* LinkServiceBinder::getAllLinks(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			LinkServicePtr o;
			
			if (!python_cast<LinkServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;
			
			int flavor = 0, src, dst;
			PyObject* objflav;

			// let the third parameter be either string or integer
			// if it's a string, we first have to parse the string to get flavor id

			if (PyArg_ParseTuple(args, "Oii", &objflav, &src, &dst)) {
				if (PyString_Check(objflav)) {
					char* str = PyString_AsString(objflav);
					flavor = o->nameToFlavor(str);
				} else if (PyInt_Check(objflav)) {
					flavor = PyInt_AsLong(objflav);
				} else {
					PyErr_SetString(PyExc_TypeError, "Invalid type given for flavor: expected string or integer");
					return NULL;
				}

				LinkQueryResultPtr res = o->getAllLinks(flavor, src, dst);

				return LinkQueryResultBinder::create(res);
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected three parameters: flavor, src and dst!");
				return NULL;
			}
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		bool LinkServiceBinder::getFlavor(PyObject *src, LinkServicePtr& obj, int& flavor) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			if (PyString_Check(src)) {
				char* str = PyString_AsString(src);
				flavor = obj->nameToFlavor(str);
				return true;
			} else if (PyInt_Check(src)) {
				flavor = PyInt_AsLong(src);
				return true;
			} else {
				PyErr_SetString(PyExc_TypeError, "Invalid type given for flavor: expected string or integer");
				return false;
			}
			__PYTHON_EXCEPTION_GUARD_END_RVAL(false);
		}
		
		// ------------------------------------------
		PyObject* LinkServiceBinder::getAllInherited(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			LinkServicePtr o;
			
			if (!python_cast<LinkServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;
			
			int flavor = 0, src, dst;
			PyObject* objflav;

			// let the third parameter be either string or integer
			// if it's a string, we first have to parse the string to get flavor id

			if (PyArg_ParseTuple(args, "Oii", &objflav, &src, &dst)) {
				if (!getFlavor(objflav, o, flavor))
					return NULL;

				LinkQueryResultPtr res = o->getAllInherited(flavor, src, dst);

				return LinkQueryResultBinder::create(res);
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected three parameters: flavor, src and dst!");
				return NULL;
			}
			__PYTHON_EXCEPTION_GUARD_END_;
		}


		// ------------------------------------------
		PyObject* LinkServiceBinder::getOneLink(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			// Nearly the same as getAllLinks. Only that it returns PyObject for LinkPtr directly
			LinkServicePtr o;
			
			if (!python_cast<LinkServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			int flavor = 0, src, dst;
			PyObject* objflav;

			if (PyArg_ParseTuple(args, "Oii", &objflav, &src, &dst)) {
				if (!getFlavor(objflav, o, flavor))
					return NULL;

				LinkPtr res = o->getOneLink(flavor, src, dst);
				return LinkBinder::create(res);
			}
			else
			{
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected three integer parameters: flavor, src and dst!");
				return NULL;
			}
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* LinkServiceBinder::getAllLinkNames(PyObject* self, PyObject* args)
		{
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			LinkServicePtr o;
			
			if (!python_cast<LinkServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			// wrap the returned StringIterator into StringIteratorBinder, return
			StringIteratorPtr res = o->getAllLinkNames();

			return StringIteratorBinder::create(res);
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* LinkServiceBinder::getFieldsDesc(PyObject* self, PyObject* args) {
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			LinkServicePtr o;
			
			if (!python_cast<LinkServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			PyObject* objflav;
			int flavor = 0;

			if (PyArg_ParseTuple(args, "O", &objflav)) {
				if (!getFlavor(objflav, o, flavor))
					return NULL;

				// wrap the returned StringIterator into StringIteratorBinder, return
				DataFieldDescIteratorPtr res = o->getFieldDescIterator(flavor);
				return DataFieldDescIteratorBinder::create(res);
			}

			// Invalid parameters
			PyErr_SetString(PyExc_TypeError, "Expected a string or integer argument!");
			return NULL;
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* LinkServiceBinder::getattr(PyObject *self, char *name) {
			return Py_FindMethod(msMethods, self, name);
		}

		// ------------------------------------------
		PyObject* LinkServiceBinder::create() {
			Object* object = construct(&msType);

			if (object != NULL) {
				object->mInstance = static_pointer_cast<LinkService>(ServiceManager::getSingleton().getService(msName));
			}

			return (PyObject *)object;
		}

		// ------------------------------------------
		void LinkServiceBinder::init(PyObject* module) {
			publishType(module, &msType, msName);

			LinkBinder::init(module);
			LinkQueryResultBinder::init(module);
			RelationBinder::init(module);
		}


        // -------------------- Link --------------------
		const char* LinkBinder::msName = "Link";

		// ------------------------------------------
		PyTypeObject LinkBinder::msType = {
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			msName,                   // char *tp_name; */
			sizeof(LinkBinder::Object),      // int tp_basicsize; */
			0,                        // int tp_itemsize;       /* not used much */
			LinkBinder::dealloc,   // destructor tp_dealloc; */
			0,			              // printfunc  tp_print;   */
			LinkBinder::getattr,  // getattrfunc  tp_getattr; /* __getattr__ */
		};


        // ------------------------------------------
		PyObject* LinkBinder::getattr(PyObject *self, char *name) {
			LinkPtr o;
			
			if (!python_cast<LinkPtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			if (o.isNull())
			    // Just return PyNone
			    __PY_NONE_RET;
			

			if (strcmp(name, "id") == 0) {
			    return PyLong_FromLong(o->id());
			} else if (strcmp(name, "src") == 0) {
			    return PyLong_FromLong(o->src());
			} else if (strcmp(name, "dst") == 0) {
			    return PyLong_FromLong(o->dst());
            } else if (strcmp(name, "flavor") == 0) {
			    return PyLong_FromLong(o->flavor());
            } else {
                PyErr_SetString(PyExc_TypeError, "Unknown attribute specified!");
            }

            return NULL;
		}

		// ------------------------------------------
		PyObject* LinkBinder::create(LinkPtr& link) {
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
		void LinkBinder::init(PyObject* module) {
			publishType(module, &msType, msName);
		}
  	} // namespace Python
} // namespace Opde

