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

#include "ServiceBinder.h"

#include "ConfigServiceBinder.h"
#include "LinkServiceBinder.h"
#include "PropertyServiceBinder.h"
#include "LoopServiceBinder.h"
#include "InputServiceBinder.h"
#include "DrawServiceBinder.h"
#include "GUIServiceBinder.h"
#include "DatabaseServiceBinder.h"
#include "ObjectServiceBinder.h"
#include "InheritServiceBinder.h"

namespace Opde {
	namespace Python {

		// ---------------------- class Services --------------------
		const char* ServiceBinder::msName = "opde.services";

		PyMethodDef ServiceBinder::msMethods[] = {
			{const_cast<char*>("getConfigService"), getConfigService, METH_NOARGS, "getConfigService() -> opde.services.ConfigService\n\tReturns a reference object to the ConfigService instance"},
			{const_cast<char*>("getLinkService"), getLinkService, METH_NOARGS, "getLinkService() -> opde.services.LinkService\n\tReturns a reference object to the LinkService instance"},
			{const_cast<char*>("getPropertyService"), getPropertyService, METH_NOARGS, "getPropertyService() -> opde.services.PropertyService\n\tReturns a reference object to the PropertyService instance"},
			{const_cast<char*>("getLoopService"), getLoopService, METH_NOARGS},
			{const_cast<char*>("getInputService"), getInputService, METH_NOARGS},
			{const_cast<char*>("getDrawService"), getDrawService, METH_NOARGS},
			{const_cast<char*>("getGUIService"), getGUIService, METH_NOARGS},
			{const_cast<char*>("getDatabaseService"), getDatabaseService, METH_NOARGS},
			{const_cast<char*>("getObjectService"), getObjectService, METH_NOARGS},
			{const_cast<char*>("getInheritService"), getInheritService, METH_NOARGS},
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

		PyObject* ServiceBinder::getDrawService(PyObject* self, PyObject* args) {
			try {
        		    return DrawServiceBinder::create();
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

		PyObject* ServiceBinder::getObjectService(PyObject* self, PyObject* args) {
			try {
                return ObjectServiceBinder::create();
			} catch (BasicException &e) {
			    PyErr_Format(PyExc_EnvironmentError, "Exception happened while getting service: %s", e.getDetails().c_str());
			    return NULL;
			}
		}

		PyObject* ServiceBinder::getInheritService(PyObject* self, PyObject* args) {
			try {
                return InheritServiceBinder::create();
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
			PyDict_SetItemString(dir, "services", module);

			// init the services (we init all, they register into the module, enabling pydoc usage on them)
			ConfigServiceBinder::init(module);
			PropertyServiceBinder::init(module);
			DatabaseServiceBinder::init(module);
			InputServiceBinder::init(module);
			DrawServiceBinder::init(module);
			GUIServiceBinder::init(module);
			LinkServiceBinder::init(module);
			LoopServiceBinder::init(module);
			ObjectServiceBinder::init(module);
			InheritServiceBinder::init(module);

			// Publish some service constants
			PyModule_AddIntConstant(module, "SERVICE_ALL", SERVICE_ALL);
			PyModule_AddIntConstant(module, "SERVICE_CORE", SERVICE_CORE);
			PyModule_AddIntConstant(module, "SERVICE_RENDERER", SERVICE_RENDERER);
			PyModule_AddIntConstant(module, "SERVICE_ENGINE", SERVICE_ENGINE);
			PyModule_AddIntConstant(module, "SERVICE_LINK_LISTENER", SERVICE_LINK_LISTENER);
			PyModule_AddIntConstant(module, "SERVICE_PROPERTY_LISTENER", SERVICE_PROPERTY_LISTENER);
			PyModule_AddIntConstant(module, "SERVICE_OBJECT_LISTENER", SERVICE_OBJECT_LISTENER);
			PyModule_AddIntConstant(module, "SERVICE_DATABASE_LISTENER", SERVICE_DATABASE_LISTENER);
			PyModule_AddIntConstant(module, "SERVICE_INPUT_LISTENER", SERVICE_INPUT_LISTENER);

			return module;
		}

	}
}
