#ifndef QUICKGUIBUTTON_H
#define QUICKGUIBUTTON_H

#include "QuickGUIPrerequisites.h"
#include "QuickGUILabel.h"

namespace QuickGUI
{
	/** Represents a traditional button.
	@remarks
	The Button class requires at least 3 materials to define it's image:
	Normal State, Mouse Over, and Mouse Down.  For example, if you pass
	the constructor "sample.button" as its arguement for the material,
	the class will expect "sample.button.over" and "sample.button.down"
	to exist.  By default, buttons will handle mouse over and mouse down
	events, and change the image appropriately.
	@note
	In order to get the most use out of buttons, you will need to add
	an event handler.
	@note
	Buttons are meant to be created via the Window widget.
	*/
	class _QuickGUIExport Button :
		public Label
	{
	public:
		/** Constructor
            @param
                name The name to be given to the widget (must be unique).
            @param
                dimensions The x Position, y Position, width, and height of the widget.
			@param
				positionMode The GuiMetricsMode for the values given for the position. (absolute/relative/pixel)
			@param
				sizeMode The GuiMetricsMode for the values given for the size. (absolute/relative/pixel)
			@param
				material Ogre material defining the widget image.
			@param
				group QuadContainer containing this widget.
			@param
				ParentWidget parent widget which created this widget.
        */
		Button(const Ogre::String& name, GUIManager* gm);

		/**
		* Useful when you want to simulate the button being pressed down by the mouse.
		* If you actually want to click the mouse, use the mouse, or call onMouseButtonDown.
		*/
		virtual void applyButtonDownTexture();
		virtual void applyButtonOverTexture();
		/**
		* If supplying a method to simulate the button being pressed down, we need a method
		* to restore the button to the normal looking state.
		*/
		virtual void applyDefaultTexture();

		bool isDown();

	protected:
		virtual ~Button();

		bool mButtonDown;
	protected:
		/**
		* Event Handler for the EVENT_MOUSE_ENTER event.
		*/
		void onMouseEnters(const EventArgs& args);
		/**
		* Event Handler for the EVENT_MOUSE_LEAVE event.
		*/
		void onMouseLeaves(const EventArgs& args);
		/**
		* Event Handler for the EVENT_MOUSE_BUTTON_UP event.
		*/
		void onMouseButtonUp(const EventArgs& args);
		/**
		* Event Handler for the EVENT_MOUSE_BUTTON_DOWN event.
		*/
		virtual void onMouseButtonDown(const EventArgs& args);
	};
}

#endif
