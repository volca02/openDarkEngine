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

#include "RootBinder.h"
#include "bindings.h"

namespace Opde {

namespace Python {

// -------------------- Link Service --------------------
const char *RootBinder::msName = "Root";

const char *opde_Root__doc__ = "The root object of the openDarkEngine API. "
                               "Exposes some common objects/methods.\n";

// ------------------------------------------
PyTypeObject RootBinder::msType = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0) "opde.Root", /* char *tp_name; */
    sizeof(RootBinder::Base),                         /* int tp_basicsize; */
    0,                       // int tp_itemsize;       /* not used much */
    RootBinder::dealloc,     /* destructor tp_dealloc; */
    0,                       /* printfunc  tp_print;   */
    0,                       // getattrfunc  tp_getattr; /* __getattr__ */
    0,                       // setattrfunc  tp_setattr;  /* __setattr__ */
    0,                       // cmpfunc  tp_compare;  /* __cmp__ */
    repr,                    // reprfunc  tp_repr;    /* __repr__ */
    0,                       // PyNumberMethods *tp_as_number; */
    0,                       // PySequenceMethods *tp_as_sequence; */
    0,                       // PyMappingMethods *tp_as_mapping; */
    0,                       // hashfunc tp_hash;     /* __hash__ */
    0,                       // ternaryfunc tp_call;  /* __call__ */
    0,                       // reprfunc tp_str;      /* __str__ */
    PyObject_GenericGetAttr, // getattrofunc tp_getattro; */
    0,                       // setattrofunc tp_setattro; */
    0,                       // PyBufferProcs *tp_as_buffer; */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // long tp_flags; */
    opde_Root__doc__,                         // char *tp_doc;  */
    0,                                        // traverseproc tp_traverse; */
    0,                                        // inquiry tp_clear; */
    0,                                        // richcmpfunc tp_richcompare; */
    0,                                        // long tp_weaklistoffset; */
    0,                                        // getiterfunc tp_iter; */
    0,                                        // iternextfunc tp_iternext; */
    msMethods, // struct PyMethodDef *tp_methods; */
    0,         // struct memberlist *tp_members; */
    0,         // struct getsetlist *tp_getset; */
    0,         // struct _typeobject *tp_base; */
    0,         // PyObject *tp_dict; */
    0,         // descrgetfunc tp_descr_get; */
    0,         // descrsetfunc tp_descr_set; */
    0,         // long tp_dictoffset; */
    0,         // initproc tp_init; */
    0          // allocfunc tp_alloc; */
};

// ------ Doc Strings -----------------------
const char *opde_Root_loadResourceConfig__doc__ =
    "loadResourceConfig(path)\n"
    "Loads a Ogre's resource config file (resources.cfg type) from the "
    "specified path\n"
    "@type path: string\n"
    "@param path: Path to the resource config file to be loaded\n";

const char *opde_Root_loadConfigFile__doc__ =
    "loadConfigFile(path)\n"
    "Loads a Opde's config file (opde.cfg type) from the specified path\n"
    "@type path: string\n"
    "@param path: Path to the Opde's config file to be loaded\n";

const char *opde_Root_addResourceLocation__doc__ =
    "addResourceLocation(name,type,section[,recursive])\n"
    "Adds a resource location to the specified section (group)\n"
    "@type name: string\n"
    "@param name: Resource location name\n"
    "@type type: string\n"
    "@param type: Resource location type (FileSystem, Zip, Crf, Dir,...)\n"
    "@type section: string\n"
    "@param section: The section (resource group) to add to\n"
    "@type recursive: bool\n"
    "@param recursive: Optional parameter specifying if subdirectories should "
    "be searched\n;";

const char *opde_Root_removeResourceLocation__doc__ =
    "removeResourceLocation(name,section)\n"
    "Removes resource location from the specified section (group)\n"
    "@type name: string\n"
    "@param name: Resource location name\n"
    "@type section: string\n"
    "@param section: The section (resource group) to remove from\n";

const char *opde_Root_bootstrapFinished__doc__ =
    "bootstrapFinished()\n"
    "Informs Opde that the resource setup has been done and opde can start.";

const char *opde_Root_logToFile__doc__ =
    "logToFile(filename)\n"
    "Removes resource location from the specified section (group)\n"
    "@type filename: string\n"
    "@param filename: Resource location name\n";

const char *opde_Root_setLogLevel__doc__ =
    "setLogLevel(level)\n"
    "Removes resource location from the specified section (group)\n"
    "@type level: integer\n"
    "@param level: The desired log level\n";

const char *opde_Root_registerCustomScriptLoaders__doc__ =
    "registerCustomScriptLoaders()\n"
    "Registers .dtype and .pldef script loaders to ogre for automatic loading "
    "(this means from this point in time further all encountered scripts of "
    "those types will be automatically parsed)\n";

// ------------------------------------------
PyMethodDef RootBinder::msMethods[] = {
    {"loadResourceConfig", RootBinder::loadResourceConfig, METH_VARARGS,
     opde_Root_loadResourceConfig__doc__},
    {"loadConfigFile", RootBinder::loadConfigFile, METH_VARARGS,
     opde_Root_loadConfigFile__doc__},
    {"addResourceLocation", RootBinder::addResourceLocation, METH_VARARGS,
     opde_Root_addResourceLocation__doc__},
    {"removeResourceLocation", RootBinder::removeResourceLocation, METH_VARARGS,
     opde_Root_removeResourceLocation__doc__},
    {"bootstrapFinished", RootBinder::bootstrapFinished, METH_NOARGS,
     opde_Root_bootstrapFinished__doc__},
    {"logToFile", RootBinder::logToFile, METH_VARARGS,
     opde_Root_logToFile__doc__},
    {"setLogLevel", RootBinder::setLogLevel, METH_VARARGS,
     opde_Root_setLogLevel__doc__},
    {NULL, NULL},
};

// ------------------------------------------
PyObject *RootBinder::loadResourceConfig(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;
    // ARGS: string
    Root *o = NULL;

    if (!python_cast<Root *>(self, &msType, &o))
        __PY_CONVERR_RET;

    const char *fname;

    if (PyArg_ParseTuple(args, "s", &fname)) {
        o->loadResourceConfig(fname);

        __PY_NONE_RET;
    } else {
        // Invalid parameters
        PyErr_SetString(PyExc_TypeError, "Expected a string argument!");
        return NULL;
    }
    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *RootBinder::loadConfigFile(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;
    // ARGS: string
    Root *o = NULL;

    if (!python_cast<Root *>(self, &msType, &o))
        __PY_CONVERR_RET;

    const char *fname;

    if (PyArg_ParseTuple(args, "s", &fname)) {
        o->loadConfigFile(fname);

        __PY_NONE_RET;
    } else {
        // Invalid parameters
        PyErr_SetString(PyExc_TypeError, "Expected a string argument!");
        return NULL;
    }
    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *RootBinder::addResourceLocation(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;
    // ARGS: string, string, string, bool
    Root *o = NULL;

    if (!python_cast<Root *>(self, &msType, &o))
        __PY_CONVERR_RET;

    const char *name, *type, *section;
    PyObject *recursive = Py_False;

    if (PyArg_ParseTuple(args, "sss|O", &name, &type, &section, &recursive)) {
        bool recb = false;

        if (!TypeInfo<bool>::fromPyObject(recursive, recb))
            __PY_BADPARM_RET("recursive");

        o->addResourceLocation(name, type, section, recb);

        __PY_NONE_RET;
    } else {
        // Invalid parameters
        PyErr_SetString(
            PyExc_TypeError,
            "Expected three string arguments and one optional bool!");
        return NULL;
    }
    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *RootBinder::removeResourceLocation(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;
    // ARGS: string, string
    Root *o = NULL;

    if (!python_cast<Root *>(self, &msType, &o))
        __PY_CONVERR_RET;

    const char *name, *section;

    if (PyArg_ParseTuple(args, "ss", &name, &section)) {
        o->removeResourceLocation(name, section);

        __PY_NONE_RET;
    } else {
        // Invalid parameters
        PyErr_SetString(PyExc_TypeError, "Expected two string arguments!");
        return NULL;
    }
    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *RootBinder::bootstrapFinished(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;
    // ARGS: No arguments!
    Root *o = NULL;

    if (!python_cast<Root *>(self, &msType, &o))
        __PY_CONVERR_RET;

    o->bootstrapFinished();

    __PY_NONE_RET;

    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *RootBinder::logToFile(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;
    // ARGS: string
    Root *o = NULL;

    if (!python_cast<Root *>(self, &msType, &o))
        __PY_CONVERR_RET;

    const char *fname;

    if (PyArg_ParseTuple(args, "s", &fname)) {
        o->logToFile(fname);

        __PY_NONE_RET;
    } else {
        // Invalid parameters
        PyErr_SetString(PyExc_TypeError, "Expected a string argument!");
        return NULL;
    }

    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *RootBinder::setLogLevel(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;
    // ARGS: string
    Root *o = NULL;

    if (!python_cast<Root *>(self, &msType, &o))
        __PY_CONVERR_RET;

    int level;

    if (PyArg_ParseTuple(args, "i", &level)) {
        o->setLogLevel(level); // does not throw

        __PY_NONE_RET;
    } else {
        // Invalid parameters
        PyErr_SetString(PyExc_TypeError, "Expected an integer argument!");
        return NULL;
    }

    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *RootBinder::repr(PyObject *self) {
#ifdef IS_PY3K
    return PyBytes_FromFormat("<Opde::Root at %p>", self);
#else
    return PyString_FromFormat("<Opde::Root at %p>", self);
#endif
}

// ------------------------------------------
PyObject *RootBinder::create(Root *root) {
    Base *object = construct(&msType);

    if (object != NULL) {
        object->mInstance = root;
    }

    return (PyObject *)object;
}

// ------------------------------------------
void RootBinder::init(PyObject *module) {
    // Register root in the Opde module
    publishType(module, &msType, msName);
}
} // namespace Python

} // namespace Opde
