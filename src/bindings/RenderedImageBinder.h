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

#ifndef __RENDEREDIMAGEBINDER_H
#define __RENDEREDIMAGEBINDER_H

#include "bindings.h"

#include "draw/RenderedImage.h"

namespace Opde {

namespace Python {

/// RenderedImage python binder
class RenderedImageBinder : public class_ptr_binder<RenderedImage> {
    // for msType handling
    friend class DrawOperationBinder;

public:
    static void init(PyObject *module);

    // --- Python type related methods ---
    /// to string - reprfunc conversion
    static PyObject *repr(PyObject *self);

    /// helper class pointer extractor
    static bool extract(PyObject *obj, RenderedImage *&tgt);

    // --- Methods ---
    static PyObject *setDrawSource(PyObject *self, PyObject *args);

    static PyObject *create(RenderedImage *ds);

protected:
    /// Static type definition
    static PyTypeObject msType;

    /// Name of the python type
    static const char *msName;

    /// Method list
    static PyMethodDef msMethods[];

    static TypeInfo<ClipRect> msRectTypeInfo;
};

} // namespace Python
} // namespace Opde

#endif
