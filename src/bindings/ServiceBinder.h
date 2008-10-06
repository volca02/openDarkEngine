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
 
#ifndef __SERVICEBINDER_H
#define __SERVICEBINDER_H

#include "bindings.h"

namespace Opde {
	namespace Python {
		
		/** A Service interface for python. Manages a module "Opde.services".
		* Exposes getters for services. Each getter is named exactly the same as the service itself.
		* @note When creating a new service, remember to expose it here
		*/
		class ServiceBinder {
			public:
				static PyObject* init(PyObject* container);
			
				static PyObject* getConfigService(PyObject* self, PyObject* args);
				static PyObject* getLinkService(PyObject* self, PyObject* args);
				static PyObject* getPropertyService(PyObject* self, PyObject* args);
				static PyObject* getLoopService(PyObject* self, PyObject* args);
				static PyObject* getInputService(PyObject* self, PyObject* args);
				static PyObject* getGUIService(PyObject* self, PyObject* args);
				static PyObject* getDatabaseService(PyObject* self, PyObject* args);
				static PyObject* getObjectService(PyObject* self, PyObject* args);
				static PyObject* getInheritService(PyObject* self, PyObject* args);
				
			protected:
				static PyMethodDef msMethods[];
				static char* msName;
		};
	}
}


#endif
