/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2009 openDarkEngine team
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

#ifndef __DRAWSERVICEBINDER_H
#define __DRAWSERVICEBINDER_H

#include "BinaryService.h"
#include "DrawService.h"

namespace Opde {

	namespace Python {

		/// Draw service python binder
		class DrawServiceBinder : public shared_ptr_binder<DrawServicePtr> {
			public:
				static void init(PyObject* module);

				// --- Python type related methods ---
				static PyObject* getattr(PyObject *self, char *name);
				
				/// to string - reprfunc conversion
				static PyObject* repr(PyObject *self);
				
				// --- Methods ---
				// Sheet				
				static PyObject* createSheet(PyObject* self, PyObject* args);
				static PyObject* destroySheet(PyObject* self, PyObject* args);
				static PyObject* getSheet(PyObject* self, PyObject* args);
				static PyObject* setActiveSheet(PyObject* self, PyObject* args);
				
				// various draw sources and rendered objects
				static PyObject* createDrawSource(PyObject* self, PyObject* args);
				static PyObject* createRenderedImage(PyObject* self, PyObject* args);
				static PyObject* createRenderedLabel(PyObject* self, PyObject* args);
				static PyObject* destroyDrawOperation(PyObject* self, PyObject* args);
				
				static PyObject* createAtlas(PyObject* self, PyObject* args);
				static PyObject* destroyAtlas(PyObject* self, PyObject* args);
				
				static PyObject* loadFont(PyObject* self, PyObject* args);
				static PyObject* setFontPalette(PyObject* self, PyObject* args);
				
				static PyObject* create();

			protected:
				/// Static type definition
				static PyTypeObject msType;

				/// Name of the python type
				static const char* msName;

				/// Method list
				static PyMethodDef msMethods[];
		};
		

		// ClipRect type info
		template<> struct TypeInfo<ClipRect> : TypeInfoBase<ClipRect> {
			TypeInfo() : TypeInfoBase<ClipRect>("ClipRect", VT_CUSTOM_TYPE) {};

			static PyObject* toPyObject(const ClipRect& cr) {
				// tuple it is
				PyObject *crp = PyTuple_New(4);
				PyTuple_SetItem(crp, 0, PyFloat_FromDouble(cr.left)); // Steals reference
				PyTuple_SetItem(crp, 1, PyFloat_FromDouble(cr.right)); // Steals reference
				PyTuple_SetItem(crp, 2, PyFloat_FromDouble(cr.top)); // Steals reference
				PyTuple_SetItem(crp, 3, PyFloat_FromDouble(cr.bottom)); // Steals reference
				
				return crp;
			}
			
			static bool fromPyObject(PyObject* o, ClipRect& cr) {
				if (!PyTuple_Check(o))
					return false;
					
				if (PyTuple_GET_SIZE(o) != 4)
					return false;
				
				// Type check all those before setting them
				PyObject *pl, *pr, *pt, *pb;
				pl = PyTuple_GetItem(o, 0);
				pr = PyTuple_GetItem(o, 1);
				pt = PyTuple_GetItem(o, 2);
				pb = PyTuple_GetItem(o, 3);
				
				if (!PyFloat_Check(pl) ||
					!PyFloat_Check(pr) ||
					!PyFloat_Check(pt) ||
					!PyFloat_Check(pb) )
					return false;
				
				cr.left   = PyFloat_AsDouble(pl);
				cr.right  = PyFloat_AsDouble(pr);
				cr.top    = PyFloat_AsDouble(pt);
				cr.bottom = PyFloat_AsDouble(pb);
				
				return true;
			}
		};
		
		//
		template<> struct TypeInfo<PixelCoord> : TypeInfoBase<PixelCoord> {
			TypeInfo() : TypeInfoBase<PixelCoord>("PixelCoord", VT_CUSTOM_TYPE) {};

			static PyObject* toPyObject(const PixelCoord& pc) {
				// tuple it is
				PyObject *cp = PyTuple_New(2);
				PyTuple_SetItem(cp, 0, PyLong_FromLong(pc.first)); // Steals reference
				PyTuple_SetItem(cp, 1, PyLong_FromLong(pc.second)); // Steals reference
				
				return cp;
			}
			
			static bool fromPyObject(PyObject* o, PixelCoord& pc) {
				if (!PyTuple_Check(o))
					return false;
					
				if (PyTuple_GET_SIZE(o) != 2)
					return false;
				
				PyObject *arg = PyTuple_GetItem(o, 0);
				
				if ((!PyLong_Check(arg)) && (!PyInt_Check(arg)))
					return false;
				
				PyObject *arg1 = PyTuple_GetItem(o, 1);
				
				if ((!PyLong_Check(arg1)) && (!PyInt_Check(arg1)))
					return false;
				
				pc.first  = PyLong_AsLong(arg);
				pc.second = PyLong_AsLong(arg1);
								
				return true;
			}
		};
		
		
                
        /// forward declaration of the child classes
        class RenderedImageBinder;
        class RenderedLabelBinder;
        
		/// DrawOperation base class binder
		class DrawOperationBinder : public class_ptr_binder<DrawOperation> {
			// For inheritance - msType access
			friend class RenderedImageBinder;
			friend class RenderedLabelBinder;
			
			public:
				static void init(PyObject* module);

				// --- Python type related methods ---
				static PyObject* getattr(PyObject *self, char *name);
				
				/// to string - reprfunc conversion
				static PyObject* repr(PyObject *self);
				
				/// helper class pointer extractor
				static bool extract(PyObject *object, DrawOperation*& op);
				
				// --- Methods ---
				static PyObject* setPosition(PyObject *self, PyObject *args);
				static PyObject* setZOrder(PyObject *self, PyObject *args);
				static PyObject* setClipRect(PyObject *self, PyObject *args);

				// --- Properties ---
				// -- setters --
				static int setPPosition(PyObject *self, PyObject *value, void *closure);
				static int setPZOrder(PyObject *self, PyObject *value, void *closure);
				static int setPClipRect(PyObject *self, PyObject *value, void *closure);
				
				// -- getters --				
				static PyObject* getPPosition(PyObject *self, void *closure);
				static PyObject* getPZOrder(PyObject *self, void *closure);
				static PyObject* getPClipRect(PyObject *self, void *closure);
				
				
				static PyObject* create(DrawOperation* ds);

			protected:
				/// Static type definition
				static PyTypeObject msType;

				/// Name of the python type
				static const char* msName;

				/// Method list
				static PyMethodDef msMethods[];
				
				/// Getters and setters for field access
				static PyGetSetDef msGetSet[];
				
				/// Up-casting helper structures
				static CastInfo<DrawOperation*> msCastInfo[];
		};
		
	}
}

#endif
