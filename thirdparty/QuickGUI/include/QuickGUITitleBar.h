#ifndef QUICKGUITITLELBAR_H
#define QUICKGUITITLELBAR_H

#include "QuickGUIPrerequisites.h"
#include "QuickGUIButton.h"
#include "QuickGUILabel.h"

namespace QuickGUI
{
	/** Window TitleBar.
		@remarks
		Titlebars are the anchor for windows, and allow
		them to be moves around via the mouse.  The
		TitleBar widget contains a Button Widget by default,
		and is implemented to hide the window when clicked.
		TitleBar widget requires 4 materials: Default,
		Close Button, Close Button Mouse Over, Close Button
		Mouse Button Down.
		ex: Original Material - "sample.window.titlebar"
		Required:
			"sample.window.titlebar.button"
			"sample.window.titlebar.button.over"
			"sample.window.titlebar.button.down"
		@note
		The TitleBar widget should never be created manually.
		It is created as a part of the default Window widget.
		(as opposed to an empty Window, which has no TitleBar)
	*/
	class _QuickGUIExport TitleBar :
		public Label
	{
	public:
		friend class Sheet;
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
			@note
				no dimensions needed, the titlebar is always the width of the parent widget (window), and starts at 0,0.
			@note
				height it default at 0.05 absolute coordinates.	Height can be modified.
        */
		TitleBar(const Ogre::String& name, GUIManager* gm);

		/**
		* Gets a handle to the Button widget used for this Widget.
		*/
		Button* getCloseButton();
		/**
		* Hides Only the CloseButton, removing default ability to close the window
		*/
		void hideCloseButton();
		/**
		* Set true if the label's size should match it's font height and text width.
		* NOTE: AutoSize is set to true by default.  If you set this to false, you may
		*  end up with empty label's, as text that doesn't fit in the label won't be rendered.
		*/
		virtual void setAutoSize(bool autoSize);
		/**
		* Convenience method to set the text of the titlebar.
		*/
		void setCaption(const Ogre::UTFString& caption);
		virtual void setFont(const Ogre::String& fontScriptName, bool recursive = false);
		virtual void setText(const Ogre::UTFString& text);
		virtual void setWidth(Ogre::Real pixelWidth);
		/**
		* Shows the Close Button.
		*/
		void showCloseButton();

	protected:
		virtual ~TitleBar();

		Button* mCloseButton;
		Ogre::Real mRelativeButtonWidth;
	};
}

#endif
