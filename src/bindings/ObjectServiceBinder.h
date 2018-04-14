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

#ifndef __OBJECTSERVICEBINDER_H
#define __OBJECTSERVICEBINDER_H

#include "ObjectService.h"
#include "bindings.h"

namespace Opde {
namespace Python {

/// Object service python binder
class ObjectServiceBinder : public shared_ptr_binder<ObjectServicePtr> {
public:
    static void init(PyObject *module);

    // --- Python type related methods ---
    static PyObject *getattr(PyObject *self, char *name);

    /// creates a python object representation of the ObjectService
    static PyObject *create();

    // --- Methods ---
    // named differently to avoid mixing with the prev. create method
    static PyObject *objectCreate(PyObject *self, PyObject *args);
    static PyObject *beginCreate(PyObject *self, PyObject *args);
    static PyObject *endCreate(PyObject *self, PyObject *args);
    static PyObject *exists(PyObject *self, PyObject *args);
    static PyObject *position(PyObject *self, PyObject *args);
    static PyObject *orientation(PyObject *self, PyObject *args);
    static PyObject *getName(PyObject *self, PyObject *args);
    static PyObject *setName(PyObject *self, PyObject *args);
    static PyObject *named(PyObject *self, PyObject *args);
    static PyObject *teleport(PyObject *self, PyObject *args);
    static PyObject *addMetaProperty(PyObject *self, PyObject *args);
    static PyObject *removeMetaProperty(PyObject *self, PyObject *args);
    static PyObject *hasMetaProperty(PyObject *self, PyObject *args);

protected:
    /// Static type definition for ObjectService
    static PyTypeObject msType;

    /// Name of the python type
    static const char *msName;

    /// Method list
    static PyMethodDef msMethods[];
};
} // namespace Python
} // namespace Opde

#endif // __OBJECTSERVICEBINDER_H
