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

#include "DrawServiceBinder.h"
#include "DrawSheet.h"
#include "DrawSheetBinder.h"
#include "DrawSourceBinder.h"
#include "FontDrawSourceBinder.h"
#include "RenderedImageBinder.h"
#include "RenderedLabelBinder.h"
#include "TextureAtlasBinder.h"
#include "bindings.h"

using Ogre::ManualFonFileLoader;

namespace Opde {

namespace Python {

// -------------------- Draw Service --------------------
const char *DrawServiceBinder::msName = "DrawService";

// ------------------------------------------
PyTypeObject DrawServiceBinder::msType = {
    PyVarObject_HEAD_INIT(&PyType_Type,
                          0) "opde.services.DrawService", // char *tp_name; */
    sizeof(DrawServiceBinder::Object), // int tp_basicsize; */
    0,                          // int tp_itemsize;       /* not used much */
    DrawServiceBinder::dealloc, // destructor tp_dealloc; */
    0,                          // printfunc  tp_print;   */
    0,                          // getattrfunc  tp_getattr; /* __getattr__ */
    0,                          // setattrfunc  tp_setattr;  /* __setattr__ */
    0,                          // cmpfunc  tp_compare;  /* __cmp__ */
    repr,                       // reprfunc  tp_repr;    /* __repr__ */
    0,                          // PyNumberMethods *tp_as_number; */
    0,                          // PySequenceMethods *tp_as_sequence; */
    0,                          // PyMappingMethods *tp_as_mapping; */
    0,                          // hashfunc tp_hash;     /* __hash__ */
    0,                          // ternaryfunc tp_call;  /* __call__ */
    0,                          // reprfunc tp_str;      /* __str__ */
    PyObject_GenericGetAttr,    // getattrofunc tp_getattro; */
    0,                          // setattrofunc tp_setattro; */
    0,                          // PyBufferProcs *tp_as_buffer; */
    0,                          // long tp_flags; */
    0,                          // char *tp_doc;  */
    0,                          // traverseproc tp_traverse; */
    0,                          // inquiry tp_clear; */
    0,                          // richcmpfunc tp_richcompare; */
    0,                          // long tp_weaklistoffset; */
    0,                          // getiterfunc tp_iter; */
    0,                          // iternextfunc tp_iternext; */
    msMethods,                  // struct PyMethodDef *tp_methods; */
    0,                          // struct memberlist *tp_members; */
    0,                          // struct getsetlist *tp_getset; */
};

// ------------------------------------------
PyMethodDef DrawServiceBinder::msMethods[] = {
    {"createSheet", createSheet, METH_VARARGS},
    {"destroySheet", destroySheet, METH_VARARGS},
    {"getSheet", getSheet, METH_VARARGS},
    {"setActiveSheet", setActiveSheet, METH_VARARGS},
    {"createDrawSource", createDrawSource, METH_VARARGS},
    {"createRenderedImage", createRenderedImage, METH_VARARGS},
    {"createRenderedLabel", createRenderedLabel, METH_VARARGS},
    {"destroyDrawOperation", destroyDrawOperation, METH_VARARGS},
    {"createAtlas", createAtlas, METH_VARARGS},
    {"destroyAtlas", destroyAtlas, METH_VARARGS},
    {"loadFont", loadFont, METH_VARARGS},
    {"setFontPalette", setFontPalette, METH_VARARGS},
    {NULL, NULL},
};

// ------------------------------------------
PyObject *DrawServiceBinder::createSheet(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;

    DrawServicePtr o;

    if (!python_cast<DrawServicePtr>(self, &msType, &o))
        __PY_CONVERR_RET;

    // argument - the name of the sheet to create, string
    const char *sname;

    if (PyArg_ParseTuple(args, "s", &sname)) {
        const DrawSheetPtr &i = o->createSheet(sname);

        PyObject *o = DrawSheetBinder::create(i);

        return o;
    } else {
        // Invalid parameters
        PyErr_SetString(PyExc_TypeError, "Expected a string argument!");
        return NULL;
    }

    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *DrawServiceBinder::destroySheet(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;

    PyObject *result = NULL;
    DrawServicePtr o;

    if (!python_cast<DrawServicePtr>(self, &msType, &o))
        __PY_CONVERR_RET;

    PyObject *sheet;
    if (PyArg_ParseTuple(args, "O", &sheet)) {
        // cast to drawsheet and destroy
        DrawSheetPtr ds;

        if (!DrawSheetBinder::extract(sheet, ds))
            __PY_CONVERR_RET;

        o->destroySheet(ds);
    }

    result = Py_None;
    Py_INCREF(result);
    return result;

    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *DrawServiceBinder::getSheet(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;

    PyObject *result = NULL;
    DrawServicePtr o;

    if (!python_cast<DrawServicePtr>(self, &msType, &o))
        __PY_CONVERR_RET;

    // argument - the name of the sheet to create, string
    const char *sname;

    if (PyArg_ParseTuple(args, "s", &sname)) {
        const DrawSheetPtr &i = o->getSheet(sname);

        PyObject *o = DrawSheetBinder::create(i);

        result = o;
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
PyObject *DrawServiceBinder::setActiveSheet(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;

    DrawServicePtr o;

    if (!python_cast<DrawServicePtr>(self, &msType, &o))
        __PY_CONVERR_RET;

    PyObject *sheet;
    if (PyArg_ParseTuple(args, "O", &sheet)) {
        DrawSheetPtr ds;

        if (!DrawSheetBinder::extract(sheet, ds))
            __PY_CONVERR_RET;

        o->setActiveSheet(ds);

        __PY_NONE_RET;
    } else {
        // Invalid parameters
        PyErr_SetString(PyExc_TypeError,
                        "Expected a DrawSheet as an argument!");
        return NULL;
    }

    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *DrawServiceBinder::createDrawSource(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;

    DrawServicePtr o;

    if (!python_cast<DrawServicePtr>(self, &msType, &o))
        __PY_CONVERR_RET;

    // argument - the name of the sheet to create, string
    const char *name, *group;

    if (PyArg_ParseTuple(args, "ss", &name, &group)) {
        DrawSourcePtr i = o->createDrawSource(name, group);

        PyObject *o = DrawSourceBinder::create(i);

        return o;
    } else {
        // Invalid parameters
        PyErr_SetString(PyExc_TypeError, "Expected two string arguments!");
        return NULL;
    }

    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *DrawServiceBinder::createRenderedImage(PyObject *self,
                                                 PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;

    PyObject *result = NULL;
    DrawServicePtr o;

    if (!python_cast<DrawServicePtr>(self, &msType, &o))
        __PY_CONVERR_RET;

    PyObject *ds;
    if (PyArg_ParseTuple(args, "O", &ds)) {
        DrawSourcePtr cds;

        if (!DrawSourceBinder::extract(ds, cds))
            __PY_CONVERR_RET;

        RenderedImage *ri = o->createRenderedImage(cds);

        return RenderedImageBinder::create(ri);
    }

    return result;

    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *DrawServiceBinder::createRenderedLabel(PyObject *self,
                                                 PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;

    PyObject *result = NULL;
    DrawServicePtr o;

    if (!python_cast<DrawServicePtr>(self, &msType, &o))
        __PY_CONVERR_RET;

    PyObject *ds;
    if (PyArg_ParseTuple(args, "O", &ds)) {
        FontDrawSourcePtr cds;

        if (!FontDrawSourceBinder::extract(ds, cds))
            __PY_CONVERR_RET;

        RenderedLabel *ri = o->createRenderedLabel(cds);

        return RenderedLabelBinder::create(ri);
    }

    return result;
    __PYTHON_EXCEPTION_GUARD_END_;
}
// ------------------------------------------
PyObject *DrawServiceBinder::destroyDrawOperation(PyObject *self,
                                                  PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;
    DrawServicePtr o;

    if (!python_cast<DrawServicePtr>(self, &msType, &o))
        __PY_CONVERR_RET;

    PyObject *dop;

    if (!PyArg_ParseTuple(args, "O", &dop))
        __PY_BADPARM_RET(1);

    DrawOperation *dopc;

    if (!DrawOperationBinder::extract(dop, dopc))
        __PY_CONVERR_RET;

    o->destroyDrawOperation(dopc);

    __PY_NONE_RET;
    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *DrawServiceBinder::createAtlas(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;
    DrawServicePtr o;

    if (!python_cast<DrawServicePtr>(self, &msType, &o))
        __PY_CONVERR_RET;

    TextureAtlasPtr a = o->createAtlas();

    return TextureAtlasBinder::create(a);

    __PYTHON_EXCEPTION_GUARD_END_;
}
// ------------------------------------------
PyObject *DrawServiceBinder::destroyAtlas(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;
    DrawServicePtr o;

    if (!python_cast<DrawServicePtr>(self, &msType, &o))
        __PY_CONVERR_RET;

    PyObject *atp;

    if (!PyArg_ParseTuple(args, "O", &atp))
        __PY_BADPARM_RET(1);

    TextureAtlasPtr atl;

    if (!TextureAtlasBinder::extract(atp, atl))
        __PY_CONVERR_RET;

    o->destroyAtlas(atl);

    __PY_NONE_RET;
    __PYTHON_EXCEPTION_GUARD_END_;
}
// ------------------------------------------
PyObject *DrawServiceBinder::loadFont(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;
    DrawServicePtr o;

    if (!python_cast<DrawServicePtr>(self, &msType, &o))
        __PY_CONVERR_RET;

    // name, group
    PyObject *patlas = NULL;
    char *name, *group;
    if (!PyArg_ParseTuple(args, "Oss", &patlas, &name, &group))
        __PY_BADPARMS_RET;

    TextureAtlasPtr atlas;

    if (!TextureAtlasBinder::extract(patlas, atlas))
        __PY_BADPARM_RET("atlas");

    FontDrawSourcePtr fds = o->loadFont(atlas, name, group);

    return FontDrawSourceBinder::create(fds);

    __PYTHON_EXCEPTION_GUARD_END_;
}
// ------------------------------------------
PyObject *DrawServiceBinder::setFontPalette(PyObject *self, PyObject *args) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;
    DrawServicePtr o;

    if (!python_cast<DrawServicePtr>(self, &msType, &o))
        __PY_CONVERR_RET;

    int ptype;
    char *fname;
    char *group;

    if (!PyArg_ParseTuple(args, "iss", &ptype, &fname, &group))
        __PY_BADPARMS_RET;

    Ogre::ManualFonFileLoader::PaletteType pt;
    // validate the ptype
    switch (ptype) {
    case (int)ManualFonFileLoader::ePT_Default:
        pt = ManualFonFileLoader::ePT_Default;
        break;
    case (int)ManualFonFileLoader::ePT_DefaultBook:
        pt = ManualFonFileLoader::ePT_DefaultBook;
        break;
    case (int)ManualFonFileLoader::ePT_PCX:
        pt = ManualFonFileLoader::ePT_PCX;
        break;
    case (int)ManualFonFileLoader::ePT_External:
        pt = ManualFonFileLoader::ePT_External;
        break;
    default:
        __PY_BADPARM_RET("paltype");
    }

    // okay! call
    o->setFontPalette(pt, fname, group);

    __PY_NONE_RET;

    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *DrawServiceBinder::repr(PyObject *self) {
#ifdef IS_PY3K
    return PyBytes_FromFormat("<DrawService at %p>", self);
#else
    return PyString_FromFormat("<DrawService at %p>", self);
#endif
}

// ------------------------------------------
PyObject *DrawServiceBinder::create() {
    Object *object = construct(&msType);

    if (object != NULL) {
        object->mInstance = GET_SERVICE(DrawService);
    }

    return (PyObject *)object;
}

// ------------------------------------------
void DrawServiceBinder::init(PyObject *module) {
    publishType(module, &msType, msName);

    PyModule_AddIntConstant(module, "PT_DEFAULT",
                            ManualFonFileLoader::ePT_Default);
    PyModule_AddIntConstant(module, "PT_DEFAULTBOOK",
                            ManualFonFileLoader::ePT_DefaultBook);
    PyModule_AddIntConstant(module, "PT_PCX", ManualFonFileLoader::ePT_PCX);
    PyModule_AddIntConstant(module, "PT_EXTERNAL",
                            ManualFonFileLoader::ePT_External);
}

// -------------------- Draw Operation --------------------
const char *DrawOperationBinder::msName = "DrawOperation";

// ------------------------------------------
PyTypeObject DrawOperationBinder::msType = {
    PyVarObject_HEAD_INIT(&PyType_Type,
                          0) "opde.services.DrawOperation", // char *tp_name; */
    sizeof(DrawOperationBinder::Object), // int tp_basicsize; */
    0,                            // int tp_itemsize;       /* not used much */
    DrawOperationBinder::dealloc, // destructor tp_dealloc; */
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
#warning Check for correctness
    1, // long tp_flags; */
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
    msGetSet   // struct getsetlist /* *tp_getset; */
};

// ------------------------------------------
PyMethodDef DrawOperationBinder::msMethods[] = {
    {"setPosition", setPosition, METH_VARARGS},
    {"setZOrder", setZOrder, METH_VARARGS},
    {"setClipRect", setClipRect, METH_VARARGS},
    {NULL, NULL}};

const char *position = "position";
const char *zOrder = "zorder";
const char *clippingRect = "cliprect";

const char *positionDoc = "On-Screen Position";
const char *zOrderDoc = "Z Order";
const char *clippingRectDoc = "Clipping rectangle";

// ------------------------------------------
PyGetSetDef DrawOperationBinder::msGetSet[] = {
    {const_cast<char *>(position), getPPosition, setPPosition,
     const_cast<char *>(positionDoc), NULL},
    {const_cast<char *>(zOrder), getPZOrder, setPZOrder,
     const_cast<char *>(zOrderDoc), NULL},
    {const_cast<char *>(clippingRect), getPClipRect, setPClipRect,
     const_cast<char *>(clippingRectDoc), NULL},
    {NULL}};

// ------------------------------------------
CastInfo<DrawOperation *> DrawOperationBinder::msCastInfo[] = {
    {&RenderedImageBinder::msType,
     &defaultPythonUpcaster<DrawOperation *, RenderedImage *>},
    {&RenderedLabelBinder::msType,
     &defaultPythonUpcaster<DrawOperation *, RenderedLabel *>},
    {NULL}};

// ------------------------------------------
PyObject *DrawOperationBinder::setPosition(PyObject *self, PyObject *args) {
    // args: either (x,y) or ((x,y)). The setPPosition expects (x,y) so we have
    // to unwrap the tuple if encountered
    PyObject *tup;

    // We can't use PyArg_ParseTuple as it terminates on parameter
    // inconsistencies. instead, we look at the tuple in more detail and decide
    if (!PyTuple_Check(args))
        __PY_BADPARMS_RET;

    int len = PyTuple_Size(args);

    if (len == 2) {
        // use the args direcly, the called func. checks
        if (setPPosition(self, args, NULL) == 0) {
            __PY_NONE_RET;
        } else { // otherwise return null to indicate a problem
            return NULL;
        }
    } else if (len == 1) {
        // extract the tuple
        PyObject *tup = PyTuple_GetItem(args, 0);

        if (!PyTuple_Check(tup))
            __PY_BADPARMS_RET;

        if (setPPosition(self, tup, NULL) == 0) {
            __PY_NONE_RET;
        } else { // otherwise return null to indicate a problem
            return NULL;
        }
    } else {
        // Not the right type, no sir!
        __PY_BADPARMS_RET;
    }

    if (PyArg_ParseTuple(args, "O", &tup)) { // tuple in tuple

    } else {
        if (setPPosition(self, args, NULL) == 0) {
            __PY_NONE_RET;
        } else { // otherwise return null to indicate a problem
            return NULL;
        }
    }
}

// ------------------------------------------
PyObject *DrawOperationBinder::setZOrder(PyObject *self, PyObject *args) {
    if (setPZOrder(self, args, NULL) == 0) { // if succeeded
        __PY_NONE_RET;
    } else { // otherwise return null to indicate a problem
        return NULL;
    }
}

// ------------------------------------------
PyObject *DrawOperationBinder::setClipRect(PyObject *self, PyObject *args) {
    int len = PyTuple_Size(args);

    if (len != 1)
        __PY_BADPARMS_RET;

    PyObject *tup = PyTuple_GetItem(args, 0);

    if (!PyTuple_Check(tup))
        __PY_BADPARMS_RET;

    if (setPClipRect(self, tup, NULL) == 0) { // if succeeded
        __PY_NONE_RET;
    } else { // otherwise return null to indicate a problem
        return NULL;
    }
}

// ------------------------------------------
int DrawOperationBinder::setPPosition(PyObject *self, PyObject *value,
                                      void *closure) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;

    DrawOperation *o = NULL;

    assert(self != NULL);

    if (!python_cast<DrawOperation *>(self, &msType, &o, msCastInfo))
        __PY_CONVERR_RET_VAL(-1);

    assert(o != NULL);

    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "Cannot delete the attribute");
        return -1;
    }

    // okay, we can proceed with the setting now

    PixelCoord pc;

    if (TypeInfo<PixelCoord>::fromPyObject(value, pc)) {
        o->setPosition(pc);

        return 0;
    } else {
        // Invalid parameters
        PyErr_SetString(PyExc_TypeError, "Expected a two integer tuple!");
        return -1;
    }

    __PYTHON_EXCEPTION_GUARD_END_RVAL(-1);
}
// ------------------------------------------
int DrawOperationBinder::setPZOrder(PyObject *self, PyObject *value,
                                    void *closure) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;

    DrawOperation *o = NULL;

    if (!python_cast<DrawOperation *>(self, &msType, &o, msCastInfo))
        __PY_CONVERR_RET_VAL(-1);

    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "Cannot delete the attribute");
        return -1;
    }

    int z;

    if (PyArg_ParseTuple(value, "i", &z)) {
        o->setZOrder(z);

        return 0;
    } else {
        // Invalid parameters
        PyErr_SetString(PyExc_TypeError, "Expected an integer argument!");
        return -1;
    }

    __PYTHON_EXCEPTION_GUARD_END_RVAL(-1);
}

// ------------------------------------------
int DrawOperationBinder::setPClipRect(PyObject *self, PyObject *value,
                                      void *closure) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;

    DrawOperation *o = NULL;

    if (!python_cast<DrawOperation *>(self, &msType, &o, msCastInfo))
        __PY_CONVERR_RET_VAL(-1);

    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "Cannot delete the attribute");
        return -1;
    }

    ClipRect rect;
    if (!TypeInfo<ClipRect>::fromPyObject(value, rect)) {
        PyErr_SetString(PyExc_TypeError,
                        "Expected a 4 integer tuple(left,right,top,bottom)!");
        return -1;
    };

    o->setClipRect(rect);

    return 0;

    __PYTHON_EXCEPTION_GUARD_END_RVAL(-1);
}

// ------------------------------------------
PyObject *DrawOperationBinder::getPPosition(PyObject *self, void *closure) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;
    DrawOperation *o = NULL;

    if (!python_cast<DrawOperation *>(self, &msType, &o, msCastInfo))
        __PY_CONVERR_RET;

    return TypeInfo<PixelCoord>::toPyObject(o->getPosition());

    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *DrawOperationBinder::getPZOrder(PyObject *self, void *closure) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;
    DrawOperation *o = NULL;

    if (!python_cast<DrawOperation *>(self, &msType, &o, msCastInfo))
        __PY_CONVERR_RET;

    return TypeInfo<int>::toPyObject(o->getZOrder());

    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *DrawOperationBinder::getPClipRect(PyObject *self, void *closure) {
    __PYTHON_EXCEPTION_GUARD_BEGIN_;
    DrawOperation *o = NULL;

    if (!python_cast<DrawOperation *>(self, &msType, &o, msCastInfo))
        __PY_CONVERR_RET;

    return TypeInfo<ClipRect>::toPyObject(o->getClipRect());

    __PYTHON_EXCEPTION_GUARD_END_;
}

// ------------------------------------------
PyObject *DrawOperationBinder::repr(PyObject *self) {
#ifdef IS_PY3K
    return PyBytes_FromFormat("<DrawOperation at %p>", self);
#else
    return PyString_FromFormat("<DrawOperation at %p>", self);
#endif
}

// ------------------------------------------
bool DrawOperationBinder::extract(PyObject *object, DrawOperation *&op) {
    return python_cast<DrawOperation *>(object, &msType, &op, msCastInfo);
}

// ------------------------------------------
PyObject *DrawOperationBinder::create(DrawOperation *sh) {
    Object *object = construct(&msType);

    if (object != NULL) {
        object->mInstance = sh;
    }

    return (PyObject *)object;
}

// ------------------------------------------
void DrawOperationBinder::init(PyObject *module) {
    publishType(module, &msType, msName);
}

} // namespace Python
} // namespace Opde
