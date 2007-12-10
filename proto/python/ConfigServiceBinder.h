#ifndef __PYCONFIGSERVICE_H
#define __PYCONFIGSERVICE_H

#include  "ConfigService.h"

namespace Opde {
	
	namespace Python {
		
		/// Config service python binder
		class ConfigServiceBinder {
			public:
				static void init();
			
				// --- Python type releted methods ---
				
				static void dealloc(PyObject *self);
				static PyObject* getattr(PyObject *self, char *name);
					
				static PyObject* create();
				
				// --- Methods ---
				
				static PyObject* setParam(PyObject* self, PyObject* args);
				static PyObject* getParam(PyObject* self, PyObject* args);
				static PyObject* hasParam(PyObject* self, PyObject* args);
				static PyObject* loadParams(PyObject* self, PyObject* args);
				
			protected:
				/// Python object instance definition
				typedef ObjectBase<ConfigServicePtr> Object;
				
				/// Static type definition for ConfigService
				static PyTypeObject msType;
				
				/// Name of the python type
				static char* msName;
				
				/// Method list
				static PyMethodDef msMethods[];
		};
	}
}

#endif
