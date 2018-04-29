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

#ifndef __PYTHONCALLBACK_H
#define __PYTHONCALLBACK_H

#include "bindings.h"
#include "Callback.h"

namespace Opde {

template<typename MSG, typename CVT>
void call_python_callback(Python::Object &callable, const MSG &msg) {
    using Object = Python::Object;

    // already increased refcount here.
    Object py_msg{CVT::convert(msg), Object::TAKE_REF};

    // Call the pyobject
    Object args{PyTuple_New(1), Object::TAKE_REF};
    PyTuple_SetItem(args, 0, py_msg.release()); // Steals reference

    {
        Object rslt = PyObject_CallObject(callable, args);
        // will ignore the result, but decref it!
    }

    __PY_HANDLE_PYTHON_ERROR; // converts python side error to
    // PythonException
}

/// Python callback template. MSG marks the message sent, C the converter that
/// is a functor - converts the message to PyObject* as required
/// @note Please catch exceptions upon construction. There is no way to pas
/// PyErr directly!
template <typename MSG, typename CVT>
class PythonCallback : public Callback<MSG> {
public:
    PythonCallback(PyObject *callable) : mCallable(callable) {
        if (!PyCallable_Check(mCallable))
            // WOULD BE: PyErr_SetString(PyExc_TypeError, "Python callback can't
            // be constructed on non-callable!");
            OPDE_EXCEPT(
                "Python callback can't be constructed on non-callable!");
    };

    ~PythonCallback() { Py_DECREF(mCallable); };

    virtual void operator()(const MSG &msg) {
        call_python_callback<MSG, CVT>(mCallable, msg);
    }

protected:
    Python::Object mCallable;
};

} // namespace Opde

#endif
