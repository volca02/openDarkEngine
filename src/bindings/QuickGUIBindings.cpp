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

#include "bindings.h"
#include "QuickGUIBindings.h"
#include "QuickGUIMemberFunctionPointer.h"
#include "QuickGUIEventArgs.h"
#include "OpdeException.h"

using namespace QuickGUI;

namespace Opde {
	namespace Python {
		// -------------------- QuickGUI event args conversion -------------
		
		PyObject* EventArgsToPyObject(const QuickGUI::EventArgs& ea) {
			// cast to the right type of event, then fill the directory according to the event's params
			
			// First, the common event args
			PyObject* base = PyDict_New();
				
			PyObject* boolconv = ea.handled ? Py_True : Py_False;
			Py_INCREF(boolconv);
			PyDict_SetItemString(base, "handled", boolconv);
			PyDict_SetItemString(base, "eventType", PyLong_FromLong(ea.eventType));
			PyDict_SetItemString(base, "type", PyLong_FromLong(ea.type)); // Long (should be dict item or constant or something)
			
			if (ea.type == EventArgs::TYPE_DEFAULT) {
				return base;
			} else if (ea.type == EventArgs::TYPE_TEXT) {
				const TextEventArgs& tea = static_cast<const TextEventArgs&>(ea);
				// Additional text attrs.
				boolconv = tea.colorChanged ? Py_True : Py_False; Py_INCREF(boolconv);
				PyDict_SetItemString(base, "colorChanged", boolconv);
				boolconv = tea.captionChanged ? Py_True : Py_False; Py_INCREF(boolconv);
				PyDict_SetItemString(base, "captionChanged", boolconv);
				boolconv = tea.fontChanged ? Py_True : Py_False; Py_INCREF(boolconv);
				PyDict_SetItemString(base, "fontChanged", boolconv);
				
				// TODO: Text* converted handle!
				// PyDict_SetItem(base, PyString_FromString("text"), PyString_FromString(tea.Text->c_str()));
			} else {
				
				// TODO: bind widget* into the event args
				PyDict_SetItemString(base, "widget", QG_WidgetBinder::create(((const WidgetEventArgs&)(ea)).widget));
				
				if (ea.type == EventArgs::TYPE_KEY) {
					// Additional key attributes
					const KeyEventArgs& kea = static_cast<const KeyEventArgs&>(ea);
					// Only codepoint and modifiers for now, the enum would be a nightmare to bind
					PyDict_SetItemString(base, "codepoint", PyUnicode_FromOrdinal(kea.codepoint));
					PyDict_SetItemString(base, "keyModifiers", PyLong_FromLong(kea.keyModifiers));
					
				} else if (ea.type == EventArgs::TYPE_MOUSE) {
					const MouseEventArgs& mea = static_cast<const MouseEventArgs&>(ea);
					// Mouse type event
					PyObject* pos = PyTuple_New(2);
					PyTuple_SET_ITEM(pos, 0, PyFloat_FromDouble(mea.position.x));
					PyTuple_SET_ITEM(pos, 1, PyFloat_FromDouble(mea.position.y));
					
					PyDict_SetItemString(base, "position", pos);
					
					pos = PyTuple_New(2);
					PyTuple_SET_ITEM(pos, 0, PyFloat_FromDouble(mea.moveDelta.x));
					PyTuple_SET_ITEM(pos, 1, PyFloat_FromDouble(mea.moveDelta.y));
					
					PyDict_SetItemString(base, "moveDelta", pos);
					
					
					PyDict_SetItemString(base, "button", PyLong_FromLong(mea.button));
					PyDict_SetItemString(base, "wheelChange", PyFloat_FromDouble(mea.wheelChange));
					PyDict_SetItemString(base, "keyModifiers", PyLong_FromLong(mea.keyModifiers));
					
				} else if (ea.type == EventArgs::TYPE_SCROLL) {
					// TODO:
				}
				
			}
			
			return base;
		}
		
		// -------------------- Python targetted quickGUI callback ---------
		class PythonFunctionSlot : public MemberFunctionSlot {
			public:
				PythonFunctionSlot(PyObject* callable) : mCallable(callable) {
				if (!PyCallable_Check(mCallable))  
					OPDE_EXCEPT("Python callback can't be constructed on non-callable!", "PythonFunctionSlot::PythonFunctionSlot");
				
				Py_INCREF(mCallable); 
			};
			
			~PythonFunctionSlot() { 
				Py_DECREF(mCallable);
			};
			
			virtual void execute(const QuickGUI::EventArgs& args) {
				// call the object
				PyObject* py_msg = EventArgsToPyObject(args);
				
				// Call the pyobject
				PyObject* pyarg; 
				pyarg = PyTuple_New(1);
				
				PyTuple_SetItem(pyarg, 0, py_msg);
				
				PyObject* rslt = PyObject_CallObject(mCallable, pyarg);
				
				if (rslt) { // To be sure no leak happened
					Py_XDECREF(rslt);
				}
				
				Py_XDECREF(py_msg);
				Py_XDECREF(pyarg);
			}
			
			protected:
				PyObject* mCallable;
		};
		
		// -------------------- QuickGUI Manager Binder --------------------
		char* QG_ManagerBinder::msName = "Manager";

		// ------------------------------------------
		PyTypeObject QG_ManagerBinder::msType = {
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			msName,                   /* char *tp_name; */
			sizeof(QG_ManagerBinder::Object),      /* int tp_basicsize; */
			0,                        /* int tp_itemsize;       /* not used much */
			QG_ManagerBinder::dealloc,   /* destructor tp_dealloc; */
			0,			              /* printfunc  tp_print;   */
			QG_ManagerBinder::getattr,  /* getattrfunc  tp_getattr; /* __getattr__ */
			0,   					  /* setattrfunc  tp_setattr;  /* __setattr__ */
			0,				          /* cmpfunc  tp_compare;  /* __cmp__ */
			repr,		              /* reprfunc  tp_repr;    /* __repr__ */
			0,				          /* PyNumberMethods *tp_as_number; */
			0,                        /* PySequenceMethods *tp_as_sequence; */
			0,                        /* PyMappingMethods *tp_as_mapping; */
			0,			              /* hashfunc tp_hash;     /* __hash__ */
			0,                        /* ternaryfunc tp_call;  /* __call__ */
			0,			              /* reprfunc tp_str;      /* __str__ */
			0,			              /* getattrofunc tp_getattro; */
			0,			              /* setattrofunc tp_setattro; */
			0,			              /* PyBufferProcs *tp_as_buffer; */
			0,					      /* long tp_flags; */
			0,			              /* char *tp_doc;  */
			0,			              /* traverseproc tp_traverse; */
			0,			              /* inquiry tp_clear; */
			0,			              /* richcmpfunc tp_richcompare; */
			0,			              /* long tp_weaklistoffset; */
			0,			              /* getiterfunc tp_iter; */
			0,			              /* iternextfunc tp_iternext; */
			msMethods,	              /* struct PyMethodDef *tp_methods; */
			0,			              /* struct memberlist *tp_members; */
			0,			              /* struct getsetlist *tp_getset; */
		};

		// ------------------------------------------
		PyMethodDef QG_ManagerBinder::msMethods[] = {

			{NULL, NULL},
		};
		
		
		// -------------------- QuickGUI Widget Binder --------------------
		char* QG_WidgetBinder::msName = "Widget";

		// ------------------------------------------
		PyTypeObject QG_WidgetBinder::msType = {
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			msName,                   /* char *tp_name; */
			sizeof(QG_WidgetBinder::Object),      /* int tp_basicsize; */
			0,                        /* int tp_itemsize;       /* not used much */
			QG_WidgetBinder::dealloc,   /* destructor tp_dealloc; */
			0,			              /* printfunc  tp_print;   */
			QG_WidgetBinder::getattr,  /* getattrfunc  tp_getattr; /* __getattr__ */
			0,   					  /* setattrfunc  tp_setattr;  /* __setattr__ */
			0,				          /* cmpfunc  tp_compare;  /* __cmp__ */
			repr,		              /* reprfunc  tp_repr;    /* __repr__ */
			0,				          /* PyNumberMethods *tp_as_number; */
			0,                        /* PySequenceMethods *tp_as_sequence; */
			0,                        /* PyMappingMethods *tp_as_mapping; */
			0,			              /* hashfunc tp_hash;     /* __hash__ */
			0,                        /* ternaryfunc tp_call;  /* __call__ */
			0,			              /* reprfunc tp_str;      /* __str__ */
			0,			              /* getattrofunc tp_getattro; */
			0,			              /* setattrofunc tp_setattro; */
			0,			              /* PyBufferProcs *tp_as_buffer; */
			0,					      /* long tp_flags; */
			0,			              /* char *tp_doc;  */
			0,			              /* traverseproc tp_traverse; */
			0,			              /* inquiry tp_clear; */
			0,			              /* richcmpfunc tp_richcompare; */
			0,			              /* long tp_weaklistoffset; */
			0,			              /* getiterfunc tp_iter; */
			0,			              /* iternextfunc tp_iternext; */
			msMethods,	              /* struct PyMethodDef *tp_methods; */
			0,			              /* struct memberlist *tp_members; */
			0,			              /* struct getsetlist *tp_getset; */
		};

		// ------------------------------------------
		PyMethodDef QG_WidgetBinder::msMethods[] = {
			{"addChild", addChild, METH_VARARGS},
			{NULL, NULL},
		};
		
		
		// ------------------------ IMPLEMENTATION OF THE METHODS -------------------------------
		// ------------------------------------------
		PyObject* QG_ManagerBinder::getattr(PyObject *self, char *name) {
			return Py_FindMethod(msMethods, self, name);
		}
		
		// ------------------------------------------
		PyObject* QG_ManagerBinder::repr(PyObject *self) {
			return PyString_FromFormat("<QuickGUI::Manager at %p>", self);
		}

		// ------------------------------------------
		PyObject* QG_ManagerBinder::create(QuickGUI::GUIManager* mgr) {
			Object* object = construct(&msType);

			if (object != NULL) {
				object->mInstance = mgr;
			}

			return (PyObject *)object;
		}
		
		// ------------------------------------------
		PyObject* QG_WidgetBinder::addChild(PyObject *self, PyObject *args) {
			Object* o = python_cast<Object*>(self, &msType);
			
			PyObject* child;

			if (PyArg_ParseTuple(args, "O", &child)) {
				// Check if the type is Widget as well
				Object* cho = python_cast<Object*>(child, &msType);
				
				// It is. We can insert it as a child
				o->mInstance->addChild(cho->mInstance);

				PyObject* result = Py_None;
				Py_INCREF(result);
				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a Widget object as argument!");
				return NULL;
			}
		}
		
		// ------------------------------------------
		PyObject* QG_WidgetBinder::getattr(PyObject *self, char *name) {
			return Py_FindMethod(msMethods, self, name);
		}
		
		// ------------------------------------------
		PyObject* QG_WidgetBinder::repr(PyObject *self) {
			return PyString_FromFormat("<QuickGUI::Widget at %p>", self);
		}

		// ------------------------------------------
		PyObject* QG_WidgetBinder::create(QuickGUI::Widget* wgt) {
			Object* object = construct(&msType);

			if (object != NULL) {
				object->mInstance = wgt;
			}

			return (PyObject *)object;
		}
	}

} // namespace Opde

