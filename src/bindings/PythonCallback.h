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

#ifndef __PYTHONCALLBACK_H
#define __PYTHONCALLBACK_H

#include "Callback.h"

namespace Opde 
{
	/// Python callback template. MSG marks the message sent, C the converter that is a functor - converts the message to PyObject* as required
	/// @note Please catch exceptions upon construction. There is no way to pas PyErr directly!
	template<typename MSG, typename C> class PythonCallback : public Callback<MSG> {
		public:
			PythonCallback(PyObject* callable) : mCallable(callable), mConverter() { 
				if (!PyCallable_Check(mCallable))  
					// WOULD BE: PyErr_SetString(PyExc_TypeError, "Python callback can't be constructed on non-callable!");
					OPDE_EXCEPT("Python callback can't be constructed on non-callable!", "PythonCallback::PythonCallback");
				
				Py_INCREF(mCallable); 
			};
			
			~PythonCallback() { 
				Py_DECREF(mCallable);
			};
			
			virtual void operator ()(const MSG& msg) {
				PyObject* py_msg = mConverter(msg);
				
				// Call the pyobject
				PyObject* args; 
				args = PyTuple_New(1);
				
				PyTuple_SetItem(args, 0, py_msg); // Steals reference
				
				PyObject* rslt = PyObject_CallObject(mCallable, args);
				
				if (rslt) { 
				    // To be sure no leak happened
					Py_DECREF(rslt);
				}
				
				Py_DECREF(args);
			}
			
		protected:
			PyObject* mCallable;
			C mConverter;
	};
}

#endif
