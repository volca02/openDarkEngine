/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2009 openDarkEngine team
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

#include "RenderedImageBinder.h"
#include "DrawServiceBinder.h"
#include "DrawSourceBinder.h"
#include "bindings.h"
#include "draw/RenderedImage.h"

namespace Opde {

namespace Python {

// -------------------- Draw Source --------------------
const char *RenderedImageBinder::msName = "RenderedImage";

// ------------------------------------------
PyTypeObject RenderedImageBinder::msType = {
    PyVarObject_HEAD_INIT(&PyType_Type,
                          0) "opde.services.RenderedImage", // char *tp_name; */
    sizeof(RenderedImageBinder::Object), // int tp_basicsize; */
    0,                            // int tp_itemsize;       /* not used much */
    RenderedImageBinder::dealloc, // destructor tp_dealloc; */
    0,                            // printfunc  tp_print;   */
    0,                            // getattrfunc  tp_getattr; /* __getattr__ */
    0,                            // setattrfunc  tp_setattr;  /* __setattr__ */
    0,                            // cmpfunc  tp_compare;  /* __cmp__ */
    repr,                         // reprfunc  tp_repr;    /* __repr__ */
    0,                            // PyNumberMethods *tp_as_number; */
    0,                            // PySequenceMethods *tp_as_sequence; */
    0,                            // PyMappingMethods *tp_as_mapping; */
    0,                            // hashfunc tp_hash;     /* __hash__ */
    0,                            // ternaryfunc tp_call;  /* __call__ */
    0,                            // reprfunc tp_str;      /* __str__ */
    PyObject_GenericGetAttr,      // getattrofunc tp_getattro; */
    PyObject_GenericSetAttr,      // setattrofunc tp_setattro; */
    0,                            // PyBufferProcs *tp_as_buffer; */
// for inheritance searches to work we need this
#ifdef IS_PY3K
#warning Check this for correctness
    1,
#else
    Py_TPFLAGS_HAVE_CLASS, // long tp_flags; */
#endif
    0,         // char *tp_doc;  */
    0,         // traverseproc tp_traverse; */
    0,         // inquiry tp_clear; */
    0,         // richcmpfunc tp_richcompare; */
    0,         // long tp_weaklistoffset; */
    0,         // getiterfunc tp_iter; */
    0,         // iternextfunc tp_iternext; */
    msMethods, // struct PyMethodDef *tp_methods; */
    0,         // struct memberlist /*  *tp_members; */
    0,         // struct getsetlist /* *tp_getset; */
    // Base object type - needed for inheritance checks. Here, it is the
    // DrawOperationBinder stub.
    &DrawOperationBinder::msType // struct _typeobject *tp_base;
};

// ------------------------------------------
PyMethodDef RenderedImageBinder::msMethods[] = {
    {"setDrawSource", setDrawSource, METH_VARARGS}, {NULL, NULL}};

// ------------------------------------------
PyObject *RenderedImageBinder::setDrawSource(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;

    RenderedImage *o = NULL;

    if (!python_cast<RenderedImage *>(self, &msType, &o))
        __PY_CONVERR_RET;

    PyObject *ds;

    if (PyArg_ParseTuple(args, "O", &ds)) {
        // if it's a tuple, it should contain four floats
        DrawSourcePtr dsp;

        if (!DrawSourceBinder::extract(ds, dsp)) {
            PyErr_SetString(PyExc_TypeError,
                            "Expected a DrawSource as an argument!");
            return NULL;
        }

        o->setDrawSource(dsp);

        __PY_NONE_RET;
    } else {
        // Invalid parameters
        PyErr_SetString(PyExc_TypeError,
                        "Expected a DrawSource as an argument!");
        return NULL;
    }

    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *RenderedImageBinder::repr(PyObject *self) {
#ifdef IS_PY3K
    return PyBytes_FromFormat("<RenderedImage at %p>", self);
#else
    return PyString_FromFormat("<RenderedImage at %p>", self);
#endif
}

// ------------------------------------------
bool RenderedImageBinder::extract(PyObject *obj, RenderedImage *&tgt) {
    return python_cast<RenderedImage *>(obj, &msType, &tgt);
}

// ------------------------------------------
PyObject *RenderedImageBinder::create(RenderedImage *sh) {
    Object *object = construct(&msType);

    if (object != NULL) {
        object->mInstance = sh;
    }

    return (PyObject *)object;
}

// ------------------------------------------
void RenderedImageBinder::init(PyObject *module) {
    publishType(module, &msType, msName);
}

} // namespace Python
} // namespace Opde
