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

#ifndef __BINDINGS_H
#define __BINDINGS_H

#include <utility>

// this has to be the first one - _POSIX_C_SOURCE crazyness...
#include <Python.h>

#include "DVariant.h"
#include "OpdeException.h"

#include <OgreColourValue.h>
#include <OgreException.h>
#include <OgreVector3.h>
#include <frameobject.h>
#include <traceback.h>

namespace Opde {
class Root;
} // namespace Opde

// This implements error handling independently on python2/3
struct module_state {
    PyObject *error;
};

#if PY_MAJOR_VERSION >= 3
// This will guide our bindings code through the python2/python3 differences.
#define IS_PY3K
#define GETSTATE(m) ((struct module_state *)PyModule_GetState(m))

#else // PY_MAJOR_VERSION >= 3

#define GETSTATE(m) (&_state)

#endif // PY_MAJOR_VERSION >= 3

// TODO: We should create our own exception type to help python side coding
// Exception guard for the python code. use this as first/last lines of the
// binding function to have it prepared for exceptions
#define __PYTHON_EXCEPTION_GUARD_BEGIN_ try {

#define __PYTHON_EXCEPTION_GUARD_END_RVAL(rval)                                \
    }                                                                          \
    catch (BasicException & e) {                                               \
        PyErr_Format(PyExc_RuntimeError, "C++ side exception (%s:%u) : %s",    \
                     __FILE__, __LINE__, e.getDetails().c_str());              \
        return rval;                                                           \
    }                                                                          \
    catch (Ogre::Exception & e) {                                              \
        PyErr_Format(PyExc_RuntimeError, "Ogre exception (%s:%u) : %s",        \
                     __FILE__, __LINE__, e.getFullDescription().c_str());      \
        return rval;                                                           \
    }

#define __PYTHON_EXCEPTION_GUARD_END_ __PYTHON_EXCEPTION_GUARD_END_RVAL(NULL)

/// Checks if the specified PyObject is int or long
#define __PYINTLONG_CHECK(a) (PyLong_Check(a) || PyLong_Check(a))

namespace Opde {
namespace Python {

class OPDELIB_EXPORT PythonException : public Opde::BasicException {
protected:
    std::string mType;

public:
    PythonException(const std::string &type, const std::string &desc,
                    const std::string &src, const char *file = NULL,
                    long line = -1)
        : BasicException(desc, src, file, line), mType(type) {
        details = std::string("PythonException: " + type + "\n\t" + details);
    }

    ~PythonException() throw(){};

    /// wraps and throws PythonException from the python error detected, if such
    /// happened...
    static void checkAndThrow() {
        if (PyErr_Occurred()) {
            // get the error into a string description (filename, line are
            // needed as well)
            PyObject *errobj, *errdata, *errtraceback, *pystring = NULL;

            /* get latest python exception info */
            PyErr_Fetch(&errobj, &errdata, &errtraceback); /* increfs all 3 */

            std::string etype = "<unknown exception type>";
            std::string edata = "<unknown exception data>";

            if (errobj != NULL &&
                (pystring = PyObject_Str(errobj)) != NULL && /*	str(errobj) */
#ifdef IS_PY3K
                (PyBytes_Check(pystring)) /* str():increfs */
            )
                etype = PyBytes_AsString(pystring); /* Py->C */
#else
                (PyString_Check(pystring)) /* str():increfs */
            )
                etype = PyString_AsString(pystring); /* Py->C */
#endif
            // decref again
            Py_XDECREF(pystring);

            pystring = NULL;
            if (errdata != NULL &&
                (pystring = PyObject_Str(errdata)) !=
                    NULL && /* str():increfs */
#ifdef IS_PY3K
                (PyBytes_Check(pystring)) /* str():increfs */
            )
                etype = PyBytes_AsString(pystring); /* Py->C */
#else
                (PyString_Check(pystring))           /* str():increfs */
            )
                etype = PyString_AsString(pystring); /* Py->C */
#endif
            Py_XDECREF(pystring);

            long lineno = -1;
            std::string fname = "<unknown file>";

            if (errtraceback) {
                lineno = ((PyTracebackObject *)errtraceback)->tb_lineno;
                PyObject *pyfname = ((PyTracebackObject *)errtraceback)
                                        ->tb_frame->f_code->co_filename;

#ifdef IS_PY3K
                if (pyfname != NULL && PyBytes_Check(pyfname))
                    fname = PyBytes_AsString(pyfname);
#else
                if (pyfname != NULL && PyString_Check(pyfname))
                    fname = PyString_AsString(pyfname);
#endif
            }

            Py_XDECREF(errobj);
            Py_XDECREF(errdata);      /* caller owns all 3 objects */
            Py_XDECREF(errtraceback); /* already NULL'd out in Python */

            throw(
                PythonException(etype, edata, "Python", fname.c_str(), lineno));
        }
    }
};

#define __PY_HANDLE_PYTHON_ERROR Opde::Python::PythonException::checkAndThrow()

// Type converters
enum VariableType {
    VT_INVALID,
    VT_BOOL,
    VT_INT,
    VT_LONG,
    VT_FLOAT,
    VT_CHARPTR,
    VT_STRING,
    VT_CUSTOM_TYPE
};

template <typename T> struct TypeInfoBase {
    const char *typeName;
    VariableType type;

    TypeInfoBase(const char *tn, VariableType tt) : typeName(tn), type(tt){};
};

template <typename T> struct TypeInfo : public TypeInfoBase<T> {
    TypeInfo() : TypeInfoBase<T>("invalid", VT_INVALID){};

    // TypeInfo(const char* tname, VariableType tt) : TypeInfoBase(tname, tt)
    // {};

    static PyObject *toPyObject(T val) {
        PyErr_SetString(
            PyExc_TypeError,
            "Binding error: Type has no conversion or TypeInfo specified!");
        return NULL;
    }

    static bool fromPyObject(PyObject *src, T &dst) { return false; }
};

template <> struct TypeInfo<bool> : TypeInfoBase<bool> {
    TypeInfo() : TypeInfoBase<bool>("bool", VT_BOOL){};

    static PyObject *toPyObject(bool val) {
        PyObject *res = val ? Py_True : Py_False;

        Py_INCREF(res);

        return res;
    }

    static bool fromPyObject(PyObject *src, bool &dst) {
        if (PyBool_Check(src)) {
            dst = (src == Py_True);
            return true;
        } else {
            return false;
        }
    }
};

template <> struct TypeInfo<int> : TypeInfoBase<int> {
    TypeInfo() : TypeInfoBase<int>("int", VT_INT){};

    static PyObject *toPyObject(int val) { return PyLong_FromLong(val); }

    static bool fromPyObject(PyObject *src, int &dst) {
        if (PyLong_Check(src)) {
            dst = static_cast<int>(PyLong_AsLong(src));
            return true;
        } else {
            return false;
        }
    }
};

template <> struct TypeInfo<long> : TypeInfoBase<long> {
    TypeInfo() : TypeInfoBase<long>("long", VT_LONG){};

    static PyObject *toPyObject(long val) { return PyLong_FromLong(val); }

    static bool fromPyObject(PyObject *src, long &dst) {
        if (PyLong_Check(src)) {
            dst = PyLong_AsLong(src);
            return true;
        } else {
            return false;
        }
    }
};

template <> struct TypeInfo<float> : TypeInfoBase<float> {
    TypeInfo() : TypeInfoBase<float>("float", VT_FLOAT){};

    static PyObject *toPyObject(float val) { return PyFloat_FromDouble(val); }

    static bool fromPyObject(PyObject *src, float &dst) {
        if (PyFloat_Check(src)) {
            dst = PyFloat_AsDouble(src);
            return true;
        } else {
            return false;
        }
    }
};

template <> struct TypeInfo<std::string> : TypeInfoBase<std::string> {
    TypeInfo() : TypeInfoBase<std::string>("std::string", VT_STRING){};

    static PyObject *toPyObject(const std::string &val) {
#ifdef IS_PY3K
        return PyBytes_FromString(val.c_str());
#else
        return PyString_FromString(val.c_str());
#endif
    }

    static bool fromPyObject(PyObject *src, std::string &dst) {
#ifdef IS_PY3K
        if (PyBytes_Check(src)) {
            dst = PyBytes_AsString(src);
#else
        if (PyString_Check(src)) {
            dst = PyString_AsString(src);
#endif
            return true;
        } else {
            return false;
        }
    }
};

template <> struct TypeInfo<Vector3> : TypeInfoBase<Vector3> {
    TypeInfo() : TypeInfoBase<Vector3>("Vector3", VT_CUSTOM_TYPE){};

    static PyObject *toPyObject(const Vector3 &val) {
        return Py_BuildValue("[fff]", val.x, val.y, val.z);
    }

    static bool fromPyObject(PyObject *src, Vector3 &dst) {
        float x, y, z;
        if (PyArg_Parse(src, "[fff]", &x, &y, &z)) {
            dst.x = x;
            dst.y = y;
            dst.z = z;
            return true;
        } else {
            return false;
        }
    }
};

template <> struct TypeInfo<Quaternion> : TypeInfoBase<Quaternion> {
    TypeInfo() : TypeInfoBase<Quaternion>("Quaternion", VT_CUSTOM_TYPE){};

    static PyObject *toPyObject(const Quaternion &val) {
        return Py_BuildValue("[ffff]", val.x, val.y, val.z, val.w);
    }

    static bool fromPyObject(PyObject *src, Quaternion &dst) {
        float x, y, z, w;
        if (PyArg_Parse(src, "[ffff]", &x, &y, &z, &w)) {
            dst.x = x;
            dst.y = y;
            dst.z = z;
            dst.z = w;
            return true;
        } else {
            return false;
        }
    }
};

template <>
struct TypeInfo<Ogre::ColourValue> : TypeInfoBase<Ogre::ColourValue> {
    TypeInfo()
        : TypeInfoBase<Ogre::ColourValue>("Ogre::ColourValue",
                                          VT_CUSTOM_TYPE){};

    static PyObject *toPyObject(const Ogre::ColourValue &val) {
        return Py_BuildValue("[ffff]", val.r, val.g, val.b, val.a);
    }

    static bool fromPyObject(PyObject *src, Ogre::ColourValue &dst) {
        float r, g, b, a;
        if (PyArg_Parse(src, "[ffff]", &r, &g, &b, &a)) {
            dst.r = r;
            dst.g = g;
            dst.b = b;
            dst.a = a;
            return true;
        } else {
            return false;
        }
    }
};

// Global utilities - object conversion and such
PyObject *DVariantToPyObject(const DVariant &inst);
DVariant PyObjectToDVariant(PyObject *obj);

// DVariant type info
template <> struct TypeInfo<DVariant> : TypeInfoBase<DVariant> {
    TypeInfo() : TypeInfoBase<DVariant>("DVariant", VT_CUSTOM_TYPE){};

    static PyObject *toPyObject(const DVariant &val) {
        return DVariantToPyObject(val);
    }

    static bool fromPyObject(PyObject *src, DVariant &dst) {
        dst = PyObjectToDVariant(src);
        return true;
    }
};

// One casting info slot. Contains Python type object an casting method pointer
template <class C> struct CastInfo {
    PyTypeObject *type;
    /** Upcasting method. Should return a static_cast'ed instance if class C
     * given the wrapping repr. of object Example:
     * @code
     * ParentClass *castChildToParent(PyObject* obj) {
     * 	ChildClass* myi =
     * reinterpret_cast<ObjectBase<ChildClass>*>(obj)->mInstance;
     * 	assert(mywrap->ob_type == msType);
     * 	return static_cast<Parent>(myi);
     * }
     * @endcode
     * @see defaultPythonCaster for default impl.
     * @todo It should be possible to templatize the casting method and
     * reference it
     */
    typedef C *(*CastMethod)(PyObject *obj);
    CastMethod caster;
};

/// Template definition of a Python instance holding a single object
template <typename T> struct ObjectBase { PyObject_HEAD T mInstance; };

/// Upcaster for python side class inheritance resolution for non-smartptr
/// objects
template <class P, class C> P *defaultPythonUpcaster(PyObject *obj) {
    ObjectBase<C> *mywrap = reinterpret_cast<ObjectBase<C> *>(obj);
    return reinterpret_cast<P*>(&mywrap->mInstance);
}

/** helper function to get user data from Python's Object
 * @param obj The source object to extract
 * @param typ The type object of our type
 * @param target Pointer to Type instance. Filled with the pointer to the
 * instance in the object
 * @param castInfo The optional structure containing casting methods for
 * conversion casts */
template <typename T>
bool python_cast(PyObject *obj, PyTypeObject *type, T *target,
                 CastInfo<T> *castInfo = NULL) {
    // reinterpret cast is not a correct operation on subclasses
    // so we need the type info to agree

    // we can try searching the cast info for subtypes if those differ
    PyTypeObject *ot = obj->ob_type;

    if (ot != type) {
        CastInfo<T> *current = castInfo;
        if (current != NULL) {
            for (; current->type != NULL; ++current) {
                // do we have a match?
                if (ot == current->type) {
                    // yep. Cast using the upcaster
                    *target = *current->caster(obj);
                    return true;
                }
            }
        }

        return false;
    }

    *target = reinterpret_cast<ObjectBase<T> *>(obj)->mInstance;
    return true;
}

/** helper function to get user data ptr from Python's Object
 * @param obj The source object to extract
 * @param typ The type object of our type
 * @param target Pointer to Type instance. Filled with the pointer to the
 * instance in the object
 * @param castInfo The optional structure containing casting methods for
 * conversion casts */
template <typename T>
T *python_cast(PyObject *obj, PyTypeObject *type,
               CastInfo<T> *castInfo = NULL) {
    // reinterpret cast is not a correct operation on subclasses
    // so we need the type info to agree

    // we can try searching the cast info for subtypes if those differ
    PyTypeObject *ot = obj->ob_type;

    if (ot != type) {
        CastInfo<T> *current = castInfo;
        if (current != NULL) {
            for (; current->type != NULL; ++current) {
                // do we have a match?
                if (ot == current->type) {
                    // yep. Cast using the upcaster
                    return current->caster(obj);
                }
            }
        }

        return nullptr;
    }

    return &reinterpret_cast<ObjectBase<T> *>(obj)->mInstance;
}

// Conversion error for simplicity
#define __PY_CONVERR_RET_VAL(val)                                              \
    {                                                                          \
        PyErr_SetString(PyExc_TypeError, "Incompatible types error.");         \
        return val;                                                            \
    }
#define __PY_CONVERR_RET __PY_CONVERR_RET_VAL(NULL)
#define __PY_BADPARM_RET(parm)                                                 \
    {                                                                          \
        PyErr_SetString(PyExc_TypeError,                                       \
                        "Incompatible parameter type on '" #parm "'");         \
        return NULL;                                                           \
    }
#define __PY_BADPARMS_RET                                                      \
    {                                                                          \
        PyErr_SetString(PyExc_TypeError, "Incompatible parameters");           \
        return NULL;                                                           \
    }
#define __PY_NONE_RET                                                          \
    {                                                                          \
        Py_INCREF(Py_None);                                                    \
        return Py_None;                                                        \
    }

/// Common ancestor for all python published C types
class PythonPublishedType {
protected:
    /** Publishes the type as a member of a specified module
        @param containter The module to publish the type in
        @param type The python type object to publish
        @param name The name of the type to use
    */
    static void publishType(PyObject *containter, PyTypeObject *type,
                            const char *name);
};

template<typename T>
class object_binder : public PythonPublishedType {
public:
    /// A python object type
    typedef ObjectBase<T> Object;

protected:
    /// A sort-of constructor method. To be used to create a new NULL Object*
    template<typename...ArgsT>
    static Object *construct(PyTypeObject *type, ArgsT&&...args) {
        Object *object;

        object = PyObject_New(Object, type);

        // At this point, the shared_ptr instance in the object is invalid (I.E.
        // Allocated, but not initialized). If we try to assign into it, we'll
        // segfault. Because of that, we have to do placed new to initialize the
        // object
        if (object != nullptr) {
            ::new (&object->mInstance) T(std::forward<ArgsT>(args)...);
        }

        return object;
    }

    /// Destructor for the python object. Safely decreases the reference to the
    /// shared_ptr. To be used in msType
    static void dealloc(PyObject *self) {
        // cast the object to T::Object
        Object *o = reinterpret_cast<Object *>(self);

        // Call the destructor to clean up
        (&o->mInstance)->~T();

        // Finally delete the object
        PyObject_Del(self);
    }
};


/// A template that binds sharedptr typed classes
template <typename T> class shared_ptr_binder : public object_binder<T> {
public:
    /// A python object type
    using Object = typename object_binder<T>::Object;

protected:
    /// A sort-of constructor method. To be used to create a new NULL Object*
    static Object *construct(PyTypeObject *type) {
        return object_binder<T>::construct(type);
    }

    /// Destructor for the python object. Safely decreases the reference to the
    /// shared_ptr. To be used in msType
    static void dealloc(PyObject *self) {
        // cast the object to T::Object
        Object *o = reinterpret_cast<Object *>(self);

        // Decreases the shared_ptr counter (just to be sure here, dtor does
        // that to)
        o->mInstance.reset();

        object_binder<T>::dealloc(self);
    }
};

/// A template that binds a pointer to class (simmilar to shared_ptr_binder, but
/// no special handling is used)
template <typename T> class class_ptr_binder : public PythonPublishedType {
protected:
    static PyTypeObject msType;

public:
    /// A python object type
    typedef ObjectBase<T *> Object;

protected:
    /// A sort-of constructor method. To be used to create a new NULL Object*
    static Object *construct(PyTypeObject *type) {
        Object *object;

        object = PyObject_New(Object, type);

        if (object != NULL) {
            // Here, tidy!
            object->mInstance = NULL;
        }

        return object;
    }

    /// Destructor for the python object. Safely decreases the reference to the
    /// shared_ptr. To be used in msType
    static void dealloc(PyObject *self) {
        // Object* o = reinterpret_cast<Object*>(self);
        // delete the object
        PyObject_Del(self);
    }
};

}; // namespace Python

/** Central class for python bindings. Call PythonLanguage::Init() to prepare
 * python environment */
class OPDELIB_EXPORT PythonLanguage {
protected:
    static Opde::Root *msRoot;

public:
    /** Initializes python lang and all the bindings */
    static void init(int argc, char **argv);

    /** Initializes the Opde module itself, without the interpretter */
    static PyObject *initModule();

    /** Finalizes python lang */
    static void term();

    /** Runs a script loaded in memory on a given address */
    static void runScriptPtr(const char *ptr);

    /** Runs a script from a file */
    static bool runScript(const char *fname);

    /// Python side Root singleton handler
    static PyObject *createRoot(PyObject *self, PyObject *args);

    /// Python side Root singleton handler
    static PyObject *getRoot(PyObject *self, PyObject *args);
};

} // namespace Opde

#endif
