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

#ifndef __BINDINGS_H
#define __BINDINGS_H

#include "DVariant.h"
#include "Root.h"

#include <Python.h>
#include <OgreVector3.h>

// Exception guard for the python code. use this as first/last lines of the binding function to have it prepared for exceptions
#define __PYTHON_EXCEPTION_GUARD_BEGIN_ try {
#define __PYTHON_EXCEPTION_GUARD_END_ } catch (BasicException& e) { \
			        PyErr_Format(PyExc_RuntimeError, "C++ side exception (%s:%u) : %s", __FILE__, __LINE__, e.getDetails().c_str()); \
			        return NULL; \
			    }

#define __PYTHON_EXCEPTION_GUARD_END_RVAL(rval) } catch (BasicException& e) { \
			        PyErr_Format(PyExc_RuntimeError, "C++ side exception (%s:%u) : %s", __FILE__, __LINE__, e.getDetails().c_str()); \
			        return rval; \
			    }

namespace Opde {
	namespace Python {
	    // Type converters
	    enum VariableType { VT_INVALID, VT_BOOL, VT_INT, VT_LONG, VT_FLOAT, VT_CHARPTR, VT_STRING, VT_CUSTOM_TYPE };

		template<typename T> struct TypeInfoBase {
			const char* typeName;
			VariableType type;
			
			TypeInfoBase(const char* tn, VariableType tt) : typeName(tn), type(tt) {};
		};

		template<typename T> struct TypeInfo : public TypeInfoBase<T> {
			TypeInfo() : TypeInfoBase<T>("invalid", VT_INVALID) {};

			// TypeInfo(const char* tname, VariableType tt) : TypeInfoBase(tname, tt) {};

			static PyObject* toPyObject(T val) {
				PyErr_SetString(PyExc_TypeError, "Binding error: Type has no conversion or TypeInfo specified!");
				return NULL;
			}
			
			static bool fromPyObject(PyObject* src, T& dst) {
				return false;
			}
		};


		template<> struct TypeInfo<bool> : TypeInfoBase<bool> {
			TypeInfo() : TypeInfoBase<bool>("bool", VT_BOOL) {};

			static PyObject* toPyObject(bool val) {
				PyObject* res = val ? Py_True : Py_False;

				Py_INCREF(res);

				return res;
			}
			
			static bool fromPyObject(PyObject* src, bool& dst) {
				if (PyBool_Check(src)) {
					dst = (src == Py_True);
					return true;
				} else {
					return false;
				}
			}
		};

		template<> struct TypeInfo<int> : TypeInfoBase<int> {
			TypeInfo() : TypeInfoBase<int>("int", VT_INT) {};

			static PyObject* toPyObject(int val) {
				return PyLong_FromLong(val);
			}
			
			static bool fromPyObject(PyObject *src, int& dst) {
				if (PyInt_Check(src)) {
					dst = static_cast<int>(PyInt_AsLong(src));
					return true;
				} else {
					return false;
				}
			}
		};

		template<> struct TypeInfo<long> : TypeInfoBase<long> {
			TypeInfo() : TypeInfoBase<long>("long", VT_LONG) {};

			static PyObject* toPyObject(long val) {
				return PyLong_FromLong(val);
			}
			
			static bool fromPyObject(PyObject *src, long& dst) {
				if (PyLong_Check(src)) {
					dst = PyLong_AsLong(src);
					return true;
				} else {
					return false;
				}
			}
		};


		template<> struct TypeInfo<float> : TypeInfoBase<float> {
			TypeInfo() : TypeInfoBase<float>("float", VT_FLOAT) {};

			static PyObject* toPyObject(float val) {
				return PyFloat_FromDouble(val);
			}
			
			static bool fromPyObject(PyObject *src, float& dst) {
				if (PyFloat_Check(src)) {
					dst = PyFloat_AsDouble(src);
					return true;
				} else {
					return false;
				}
			}
		};

		template<> struct TypeInfo<std::string>  : TypeInfoBase<std::string> {
			TypeInfo() : TypeInfoBase<std::string>("std::string", VT_STRING) {};

			static PyObject* toPyObject(const std::string& val) {
				return PyString_FromString(val.c_str());
			}
			
			static bool fromPyObject(PyObject *src, std::string& dst) {
				if (PyString_Check(src)) {
					dst = PyString_AsString(src);
					return true; 
				} else {
					return false;
				}
			}
		};
		
		template<> struct TypeInfo<Vector3>  : TypeInfoBase<Vector3> {
			TypeInfo() : TypeInfoBase<Vector3>("Vector3", VT_CUSTOM_TYPE) {};

			static PyObject* toPyObject(const Vector3& val) {
				return Py_BuildValue("[fff]", val.x, val.y, val.z);
			}
			
			static bool fromPyObject(PyObject *src, Vector3& dst) {
				float x, y, z;
				if (PyArg_Parse(src, "[fff]", &x, &y, &z)) {
					dst.x = x;
					dst.y = y;
					dst.z = z;
					return true;
				} else {
					return false;
				}
			}
		};
		
		template<> struct TypeInfo<Quaternion>  : TypeInfoBase<Quaternion> {
			TypeInfo() : TypeInfoBase<Quaternion>("Quaternion", VT_CUSTOM_TYPE) {};

			static PyObject* toPyObject(const Quaternion& val) {
				return Py_BuildValue("[ffff]", val.x, val.y, val.z, val.w);
			}
			
			static bool fromPyObject(PyObject *src, Quaternion& dst) {
				float x, y, z, w;
				if (PyArg_Parse(src, "[ffff]", &x, &y, &z, &w)) {
					dst.x = x;
					dst.y = y;
					dst.z = z;
					dst.z = w;
					return true;
				} else {
					return false;
				}
			}
		};
		
		template<> struct TypeInfo<Ogre::ColourValue>  : TypeInfoBase<Ogre::ColourValue> {
			TypeInfo() : TypeInfoBase<Ogre::ColourValue>("Ogre::ColourValue", VT_CUSTOM_TYPE) {};

			static PyObject* toPyObject(const Ogre::ColourValue& val) {
				return Py_BuildValue("[ffff]", val.r, val.g, val.b, val.a);
			}
			
			static bool fromPyObject(PyObject *src, Ogre::ColourValue& dst) {
				float r, g, b, a;
				if (PyArg_Parse(src, "[ffff]", &r, &g, &b, &a)) {
					dst.r = r;
					dst.g = g;
					dst.b = b;
					dst.a = a;
					return true;
				} else {
					return false;
				}
			}
		};	

		// Global utilities - object conversion and such
		PyObject* DVariantToPyObject(const DVariant& inst);
		DVariant PyObjectToDVariant(PyObject* obj);

		// DVariant type info
		template<> struct TypeInfo<DVariant> : TypeInfoBase<DVariant> {
			TypeInfo() : TypeInfoBase<DVariant>("DVariant", VT_CUSTOM_TYPE) {};

			static PyObject* toPyObject(const DVariant& val) {
				return DVariantToPyObject(val);
			}
			
			static bool fromPyObject(PyObject *src, DVariant& dst) {
				dst = PyObjectToDVariant(src);
				return true;
			}
		};
		
		// One casting info slot. Contains Python type object an casting method pointer
		template<class C> struct CastInfo {
			PyTypeObject *type;
			/** Upcasting method. Should return a static_cast'ed instance if class C given the wrapping repr. of object
			 * Example:
			 * @code
			 * ParentClass *castChildToParent(PyObject* obj) {
			 * 	Object* mywrap = reinterpret_cast<ObjectBase<ChildClass*>>(obj)->mInstance;
			 * 	assert(mywrap->ob_type == msType); 
			 * 	return &mywrap->mInstance;
			 * }
			 * @endcode
			 * @see defaultPythonCaster for default impl.
			 * @todo It should be possible to templatize the casting method and reference it
			 */ 
			typedef C (*CastMethod)(PyObject* obj);
			CastMethod caster;
		};
		
		
		
		/// Template definition of a Python instance holding a single object
		template<typename T> struct ObjectBase {
			PyObject_HEAD
			T mInstance;
		};
		
		/// Upcaster for python side class inheritance resolution for non-smartptr objects 
		template<class P, class C> P defaultPythonUpcaster(PyObject *obj) {
			ObjectBase<C> *mywrap = reinterpret_cast< ObjectBase<C>* >(obj);
			return static_cast<P>(mywrap->mInstance);
		}

		/** helper function to get user data from Python's Object 
		 * @param obj The source object to extract
		 * @param typ The type object of our type
		 * @param target Pointer to Type instance. Filled with the pointer to the instance in the object
		 * @param castInfo The optional structure containing casting methods for conversion casts */
		template<typename T> bool python_cast(PyObject *obj, PyTypeObject *type, T *target, CastInfo<T> *castInfo = NULL) {
			// reinterpret cast is not a correct operation on subclasses
			// so we need the type info to agree
			
			// we can try searching the cast info for subtypes if those differ 
			PyTypeObject* ot = obj->ob_type;
			
			if (ot != type) {
				CastInfo<T> * current = castInfo;
				if (current != NULL) {
					while (current->type != NULL) {
						// do we have a match?
						if (ot == current->type)
							// yep. Cast using the upcaster
							*target = current->caster(obj);
							return true;
					}
				}
				
				return false;
			}
			
			*target = reinterpret_cast< ObjectBase<T>* >(obj)->mInstance;
			return true;
		}
		
		// Conversion error for simplicity
#define __PY_CONVERR_RET { PyErr_SetString(PyExc_TypeError, "Incompatible types error."); return NULL;	}
#define __PY_BADPARM_RET(parm) { PyErr_SetString(PyExc_TypeError, "Incompatible parameter type on '" #parm "'"); return NULL;	}
#define __PY_BADPARMS_RET { PyErr_SetString(PyExc_TypeError, "Incompatible parameters"); return NULL;	}
#define __PY_NONE_RET { Py_INCREF(Py_None); return Py_None; }		
		

		/// Common ancestor for all python published C types
		class PythonPublishedType {
			protected:
				/** Publishes the type as a member of a specified module
					@param containter The module to publish the type in
					@param type The python type object to publish
					@param name The name of the type to use
				*/
				static void publishType(PyObject* containter, PyTypeObject* type, const char* name);
		};

		/// A template that binds sharedptr typed classes
		template <typename T> class shared_ptr_binder : public PythonPublishedType {
			public:
				/// A python object type
				typedef ObjectBase<T> Object;
				
			protected:
				/// A sort-of constructor method. To be used to create a new NULL Object*
				static Object* construct(PyTypeObject* type) {
					Object* object;

					object = PyObject_New(Object, type);

					// At this point, the shared_ptr instance in the object is invalid (I.E. Allocated, but not initialized).
					// If we try to assign into it, we'll segfault. Because of that, we have to do placed new to initialize the object
					if (object != NULL) {
						// Here, tidy!
						::new (&object->mInstance) T();
					}

					return object;
				}

				/// Destructor for the python object. Safely decreases the reference to the shared_ptr. To be used in msType
				static void dealloc(PyObject *self) {
					// cast the object to T::Object
					Object* o = reinterpret_cast<Object*>(self);

					// Decreases the shared_ptr counter
					o->mInstance.setNull();

					// Call the destructor to clean up
					(&o->mInstance)->~T();

					// Finally delete the object
					PyObject_Del(self);
				}
		};

		/// A template that binds a pointer to class (simmilar to shared_ptr_binder, but no special handling is used)
		template <typename T> class class_ptr_binder : public PythonPublishedType {
			protected:
				static PyTypeObject msType;
				
			public:
				/// A python object type
				typedef ObjectBase<T*> Object;
			

			protected:
				/// A sort-of constructor method. To be used to create a new NULL Object*
				static Object* construct(PyTypeObject* type) {
					Object* object;

					object = PyObject_New(Object, type);

					if (object != NULL) {
						// Here, tidy!
						object->mInstance = NULL;
					}

					return object;
				}

				/// Destructor for the python object. Safely decreases the reference to the shared_ptr. To be used in msType
				static void dealloc(PyObject *self) {
					// Object* o = reinterpret_cast<Object*>(self);
					// delete the object
					PyObject_Del(self);
				}
		};

	};

    /** Central class for python bindings. Call PythonLanguage::Init() to prepare python environment */
    class OPDELIB_EXPORT PythonLanguage {
    	protected:
			static Opde::Root* msRoot;

    	public:
			/** Initializes python lang and all the bindings */
			static void init(int argc, char **argv);
			
			/** Initializes the Opde module itself, without the interpretter */
			static void initModule();
			
			/** Finalizes python lang */
			static void term();

			/** Runs a script loaded in memory on a given address */
			static void runScriptPtr(const char* ptr);

			/** Runs a script from a file */
			static bool runScript(const char* fname);

			/// Python side Root singleton handler
			static PyObject* createRoot(PyObject *self, PyObject* args);

			/// Python side Root singleton handler
			static PyObject* getRoot(PyObject *self, PyObject* args);
    };
}


#endif
