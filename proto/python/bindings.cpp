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
 *	   $Id:$
 *
 *****************************************************************************/
 
#include "bindings.h"
#include "ServiceBinder.h"

namespace Opde {
	namespace Python {

		PyObject* DVariantToPyObject(const DVariant& inst) {
			// Do a conversion to python object from variant
			// Look for the type
			switch (inst.type()) {
				case DVariant::DV_INVALID:
					Py_INCREF(Py_None);
					return Py_None;

				case DVariant::DV_BOOL: {
							PyObject* ret = inst.toBool() ? Py_True : Py_False;
							Py_INCREF(ret);
							return ret;
					}

				case DVariant::DV_FLOAT:
					return PyFloat_FromDouble(inst.toFloat());

				case DVariant::DV_INT:
					return PyInt_FromLong(inst.toInt());

				case DVariant::DV_UINT:
					return PyInt_FromLong(inst.toUInt());

				case DVariant::DV_STRING:
					return PyString_FromString(inst.toString().c_str());

				case DVariant::DV_VECTOR: {
					// Build a touple
					const Ogre::Vector3& v = inst.toVector();
					return Py_BuildValue("[fff]", v.x, v.y, v.z);
				}
			}
		}
		
		DVariant PyObjectToDVariant(PyObject* obj) {
			// Do a conversion from python object to DVariant instance
			// Look for the type of the python object
			
		}

	} // namespace Python
	
	PyMethodDef sOpdeMethods[] = {
		{NULL, NULL},
	};
	
	void PythonLanguage::init() {
		Py_Initialize();
		
		// Create an Opde module
		PyObject* module = Py_InitModule("Opde", sOpdeMethods);
		
		// Call all the binders here. The result is initialized Python VM
		PyObject *servicemod = Python::ServiceBinder::init(module);
		
		if (PyErr_Occurred()) {
			PyErr_Print();
			PyErr_Clear();
		}

	}
	
	void PythonLanguage::term() {
		Py_Finalize();
	}

} // namespace Opde
