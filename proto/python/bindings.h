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
 
#ifndef __BINDINGS_H
#define __BINDINGS_H

#include "DVariant.h"

#include <Python.h>
#include <OgreVector3.h>

namespace Opde {
	namespace Python {
		// Global utilities - object conversion and such
		PyObject* DVariantToPyObject(const DVariant& inst);
		DVariant PyObjectToDVariant(PyObject* obj);
		
		/// Template definition of a Python instance holding a single object
		template<typename T> struct ObjectBase {
			PyObject_HEAD
			T mInstance;
		};
		
		/// helper function to get type from Object
		template<typename T> T python_cast(PyObject* obj, PyTypeObject* type) {
			assert(obj->ob_type == type);

			return reinterpret_cast< T >(obj);
		}
		
	};
	
    /** Central class for python bindings. Call PythonLanguage::Init() to prepare python environment */
    class PythonLanguage {
    	public:
			/** Initializes python lang and all the bindings */
			static void init();
			
			/** Finalizes python lang */
			static void term();
    };
}


#endif
