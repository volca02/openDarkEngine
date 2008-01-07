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

#ifndef __QUICKGUIBINDINGS_H
#define __QUICKGUIBINDINGS_H

#include  "bindings.h"
#include  "QuickGUIEventArgs.h"
#include  "QuickGUIManager.h"

namespace Opde {

	namespace Python {
		/// QuickGUI manager
		class QG_ManagerBinder : public class_ptr_binder<QuickGUI::GUIManager> {
			public:
				// --- Python type related methods ---
				static PyObject* getattr(PyObject *self, char *name);

				/// to string - reprfunc conversion
				static PyObject* repr(PyObject *self);

				/// creates a python object representation of the Manager
				static PyObject* create(QuickGUI::GUIManager *mgr);

				// --- Methods ---
			protected:
				/// Static type definition
				static PyTypeObject msType;

				/// Name of the python type
				static char* msName;

				/// Method list
				static PyMethodDef msMethods[];
		};

		/// Base Widget representation
		class QG_WidgetBinder : public class_ptr_binder<QuickGUI::Widget> {
				public:
				// --- Python type related methods ---
				static PyObject* getattr(PyObject *self, char *name);

				/// to string - reprfunc conversion
				static PyObject* repr(PyObject *self);

				/// creates a python object representation of the Manager
				static PyObject* create(QuickGUI::Widget *wgt);

				// --- Methods ---
				static PyObject* addChild(PyObject *self, PyObject *args);
				static PyObject* addEventHandler(PyObject *self, PyObject *args);

			protected:
				/// Static type definition for LinkService
				static PyTypeObject msType;

				/// Name of the python type
				static char* msName;

				/// Method list
				static PyMethodDef msMethods[];
		};
	}
}

#endif
