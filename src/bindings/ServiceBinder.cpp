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
 
#include "ServiceBinder.h"

#include "ConfigServiceBinder.h"
#include "LinkServiceBinder.h"
#include "PropertyServiceBinder.h"
#include "LoopServiceBinder.h"
#include "InputServiceBinder.h"
#include "GUIServiceBinder.h"
#include "DatabaseServiceBinder.h"

namespace Opde {
	namespace Python {
		
		// ---------------------- class Services --------------------
		char* ServiceBinder::msName = "Opde.Services";
		
		PyMethodDef ServiceBinder::msMethods[] = {
			{"ConfigService", getConfigService, METH_NOARGS},
			{"LinkService", getLinkService, METH_NOARGS},
			{"PropertyService", getPropertyService, METH_NOARGS},
			{"LoopService", getLoopService, METH_NOARGS},
			{"InputService", getInputService, METH_NOARGS},
			{"GUIService", getGUIService, METH_NOARGS},
			{"DatabaseService", getDatabaseService, METH_NOARGS},
			{NULL, NULL},
		};
		
		PyObject* ServiceBinder::getConfigService(PyObject* self, PyObject* args) {
			try {
                return ConfigServiceBinder::create();
			} catch (BasicException &e) {
			    PyErr_Format(PyExc_EnvironmentError, "Exception happened while getting service: %s", e.getDetails().c_str());
			    return NULL;
			}
		}
		
		PyObject* ServiceBinder::getLinkService(PyObject* self, PyObject* args) {
			try {
                return LinkServiceBinder::create();
			} catch (BasicException &e) {
			    PyErr_Format(PyExc_EnvironmentError, "Exception happened while getting service: %s", e.getDetails().c_str());
			    return NULL;
			}
		}

		PyObject* ServiceBinder::getPropertyService(PyObject* self, PyObject* args) {
			try {
                return PropertyServiceBinder::create();
			} catch (BasicException &e) {
			    PyErr_Format(PyExc_EnvironmentError, "Exception happened while getting service: %s", e.getDetails().c_str());
			    return NULL;
			}
		}

		PyObject* ServiceBinder::getLoopService(PyObject* self, PyObject* args) {
			try {
                return LoopServiceBinder::create();
			} catch (BasicException &e) {
			    PyErr_Format(PyExc_EnvironmentError, "Exception happened while getting service: %s", e.getDetails().c_str());
			    return NULL;
			}
		}

		PyObject* ServiceBinder::getInputService(PyObject* self, PyObject* args) {
			try {
                return InputServiceBinder::create();
			} catch (BasicException &e) {
			    PyErr_Format(PyExc_EnvironmentError, "Exception happened while getting service: %s", e.getDetails().c_str());
			    return NULL;
			}
		}

		PyObject* ServiceBinder::getGUIService(PyObject* self, PyObject* args) {
			try {
                return GUIServiceBinder::create();
			} catch (BasicException &e) {
			    PyErr_Format(PyExc_EnvironmentError, "Exception happened while getting service: %s", e.getDetails().c_str());
			    return NULL;
			}
		}
		
		PyObject* ServiceBinder::getDatabaseService(PyObject* self, PyObject* args) {
			try {
                return DatabaseServiceBinder::create();
			} catch (BasicException &e) {
			    PyErr_Format(PyExc_EnvironmentError, "Exception happened while getting service: %s", e.getDetails().c_str());
			    return NULL;
			}
		}

		PyObject* ServiceBinder::init(PyObject* container) {
			PyObject* module = Py_InitModule(msName, msMethods);
			
			assert(module);
			
			// Register itself as a member of the container we got
			PyObject *dir = PyModule_GetDict(container);
			PyDict_SetItemString(dir, "Services", module);
			
			return module;
		}
		
	}
}
