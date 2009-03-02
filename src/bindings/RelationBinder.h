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

#ifndef __RELATIONBINDER_H
#define __RELATIONBINDER_H

#include  "DTypeDef.h"
#include  "BinaryService.h"
#include  "LinkService.h"
#include  "Relation.h"

namespace Opde {

	namespace Python {

		/// Link relation python binder
		class RelationBinder : public shared_ptr_binder<RelationPtr> {
			public:
				static void init(PyObject* module);

				// --- Python type related methods ---
				static PyObject* getattr(PyObject *self, char *name);

				/// to string - reprfunc conversion
				static PyObject* repr(PyObject *self);

				/// creates a python object representation of the relation
				static PyObject* create(const RelationPtr& relation);

				// --- Methods ---
				static PyObject* getID(PyObject* self, PyObject* args);
				static PyObject* getName(PyObject* self, PyObject* args);
				static PyObject* remove(PyObject* self, PyObject* args);
				static PyObject* createLink(PyObject* self, PyObject* args);
				static PyObject* getLinkField(PyObject* self, PyObject* args);
				static PyObject* setLinkField(PyObject* self, PyObject* args);

				static PyObject* getAllLinks(PyObject* self, PyObject* args);
				static PyObject* getOneLink(PyObject* self, PyObject* args);

				static PyObject* getFieldsDesc(PyObject* self, PyObject* args);

			protected:
				/// Static type definition for LinkService
				static PyTypeObject msType;

				/// Name of the python type
				static const char* msName;

				/// Method list
				static PyMethodDef msMethods[];
		};
	}
}

#endif
