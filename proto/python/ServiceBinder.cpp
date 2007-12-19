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

namespace Opde {
	namespace Python {
		
		// ---------------------- class Services --------------------
		char* ServiceBinder::msName = "Opde.Services";
		
		PyMethodDef ServiceBinder::msMethods[] = {
			{"ConfigService", getConfigService, METH_NOARGS},
			{"LinkService", getLinkService, METH_NOARGS},
			{"PropertyService", getPropertyService, METH_NOARGS},
			{NULL, NULL},
		};
		
		PyObject* ServiceBinder::getConfigService(PyObject* self, PyObject* args) {
			return ConfigServiceBinder::create();
		}
		
		PyObject* ServiceBinder::getLinkService(PyObject* self, PyObject* args) {
			return LinkServiceBinder::create();
		}

		PyObject* ServiceBinder::getPropertyService(PyObject* self, PyObject* args) {
			return PropertyServiceBinder::create();
		}

		PyObject* ServiceBinder::init(PyObject* container) {
			PyObject* module = Py_InitModule(msName, msMethods);
			
			ConfigServiceBinder::init();
			
			assert(module);
			
			// Register itself as a member of the container we got
			PyObject *dir = PyModule_GetDict(container);
			PyDict_SetItemString(dir, "Services", module);
			
			return module;
		}
		
	}
}
