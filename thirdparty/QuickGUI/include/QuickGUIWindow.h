#ifndef QUICKGUIWINDOW_H
#define QUICKGUIWINDOW_H

#include "QuickGUIPanel.h"
#include "QuickGUITitleBar.h"

namespace QuickGUI
{
	/** Represents a traditional Window.
		@remarks
		The Window class is able to create all other widgets,
		and for the majority of times, will be the medium used
		to display and use other widgets.  The Window class
		requires 0-5 material definitions, depending on the
		contstructor used. (Empty Window vs Default Window)
		ex: Original Material - "sample.window"
		Required:
			"sample.window.titlebar"
			"sample.window.titlebar.button"
			"sample.window.titlebar.button.over"
			"sample.window.titlebar.button.down"
		@note
		Windows must be created by the GUIManager.
	*/
	class _QuickGUIExport Window :
		public Panel
	{
	public:
		friend class Sheet;
		friend class TitleBar;
	public:
		/** Constructor - Default Window
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
				parentWidget sheet used to create this window.
			@note
				Requires all 5 materials listed above.
			@note
				This Window will have a TitleBar
        */
		Window(const Ogre::String& name, GUIManager* gm);

		virtual void allowScrolling(bool allow);

		void bringToFront();
		
		bool getBringToFrontOnFocus();
		TitleBar* getTitleBar();

		void hideCloseButton();
		void hideTitlebar();

		// Overridden Event Handling functions
		// If user Defined Events have been created, they will be called.
		/**
		* Default Handler for the EVENT_GAIN_FOCUS event, and activates all child widgets (if exist)
		*/
		void onGainFocus(const EventArgs& args);

		void setBringToFrontOnFocus(bool BringToFront);
		void showCloseButton();
		void showTitlebar();

	protected:
		virtual ~Window();
		virtual void setQuadContainer(QuadContainer* container);
		void onMouseUpOverCloseButton(const EventArgs& args);
	protected:
		TitleBar* mTitleBar;

		bool mBringToFrontOnFocus;
	};
}

#endif
