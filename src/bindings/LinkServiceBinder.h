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

#ifndef __LINKSERVICEBINDER_H
#define __LINKSERVICEBINDER_H

#include  "DTypeDef.h"
#include  "BinaryService.h"
#include  "LinkService.h"
#include  "DTypeBinder.h"

namespace Opde {

	namespace Python {

		/// Link service python binder
		class LinkServiceBinder : public shared_ptr_binder<LinkServicePtr> {
			public:
				static void init(PyObject* module);

				// --- Python type related methods ---
				static PyObject* getattr(PyObject *self, char *name);

				static PyObject* create();

				// --- Methods ---
				static PyObject* setChunkVersion(PyObject* self, PyObject* args);
				static PyObject* nameToFlavor(PyObject* self, PyObject* args);
				static PyObject* flavorToName(PyObject* self, PyObject* args);
				static PyObject* getRelation(PyObject* self, PyObject* args);
				static PyObject* getAllLinks(PyObject* self, PyObject* args);
				static PyObject* getOneLink(PyObject* self, PyObject* args);
				static PyObject* getAllLinkNames(PyObject* self, PyObject* args);
				static PyObject* getFieldsDesc(PyObject* self, PyObject* args);

			protected:
				/// Static type definition for LinkService
				static PyTypeObject msType;

				/// Name of the python type
				static char* msName;

				/// Method list
				static PyMethodDef msMethods[];
		};

		// -------------------------------
		/// Link class binder. The methods are converted to read-only attributes
		class LinkBinder : public shared_ptr_binder<LinkPtr> {
		    public:
				static void init(PyObject* module);

				// --- Python type related methods ---
				static PyObject* getattr(PyObject *self, char *name);

				static PyObject* create(LinkPtr& link);

            protected:
				/// Static type definition for LinkService
				static PyTypeObject msType;

				/// Name of the python type
				static char* msName;
		};

	}
}

#endif
