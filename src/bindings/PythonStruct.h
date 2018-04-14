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

#ifndef __PYTHONSTRUCT_H
#define __PYTHONSTRUCT_H

#include "Callback.h"
#include "Python.h"
#include "SharedPtr.h"
#include "bindings.h"
#include "compat.h"
#include "logger.h"

namespace Opde {
namespace Python {

/** Templatized struct binder (no field writing supported). Exposes Type T as a
 * python object with attributes.
 * @note Every descendant needs to do at least three things: Initialize the
 * msName and fill the msNameToAttr using method field, and call the static init
 * method created for the filling operation from somewhere.
 */
template <typename T> class PythonStruct : PythonPublishedType {
public:
    static PyObject *create(const T &t) {
        Object *object;
        object = PyObject_New(Object, &msType);

        if (object != NULL) {
            object->mInstance = new T(t);
        }

        return (PyObject *)object;
    };

protected:
    static void dealloc(PyObject *self) {
        Object *o = reinterpret_cast<Object *>(self);

        delete o->mInstance;

        PyObject_Del(self);
    }

    /** Has to be called after field defs from init function. Completes the type
     * to be usable, and publishes it
     * @param tpDoc the doc string for the published type
     * @param tpName the name of the type (incl. the module - e.g.
     * "Opde.SampleStruct")
     * @param module the container to put the type into (has to agree with the
     * tpName's path)
     */
    static void publish(const char *tpDoc, const char *tpName,
                        PyObject *module) {
        msType.tp_doc = tpDoc;
        msType.tp_name = const_cast<char *>(tpName);

        // TODO: A way to expose the fields?
        // and publish
        publishType(module, &msType, tpName);
    };

    static PyObject *getattr(PyObject *self, char *attrname) {
        __PYTHON_EXCEPTION_GUARD_BEGIN_;

        T *object;

        if (!python_cast<T *>(self, &msType, &object))
            __PY_CONVERR_RET;

        typename NameToAttr::iterator it = msNameToAttr.find(attrname);

        if (it != msNameToAttr.end()) {
            return (*it->second)(object);
        } else {
            PyErr_Format(PyExc_AttributeError,
                         "Invalid attribute name encountered: %s", attrname);
            return NULL;
        }

        __PYTHON_EXCEPTION_GUARD_END_;
    }

    static PyObject *repr(PyObject *self) {
#ifdef IS_PY3K
        return PyBytes_FromFormat("<Struct:%s at %p>", msName, self);
#else
        return PyString_FromFormat("<Struct:%s at %p>", msName, self);
#endif
    }

    struct FieldDefBase {
        virtual PyObject *operator()(const T *var) const = 0;

        virtual const char *getType() const = 0;

        virtual ~FieldDefBase(){};
    };

    template <typename F> struct FieldDef : public FieldDefBase {
        typedef F T::*MemberPointer;
        typedef TypeInfo<F> MemberType;

        MemberPointer field;
        MemberType type;

        FieldDef(MemberPointer f) : field(f), type(){};

        virtual PyObject *operator()(const T *var) const {
            return type.toPyObject(var->*field);
        }

        virtual const char *getType() const { return type.typeName; }
    };

    typedef shared_ptr<FieldDefBase> FieldDefBasePtr;

    typedef std::map<std::string, FieldDefBasePtr> NameToAttr;

    static NameToAttr msNameToAttr;

    // Field definition statics:
    template <typename FT>
    static void field(const char *name, FT T::*fieldPtr) {
        FieldDefBasePtr fd(new FieldDef<FT>(fieldPtr));

        msNameToAttr.insert(std::make_pair(name, fd));
    }

    // Needs to be filled prior to usage
    static const char *msName;

    typedef ObjectBase<T *> Object;

    static PyTypeObject msType;
};

template <typename T>
const char *PythonStruct<T>::msName = "PythonStruct::InvalidType!";

// Static member initialization
template <typename T>
typename PythonStruct<T>::NameToAttr PythonStruct<T>::msNameToAttr;

template <typename T>
PyTypeObject PythonStruct<T>::msType = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0) msName, /* char *tp_name; */
    sizeof(Object),                                /* int tp_basicsize; */
    0,       // int tp_itemsize;       /* not used much */
    dealloc, /* destructor tp_dealloc; */
    0,       /* printfunc  tp_print;   */
    getattr, // getattrfunc  tp_getattr; /* __getattr__ */
    0,       // setattrfunc  tp_setattr;  /* __setattr__ */
    0,       // cmpfunc  tp_compare;  /* __cmp__ */
    repr,    // reprfunc  tp_repr;    /* __repr__ */
};
} // namespace Python
} // namespace Opde

#endif
