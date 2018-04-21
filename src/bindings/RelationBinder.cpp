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

#include "RelationBinder.h"
#include "DataFieldDescIteratorBinder.h"
#include "LinkQueryResultBinder.h"
#include "LinkServiceBinder.h"
#include "bindings.h"

namespace Opde {

namespace Python {

// -------------------- Relation --------------------
const char *RelationBinder::msName = "Relation";

// ------------------------------------------
PyTypeObject RelationBinder::msType = {
    PyVarObject_HEAD_INIT(&PyType_Type,
                          0) "opde.services.Relation", /* char *tp_name; */
    sizeof(RelationBinder::Object),                    /* int tp_basicsize; */
    0,                       // int tp_itemsize;       /* not used much */
    RelationBinder::dealloc, /* destructor tp_dealloc; */
    0,                       /* printfunc  tp_print;   */
    0,                       // getattrfunc  tp_getattr; /* __getattr__ */
    0,                       // setattrfunc  tp_setattr;  /* __setattr__ */
    0,                       // cmpfunc  tp_compare;  /* __cmp__ */
    repr,                    // reprfunc  tp_repr;    /* __repr__ */
    0,                       /* PyNumberMethods *tp_as_number; */
    0,                       /* PySequenceMethods *tp_as_sequence; */
    0,                       /* PyMappingMethods *tp_as_mapping; */
    0,                       // hashfunc tp_hash;     /* __hash__ */
    0,                       // ternaryfunc tp_call;  /* __call__ */
    0,                       // reprfunc tp_str;      /* __str__ */
    PyObject_GenericGetAttr, // getattrofunc tp_getattro; */
    0,                       /* setattrofunc tp_setattro; */
    0,                       /* PyBufferProcs *tp_as_buffer; */
    0,                       /* long tp_flags; */
    0,                       /* char *tp_doc;  */
    0,                       /* traverseproc tp_traverse; */
    0,                       /* inquiry tp_clear; */
    0,                       /* richcmpfunc tp_richcompare; */
    0,                       /* long tp_weaklistoffset; */
    0,                       /* getiterfunc tp_iter; */
    0,                       /* iternextfunc tp_iternext; */
    msMethods,               /* struct PyMethodDef *tp_methods; */
    0,                       /* struct memberlist *tp_members; */
    0,                       /* struct getsetlist *tp_getset; */
};

// ------------------------------------------
PyMethodDef RelationBinder::msMethods[] = {
    {"getID", getID, METH_NOARGS},
    {"getName", getName, METH_NOARGS},
    {"remove", remove, METH_VARARGS},
    {"create", createLink, METH_VARARGS},
    {"getLinkField", getLinkField, METH_VARARGS},
    {"setLinkField", setLinkField, METH_VARARGS},
    {"getAllLinks", getAllLinks, METH_VARARGS},
    {"getOneLink", getOneLink, METH_VARARGS},
    {"getFieldsDesc", getFieldsDesc, METH_NOARGS},
    {NULL, NULL},
};

// ------------------------------------------
PyObject *RelationBinder::getID(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;

    Relation *o = NULL;

    if (!python_cast<Relation *>(self, &msType, &o))
        __PY_CONVERR_RET;

    // Get the flavor, construct a python string, return.
    return PyLong_FromLong(o->getID());
    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *RelationBinder::getName(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;

    Relation *o = NULL;

    if (!python_cast<Relation *>(self, &msType, &o))
        __PY_CONVERR_RET;

        // Get the name, construct a python string, return.
#ifdef IS_PY3K
    return PyBytes_FromString(o->getName().c_str());
#else
    return PyString_FromString(o->getName().c_str());
#endif
    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *RelationBinder::remove(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;
    Relation *o = NULL;

    if (!python_cast<Relation *>(self, &msType, &o))
        __PY_CONVERR_RET;

    int id;

    if (PyArg_ParseTuple(args, "i", &id)) {
        o->remove(id);

        PyObject *result = Py_None;
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
PyObject *RelationBinder::createLink(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;
    Relation *o = NULL;

    if (!python_cast<Relation *>(self, &msType, &o))
        __PY_CONVERR_RET;

    int from, to;

    if (PyArg_ParseTuple(args, "ii", &from, &to)) {
        link_id_t id;

        id = o->create(from, to);

        return PyLong_FromUnsignedLong(id);
    } else {
        // Invalid parameters
        PyErr_SetString(PyExc_TypeError,
                        "Expected two integer arguments (from, to) and "
                        "optional link data values (DType)!");
        return NULL;
    }
    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *RelationBinder::getLinkField(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;
    PyObject *result = NULL;
    Relation *o = NULL;

    if (!python_cast<Relation *>(self, &msType, &o))
        __PY_CONVERR_RET;

    int id;
    const char *field;

    if (PyArg_ParseTuple(args, "is", &id, &field)) {
        Variant value;
        value = o->getLinkField(id, field);

        result = VariantToPyObject(value);
        return result;
    } else {
        // Invalid parameters
        PyErr_SetString(PyExc_TypeError, "Expected a string argument!");
        return NULL;
    }
    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *RelationBinder::setLinkField(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;
    Relation *o = NULL;

    if (!python_cast<Relation *>(self, &msType, &o))
        __PY_CONVERR_RET;

    int id;
    const char *field;
    PyObject *Object = NULL;

    if (PyArg_ParseTuple(args, "isO", &id, &field, &Object)) {
        Variant value;
        value = PyObjectToVariant(Object);
        o->setLinkField(id, field, value);

        __PY_NONE_RET;
    } else {
        // Invalid parameters
        PyErr_SetString(PyExc_TypeError, "Expected a string and a value!");
        return NULL;
    }
    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *RelationBinder::getAllLinks(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;
    Relation *o = NULL;

    if (!python_cast<Relation *>(self, &msType, &o))
        __PY_CONVERR_RET;

    int src, dst;

    if (PyArg_ParseTuple(args, "ii", &src, &dst)) {
        LinkQueryResultPtr res = o->getAllLinks(src, dst);

        return LinkQueryResultBinder::create(res);
    } else {
        // Invalid parameters
        PyErr_SetString(PyExc_TypeError,
                        "Expected two integer parameters: src and dst!");
        return NULL;
    }
    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *RelationBinder::getOneLink(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;
    // Nearly the same as getAllLinks. Only that it returns PyObject for LinkPtr
    // directly
    Relation *o = NULL;

    if (!python_cast<Relation *>(self, &msType, &o))
        __PY_CONVERR_RET;

    int src, dst;

    if (PyArg_ParseTuple(args, "ii", &src, &dst)) {
        LinkPtr res = o->getOneLink(src, dst);
        return LinkBinder::create(res);
    } else {
        // Invalid parameters
        PyErr_SetString(PyExc_TypeError,
                        "Expected two integer parameters: src and dst!");
        return NULL;
    }
    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *RelationBinder::getFieldsDesc(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;
    Relation *o = NULL;

    if (!python_cast<Relation *>(self, &msType, &o))
        __PY_CONVERR_RET;

    // wrap the returned StringIterator into StringIteratorBinder, return
    const DataFields& res = o->getFieldDesc();
    return DataFieldsBinder::create(res);

    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *RelationBinder::repr(PyObject *self) {
#ifdef IS_PY3K
    return PyBytes_FromFormat("<Relation at %p>", self);
#else
    return PyString_FromFormat("<Relation at %p>", self);
#endif
}

// ------------------------------------------
PyObject *RelationBinder::create(const RelationPtr &relation) {
    Object *object = construct(&msType);

    if (object != NULL) {
        object->mInstance = relation;
    }

    return (PyObject *)object;
}

// ------------------------------------------
void RelationBinder::init(PyObject *module) {
    publishType(module, &msType, msName);
}
} // namespace Python

} // namespace Opde
