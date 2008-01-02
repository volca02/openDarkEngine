#ifndef QUICKGUICHECKBOX_H
#define QUICKGUICHECKBOX_H

#include "QuickGUIButton.h"

namespace QuickGUI
{
	class _QuickGUIExport CheckBox :
		public Button
	{
	public:
		CheckBox(const Ogre::String& name, GUIManager* gm);
		~CheckBox();

		/**
		* Add user defined events, that will be called when a check state has changed.
		*/
		template<typename T> void addOnCheckChangedEventHandler(void (T::*function)(const EventArgs&), T* obj)
		{
			mOnCheckChangedUserEventHandlers.push_back(new MemberFunctionPointer<T>(function,obj)); 
		}
		void addOnCheckChangedEventHandler(MemberFunctionSlot* function);

		bool getChecked();

		void setChecked(bool checked);

	protected:
		bool mChecked;

		// User defined event handlers that are called when a Selection is made.
		EventHandlerArray mOnCheckChangedUserEventHandlers;
	protected:
		/**
		* Event Handler for the EVENT_MOUSE_BUTTON_DOWN event.
		*/
		virtual void onMouseButtonDown(const EventArgs& args);
	};
}

#endif
