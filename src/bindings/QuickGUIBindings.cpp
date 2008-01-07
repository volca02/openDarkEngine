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
		PyMethodDef QG_NoMethods[] = {
			{NULL, NULL},
		};

		void initQGUIKeyCodes(PyObject* module) {
		    PyModule_AddIntConstant(module, "KC_ESCAPE", KC_ESCAPE);
            PyModule_AddIntConstant(module, "KC_1", KC_1);
            PyModule_AddIntConstant(module, "KC_2", KC_2);
            PyModule_AddIntConstant(module, "KC_3", KC_3);
            PyModule_AddIntConstant(module, "KC_4", KC_4);
            PyModule_AddIntConstant(module, "KC_5", KC_5);
            PyModule_AddIntConstant(module, "KC_6", KC_6);
            PyModule_AddIntConstant(module, "KC_7", KC_7);
            PyModule_AddIntConstant(module, "KC_8", KC_8);
            PyModule_AddIntConstant(module, "KC_9", KC_9);
            PyModule_AddIntConstant(module, "KC_0", KC_0);
            PyModule_AddIntConstant(module, "KC_MINUS", KC_MINUS);
            PyModule_AddIntConstant(module, "KC_EQUALS", KC_EQUALS);
            PyModule_AddIntConstant(module, "KC_BACK", KC_BACK);
            PyModule_AddIntConstant(module, "KC_TAB", KC_TAB);
            PyModule_AddIntConstant(module, "KC_Q", KC_Q);
            PyModule_AddIntConstant(module, "KC_W", KC_W);
            PyModule_AddIntConstant(module, "KC_E", KC_E);
            PyModule_AddIntConstant(module, "KC_R", KC_R);
            PyModule_AddIntConstant(module, "KC_T", KC_T);
            PyModule_AddIntConstant(module, "KC_Y", KC_Y);
            PyModule_AddIntConstant(module, "KC_U", KC_U);
            PyModule_AddIntConstant(module, "KC_I", KC_I);
            PyModule_AddIntConstant(module, "KC_O", KC_O);
            PyModule_AddIntConstant(module, "KC_P", KC_P);
            PyModule_AddIntConstant(module, "KC_LBRACKET", KC_LBRACKET);
            PyModule_AddIntConstant(module, "KC_RBRACKET", KC_RBRACKET);
            PyModule_AddIntConstant(module, "KC_RETURN", KC_RETURN);
            PyModule_AddIntConstant(module, "KC_LCONTROL", KC_LCONTROL);
            PyModule_AddIntConstant(module, "KC_A", KC_A);
            PyModule_AddIntConstant(module, "KC_S", KC_S);
            PyModule_AddIntConstant(module, "KC_D", KC_D);
            PyModule_AddIntConstant(module, "KC_F", KC_F);
            PyModule_AddIntConstant(module, "KC_G", KC_G);
            PyModule_AddIntConstant(module, "KC_H", KC_H);
            PyModule_AddIntConstant(module, "KC_J", KC_J);
            PyModule_AddIntConstant(module, "KC_K", KC_K);
            PyModule_AddIntConstant(module, "KC_L", KC_L);
            PyModule_AddIntConstant(module, "KC_SEMICOLON", KC_SEMICOLON);
            PyModule_AddIntConstant(module, "KC_APOSTROPHE", KC_APOSTROPHE);
            PyModule_AddIntConstant(module, "KC_GRAVE", KC_GRAVE);
            PyModule_AddIntConstant(module, "KC_LSHIFT", KC_LSHIFT);
            PyModule_AddIntConstant(module, "KC_BACKSLASH", KC_BACKSLASH);
            PyModule_AddIntConstant(module, "KC_Z", KC_Z);
            PyModule_AddIntConstant(module, "KC_X", KC_X);
            PyModule_AddIntConstant(module, "KC_C", KC_C);
            PyModule_AddIntConstant(module, "KC_V", KC_V);
            PyModule_AddIntConstant(module, "KC_B", KC_B);
            PyModule_AddIntConstant(module, "KC_N", KC_N);
            PyModule_AddIntConstant(module, "KC_M", KC_M);
            PyModule_AddIntConstant(module, "KC_COMMA", KC_COMMA);
            PyModule_AddIntConstant(module, "KC_PERIOD", KC_PERIOD);
            PyModule_AddIntConstant(module, "KC_SLASH", KC_SLASH);
            PyModule_AddIntConstant(module, "KC_RSHIFT", KC_RSHIFT);
            PyModule_AddIntConstant(module, "KC_MULTIPLY", KC_MULTIPLY);
            PyModule_AddIntConstant(module, "KC_LMENU", KC_LMENU);
            PyModule_AddIntConstant(module, "KC_SPACE", KC_SPACE);
            PyModule_AddIntConstant(module, "KC_CAPITAL", KC_CAPITAL);
            PyModule_AddIntConstant(module, "KC_F1", KC_F1);
            PyModule_AddIntConstant(module, "KC_F2", KC_F2);
            PyModule_AddIntConstant(module, "KC_F3", KC_F3);
            PyModule_AddIntConstant(module, "KC_F4", KC_F4);
            PyModule_AddIntConstant(module, "KC_F5", KC_F5);
            PyModule_AddIntConstant(module, "KC_F6", KC_F6);
            PyModule_AddIntConstant(module, "KC_F7", KC_F7);
            PyModule_AddIntConstant(module, "KC_F8", KC_F8);
            PyModule_AddIntConstant(module, "KC_F9", KC_F9);
            PyModule_AddIntConstant(module, "KC_F10", KC_F10);
            PyModule_AddIntConstant(module, "KC_NUMLOCK", KC_NUMLOCK);
            PyModule_AddIntConstant(module, "KC_SCROLL", KC_SCROLL);
            PyModule_AddIntConstant(module, "KC_NUMPAD7", KC_NUMPAD7);
            PyModule_AddIntConstant(module, "KC_NUMPAD8", KC_NUMPAD8);
            PyModule_AddIntConstant(module, "KC_NUMPAD9", KC_NUMPAD9);
            PyModule_AddIntConstant(module, "KC_SUBTRACT", KC_SUBTRACT);
            PyModule_AddIntConstant(module, "KC_NUMPAD4", KC_NUMPAD4);
            PyModule_AddIntConstant(module, "KC_NUMPAD5", KC_NUMPAD5);
            PyModule_AddIntConstant(module, "KC_NUMPAD6", KC_NUMPAD6);
            PyModule_AddIntConstant(module, "KC_ADD", KC_ADD);
            PyModule_AddIntConstant(module, "KC_NUMPAD1", KC_NUMPAD1);
            PyModule_AddIntConstant(module, "KC_NUMPAD2", KC_NUMPAD2);
            PyModule_AddIntConstant(module, "KC_NUMPAD3", KC_NUMPAD3);
            PyModule_AddIntConstant(module, "KC_NUMPAD0", KC_NUMPAD0);
            PyModule_AddIntConstant(module, "KC_DECIMAL", KC_DECIMAL);
            PyModule_AddIntConstant(module, "KC_OEM_102", KC_OEM_102);
            PyModule_AddIntConstant(module, "KC_F11", KC_F11);
            PyModule_AddIntConstant(module, "KC_F12", KC_F12);
            PyModule_AddIntConstant(module, "KC_F13", KC_F13);
            PyModule_AddIntConstant(module, "KC_F14", KC_F14);
            PyModule_AddIntConstant(module, "KC_F15", KC_F15);
            PyModule_AddIntConstant(module, "KC_KANA", KC_KANA);
            PyModule_AddIntConstant(module, "KC_ABNT_C1", KC_ABNT_C1);
            PyModule_AddIntConstant(module, "KC_CONVERT", KC_CONVERT);
            PyModule_AddIntConstant(module, "KC_NOCONVERT", KC_NOCONVERT);
            PyModule_AddIntConstant(module, "KC_YEN", KC_YEN);
            PyModule_AddIntConstant(module, "KC_ABNT_C2", KC_ABNT_C2);
            PyModule_AddIntConstant(module, "KC_NUMPADEQUALS", KC_NUMPADEQUALS);
            PyModule_AddIntConstant(module, "KC_PREVTRACK", KC_PREVTRACK);
            PyModule_AddIntConstant(module, "KC_AT", KC_AT);
            PyModule_AddIntConstant(module, "KC_COLON", KC_COLON);
            PyModule_AddIntConstant(module, "KC_UNDERLINE", KC_UNDERLINE);
            PyModule_AddIntConstant(module, "KC_KANJI", KC_KANJI);
            PyModule_AddIntConstant(module, "KC_STOP", KC_STOP);
            PyModule_AddIntConstant(module, "KC_AX", KC_AX);
            PyModule_AddIntConstant(module, "KC_UNLABELED", KC_UNLABELED);
            PyModule_AddIntConstant(module, "KC_NEXTTRACK", KC_NEXTTRACK);
            PyModule_AddIntConstant(module, "KC_NUMPADENTER", KC_NUMPADENTER);
            PyModule_AddIntConstant(module, "KC_RCONTROL", KC_RCONTROL);
            PyModule_AddIntConstant(module, "KC_MUTE", KC_MUTE);
            PyModule_AddIntConstant(module, "KC_CALCULATOR", KC_CALCULATOR);
            PyModule_AddIntConstant(module, "KC_PLAYPAUSE", KC_PLAYPAUSE);
            PyModule_AddIntConstant(module, "KC_MEDIASTOP", KC_MEDIASTOP);
            PyModule_AddIntConstant(module, "KC_VOLUMEDOWN", KC_VOLUMEDOWN);
            PyModule_AddIntConstant(module, "KC_VOLUMEUP", KC_VOLUMEUP);
            PyModule_AddIntConstant(module, "KC_WEBHOME", KC_WEBHOME);
            PyModule_AddIntConstant(module, "KC_NUMPADCOMMA", KC_NUMPADCOMMA);
            PyModule_AddIntConstant(module, "KC_DIVIDE", KC_DIVIDE);
            PyModule_AddIntConstant(module, "KC_SYSRQ", KC_SYSRQ);
            PyModule_AddIntConstant(module, "KC_RMENU", KC_RMENU);
            PyModule_AddIntConstant(module, "KC_PAUSE", KC_PAUSE);
            PyModule_AddIntConstant(module, "KC_HOME", KC_HOME);
            PyModule_AddIntConstant(module, "KC_UP", KC_UP);
            PyModule_AddIntConstant(module, "KC_PGUP", KC_PGUP);
            PyModule_AddIntConstant(module, "KC_LEFT", KC_LEFT);
            PyModule_AddIntConstant(module, "KC_RIGHT", KC_RIGHT);
            PyModule_AddIntConstant(module, "KC_END", KC_END);
            PyModule_AddIntConstant(module, "KC_DOWN", KC_DOWN);
            PyModule_AddIntConstant(module, "KC_PGDOWN", KC_PGDOWN);
            PyModule_AddIntConstant(module, "KC_INSERT", KC_INSERT);
            PyModule_AddIntConstant(module, "KC_DELETE", KC_DELETE);
            PyModule_AddIntConstant(module, "KC_LWIN", KC_LWIN);
            PyModule_AddIntConstant(module, "KC_RWIN", KC_RWIN);
            PyModule_AddIntConstant(module, "KC_APPS", KC_APPS);
            PyModule_AddIntConstant(module, "KC_POWER", KC_POWER);
            PyModule_AddIntConstant(module, "KC_SLEEP", KC_SLEEP);
            PyModule_AddIntConstant(module, "KC_WAKE", KC_WAKE);
            PyModule_AddIntConstant(module, "KC_WEBSEARCH", KC_WEBSEARCH);
            PyModule_AddIntConstant(module, "KC_WEBREFRESH", KC_WEBREFRESH);
            PyModule_AddIntConstant(module, "KC_WEBSTOP", KC_WEBSTOP);
            PyModule_AddIntConstant(module, "KC_WEBFORWARD", KC_WEBFORWARD);
            PyModule_AddIntConstant(module, "KC_WEBBACK", KC_WEBBACK);
            PyModule_AddIntConstant(module, "KC_MYCOMPUTER", KC_MYCOMPUTER);
            PyModule_AddIntConstant(module, "KC_MAIL", KC_MAIL);
            PyModule_AddIntConstant(module, "KC_MEDIASELECT", KC_MEDIASELECT);
		}

		PyObject* initQGUIModule(PyObject* container) {
		    // We'll reside in Opde.QuickGUI
		    PyObject *module = Py_InitModule("Opde.QuickGUI",
                                        QG_NoMethods);

		    assert(module);


		    PyObject *dir = PyModule_GetDict(container);
                    PyDict_SetItemString(dir, "QuickGUI", module);

		    // Submodule EventArgs
		    PyObject* eamod = Py_InitModule("Opde.QuickGUI.EventArgs", QG_NoMethods);

		    PyModule_AddIntConstant(eamod, "TYPE_DEFAULT", EventArgs::TYPE_DEFAULT);
		    PyModule_AddIntConstant(eamod, "TYPE_WIDGET", EventArgs::TYPE_WIDGET);
		    PyModule_AddIntConstant(eamod, "TYPE_MOUSE", EventArgs::TYPE_MOUSE);
		    PyModule_AddIntConstant(eamod, "TYPE_KEY", EventArgs::TYPE_KEY);
		    PyModule_AddIntConstant(eamod, "TYPE_SCROLL", EventArgs::TYPE_SCROLL);
		    PyModule_AddIntConstant(eamod, "TYPE_TEXT", EventArgs::TYPE_TEXT);

		    dir = PyModule_GetDict(module);
		    PyDict_SetItemString(dir, "EventArgs", eamod);

            // Widget enum constants
            PyObject* wgmod = Py_InitModule("Opde.QuickGUI.Widget", QG_NoMethods);

            // Widget Types:
            PyModule_AddIntConstant(wgmod, "TYPE_BORDER", Widget::TYPE_BORDER);
            PyModule_AddIntConstant(wgmod, "TYPE_BUTTON", Widget::TYPE_BUTTON);
            PyModule_AddIntConstant(wgmod, "TYPE_CHECKBOX", Widget::TYPE_CHECKBOX);
            PyModule_AddIntConstant(wgmod, "TYPE_COMBOBOX", Widget::TYPE_COMBOBOX);
            PyModule_AddIntConstant(wgmod, "TYPE_CONSOLE", Widget::TYPE_CONSOLE);
            PyModule_AddIntConstant(wgmod, "TYPE_IMAGE", Widget::TYPE_IMAGE);
            PyModule_AddIntConstant(wgmod, "TYPE_LABEL", Widget::TYPE_LABEL);
            PyModule_AddIntConstant(wgmod, "TYPE_LIST", Widget::TYPE_LIST);
            PyModule_AddIntConstant(wgmod, "TYPE_MENULABEL", Widget::TYPE_MENULABEL);
            PyModule_AddIntConstant(wgmod, "TYPE_LABELAREA", Widget::TYPE_LABELAREA);
            PyModule_AddIntConstant(wgmod, "TYPE_TEXTAREA", Widget::TYPE_TEXTAREA);
            PyModule_AddIntConstant(wgmod, "TYPE_NSTATEBUTTON", Widget::TYPE_NSTATEBUTTON);
            PyModule_AddIntConstant(wgmod, "TYPE_PANEL", Widget::TYPE_PANEL);
            PyModule_AddIntConstant(wgmod, "TYPE_PROGRESSBAR", Widget::TYPE_PROGRESSBAR);
            PyModule_AddIntConstant(wgmod, "TYPE_SCROLL_PANE", Widget::TYPE_SCROLL_PANE);
            PyModule_AddIntConstant(wgmod, "TYPE_SCROLLBAR_HORIZONTAL", Widget:: TYPE_SCROLLBAR_HORIZONTAL);
            PyModule_AddIntConstant(wgmod, "TYPE_SCROLLBAR_VERTICAL", Widget::TYPE_SCROLLBAR_VERTICAL);
            PyModule_AddIntConstant(wgmod, "TYPE_SHEET", Widget::TYPE_SHEET);
            PyModule_AddIntConstant(wgmod, "TYPE_TEXTBOX", Widget::TYPE_TEXTBOX);
            PyModule_AddIntConstant(wgmod, "TYPE_TITLEBAR", Widget::TYPE_TITLEBAR);
            PyModule_AddIntConstant(wgmod, "TYPE_TRACKBAR_HORIZONTAL", Widget::TYPE_TRACKBAR_HORIZONTAL);
            PyModule_AddIntConstant(wgmod, "TYPE_TRACKBAR_VERTICAL", Widget::TYPE_TRACKBAR_VERTICAL);
            PyModule_AddIntConstant(wgmod, "TYPE_TREE", Widget::TYPE_TREE);
            PyModule_AddIntConstant(wgmod, "TYPE_WINDOW", Widget::TYPE_WINDOW);

            // Constants for callback:
            PyModule_AddIntConstant(wgmod, "EVENT_CHARACTER_KEY", Widget::EVENT_CHARACTER_KEY);
            PyModule_AddIntConstant(wgmod, "EVENT_CHILD_ADDED", Widget::EVENT_CHILD_ADDED);
            PyModule_AddIntConstant(wgmod, "EVENT_CHILD_CREATED", Widget::EVENT_CHILD_CREATED);
            PyModule_AddIntConstant(wgmod, "EVENT_CHILD_DESTROYED", Widget::EVENT_CHILD_DESTROYED);
            PyModule_AddIntConstant(wgmod, "EVENT_CHILD_REMOVED", Widget::EVENT_CHILD_REMOVED);
            PyModule_AddIntConstant(wgmod, "EVENT_DISABLED", Widget::EVENT_DISABLED);
            PyModule_AddIntConstant(wgmod, "EVENT_DRAGGED", Widget::EVENT_DRAGGED);
            PyModule_AddIntConstant(wgmod, "EVENT_DROPPED", Widget::EVENT_DROPPED);
            PyModule_AddIntConstant(wgmod, "EVENT_ENABLED", Widget::EVENT_ENABLED);
            PyModule_AddIntConstant(wgmod, "EVENT_GAIN_FOCUS", Widget::EVENT_GAIN_FOCUS);
            PyModule_AddIntConstant(wgmod, "EVENT_HIDDEN", Widget::EVENT_HIDDEN);
            PyModule_AddIntConstant(wgmod, "EVENT_KEY_DOWN", Widget::EVENT_KEY_DOWN);
            PyModule_AddIntConstant(wgmod, "EVENT_KEY_UP", Widget::EVENT_KEY_UP);
            PyModule_AddIntConstant(wgmod, "EVENT_LOSE_FOCUS", Widget::EVENT_LOSE_FOCUS);
            PyModule_AddIntConstant(wgmod, "EVENT_MOUSE_BUTTON_DOWN", Widget::EVENT_MOUSE_BUTTON_DOWN);
            PyModule_AddIntConstant(wgmod, "EVENT_MOUSE_BUTTON_UP", Widget::EVENT_MOUSE_BUTTON_UP);
            PyModule_AddIntConstant(wgmod, "EVENT_MOUSE_CLICK", Widget::EVENT_MOUSE_CLICK);
            PyModule_AddIntConstant(wgmod, "EVENT_MOUSE_CLICK_DOUBLE", Widget::EVENT_MOUSE_CLICK_DOUBLE);
            PyModule_AddIntConstant(wgmod, "EVENT_MOUSE_CLICK_TRIPLE", Widget::EVENT_MOUSE_CLICK_TRIPLE);
            PyModule_AddIntConstant(wgmod, "EVENT_MOUSE_ENTER", Widget::EVENT_MOUSE_ENTER);
            PyModule_AddIntConstant(wgmod, "EVENT_MOUSE_LEAVE", Widget::EVENT_MOUSE_LEAVE);
            PyModule_AddIntConstant(wgmod, "EVENT_MOUSE_MOVE", Widget::EVENT_MOUSE_MOVE);
            PyModule_AddIntConstant(wgmod, "EVENT_MOUSE_WHEEL", Widget::EVENT_MOUSE_WHEEL);
            PyModule_AddIntConstant(wgmod, "EVENT_PARENT_CHANGED", Widget::EVENT_PARENT_CHANGED);
            PyModule_AddIntConstant(wgmod, "EVENT_POSITION_CHANGED", Widget::EVENT_POSITION_CHANGED);
            PyModule_AddIntConstant(wgmod, "EVENT_SHOWN", Widget::EVENT_SHOWN);
            PyModule_AddIntConstant(wgmod, "EVENT_SIZE_CHANGED", Widget::EVENT_SIZE_CHANGED);
            PyModule_AddIntConstant(wgmod, "EVENT_END_OF_LIST", Widget::EVENT_END_OF_LIST);

            PyModule_AddIntConstant(wgmod, "ANCHOR_HORIZONTAL_LEFT", Widget::ANCHOR_HORIZONTAL_LEFT);
            PyModule_AddIntConstant(wgmod, "ANCHOR_HORIZONTAL_RIGHT", Widget::ANCHOR_HORIZONTAL_RIGHT);
            PyModule_AddIntConstant(wgmod, "ANCHOR_HORIZONTAL_LEFT_RIGHT", Widget::ANCHOR_HORIZONTAL_LEFT_RIGHT);
            PyModule_AddIntConstant(wgmod, "ANCHOR_HORIZONTAL_NONE", Widget::ANCHOR_HORIZONTAL_NONE);

            PyModule_AddIntConstant(wgmod, "ANCHOR_VERTICAL_TOP", Widget::ANCHOR_VERTICAL_TOP);
            PyModule_AddIntConstant(wgmod, "ANCHOR_VERTICAL_BOTTOM", Widget::ANCHOR_VERTICAL_BOTTOM);
            PyModule_AddIntConstant(wgmod, "ANCHOR_VERTICAL_TOP_BOTTOM", Widget::ANCHOR_VERTICAL_TOP_BOTTOM);
            PyModule_AddIntConstant(wgmod, "ANCHOR_VERTICAL_NONE", Widget::ANCHOR_VERTICAL_NONE);

		    PyDict_SetItemString(dir, "Widget", eamod);

		    PyModule_AddIntConstant(module, "SHIFT", QuickGUI::SHIFT);
		    PyModule_AddIntConstant(module, "CTRL", QuickGUI::CTRL);
		    PyModule_AddIntConstant(module, "ALT", QuickGUI::ALT);

		    // Bind all the keycodes as well...
		    initQGUIKeyCodes(module);

		    return module;
		}

		PyObject* EventArgsToPyObject(const QuickGUI::EventArgs& ea) {
			// cast to the right type of event, then fill the directory according to the event's params

			// First, the common event args
			PyObject* base = PyDict_New();

			PyObject* boolconv = ea.handled ? Py_True : Py_False;
			Py_INCREF(boolconv);
			PyDict_SetItemString(base, "handled", boolconv);
			PyDict_SetItemString(base, "eventType", PyLong_FromLong(ea.eventType));
			PyDict_SetItemString(base, "type", PyLong_FromLong(ea.type));

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
					const ScrollEventArgs& sea = static_cast<const ScrollEventArgs&>(ea);
					boolconv = sea.sliderIncreasedPosition ? Py_True : Py_False; Py_INCREF(boolconv);
					PyDict_SetItemString(base, "sliderIncreasedPosition", boolconv);
				}

			}

			return base;
		}

		// -------------------- Python targetted quickGUI callback ---------
		class PythonFunctionSlot : public MemberFunctionSlot {
		    protected:
                typedef ObjectBase<PythonFunctionSlot*> Object;

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

                /// Creates a python-side object that enables python user to delete the event handle
                PyObject* getPythonSideObject() {
                    Object* obj = PyObject_New(Object, &msType);

                    obj->mInstance = this;

                    return (PyObject *)(obj);
                }

                static void dealloc(PyObject* self) {
                    // First delete the mInstance, then python object
					Object* o = reinterpret_cast<Object*>(self);

					// Decreases the shared_ptr counter
					delete o->mInstance;

					// Finally delete the object
					PyObject_Del(self);
                }

			protected:
				PyObject* mCallable;

				/// Static type definition
				static PyTypeObject msType;

				/// Name of the python type
				static char* msName;
		};



        char* PythonFunctionSlot::msName = "PythonFunctionSlot";

        PyTypeObject PythonFunctionSlot::msType = {
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			msName,                   /* char *tp_name; */
			sizeof(PythonFunctionSlot::Object),      /* int tp_basicsize; */
			0,                        /* int tp_itemsize;       /* not used much */
			PythonFunctionSlot::dealloc,   /* destructor tp_dealloc; */
			0			              /* printfunc  tp_print;   */
        };

        // -------------------- QuickGUI Manager Binder --------------------
		char* QG_ManagerBinder::msName = "GUIManager";

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
			{"addEventHandler", addEventHandler, METH_VARARGS},
			{NULL, NULL},
		};

		// ------------------------------------------
		PyObject* QG_WidgetBinder::addChild(PyObject *self, PyObject *args) {
			Object* o = python_cast<Object*>(self, &msType);

			PyObject* child;

			if (PyArg_ParseTuple(args, "O", &child)) {
				if (PyObject_TypeCheck(child, &msType)) { // Checks for parents.
				    // NOTE: All the subclasses of widget must have Py_TPFLAGS_HAVE_OBJECT and tp_base set.
				    PyErr_SetString(PyExc_TypeError, "expected a Widget object");
				    return NULL;
				}

				// This will cast to Widget, but that does not break anything
				Object* cho = python_cast<Object*>(child, &msType);

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
		PyObject* QG_WidgetBinder::addEventHandler(PyObject *self, PyObject *args) {
			Object* o = python_cast<Object*>(self, &msType);

            int evType;
			PyObject* callable;

			if (PyArg_ParseTuple(args, "iO", &evType, &callable)) {
				if (evType >= Widget::EVENT_END_OF_LIST || evType < Widget::EVENT_CHARACTER_KEY) {
				    PyErr_SetString(PyExc_TypeError, "Given event type not valid!");
                    return NULL;
				}

                try {
                    PythonFunctionSlot* slot = new PythonFunctionSlot(callable);

                    o->mInstance->addEventHandler((Widget::Event)(evType), slot);

                    PyObject* handle = slot->getPythonSideObject();

                    return handle;
                } catch(Opde::BasicException& e) {
                    PyErr_SetString(PyExc_TypeError, "Given callback handle is probably invalid!");
                    return NULL;
                }


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

