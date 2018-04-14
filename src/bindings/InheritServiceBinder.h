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

#ifndef __INHERITSERVICEBINDER_H
#define __INHERITSERVICEBINDER_H

#include "bindings.h"

#include "InheritService.h"

namespace Opde {

namespace Python {

/// Inherit service python binder
class InheritServiceBinder : public shared_ptr_binder<InheritServicePtr> {
public:
    static void init(PyObject *module);

    // --- Python type related methods ---
    static PyObject *create();

    // --- Methods ---
    static PyObject *getSources(PyObject *self, PyObject *args);
    static PyObject *getTargets(PyObject *self, PyObject *args);
    static PyObject *hasTargets(PyObject *self, PyObject *args);
    static PyObject *getArchetype(PyObject *self, PyObject *args);
    static PyObject *setArchetype(PyObject *self, PyObject *args);
    // metaprop handling is already binded in ObjectService
    static PyObject *inheritsFrom(PyObject *self, PyObject *args);

protected:
    /// Static type definition for LinkService
    static PyTypeObject msType;

    /// Name of the python type
    static const char *msName;

    /// Method list
    static PyMethodDef msMethods[];
};

// -------------------------------
/// Inherit link struct binder. The attributes are exposed as read only to
/// python
class InheritLinkBinder : public shared_ptr_binder<InheritLinkPtr> {
public:
    static void init(PyObject *module);

    // --- Python type related methods ---
    static PyObject *getattr(PyObject *self, char *name);

    static PyObject *create(InheritLinkPtr &link);

protected:
    /// Static type definition for LinkService
    static PyTypeObject msType;

    /// Name of the python type
    static const char *msName;
};

} // namespace Python
} // namespace Opde

#endif
