/** All supported Events used in QuickGUI.
	@remarks
	Every widget supports functions to fire the events listed below.  
	These Events will go to the widget's corresponding event handlers, 
	and call user defined event handlers if they have been created.
	Events iterate through their child or parent widgets, until it
	is handled, or there are no more children/parents.
	@note
	Events are fired via the GUIManager, which interacts with injected
	inputs.
*/

#ifndef QUICKGUIEVENTARGS_H
#define QUICKGUIEVENTARGS_H

#include "OgrePrerequisites.h"
#include "OgreUTFString.h"

#include "QuickGUIPrerequisites.h"
#include "QuickGUIKeyCode.h"
#include "QuickGUIMouseButtonID.h"
#include "QuickGUIPoint.h"
#include "QuickGUIExportDLL.h"

namespace QuickGUI
{
	class Widget;
	class Text;

	/** Basic EventArgs
	*/
	class _QuickGUIExport EventArgs
	{
	public:
		typedef enum Type
		{
			TYPE_DEFAULT		=  0,
			TYPE_WIDGET				,
			TYPE_MOUSE				,
			TYPE_KEY				,
			TYPE_SCROLL				,
			TYPE_TEXT
		};
	public:
		EventArgs() : handled(false)
		{}

		~EventArgs() {}

		virtual void dummy() {} // needed to make the class polymorphic so Boost::Python can tell the arg type.

		// handlers should set this to true if they handled the event.
		bool	handled;

		Type	type;
	};

	/** Widget EventArgs
	*/
	class _QuickGUIExport WidgetEventArgs : public EventArgs
	{
	public:
		WidgetEventArgs(Widget* w) : EventArgs(), widget(w)
		{
			type = TYPE_WIDGET;
		}

		//pointer to a Widget object of relevance to the event.
		Widget*	widget;
	};

	/** Mouse EventArgs
	*/
	class _QuickGUIExport MouseEventArgs : public WidgetEventArgs
	{
	public:
		MouseEventArgs(Widget* w) : WidgetEventArgs(w) 
		{
			type = TYPE_MOUSE;
		}

		// holds current mouse position. (pixels)
		Point			position;
		// holds variation of mouse position from last mouse input
		Point			moveDelta;		
		// holds the mouse button that was down for the given event
		MouseButtonID	button;
		// holds the amount the scroll wheel has changed.
		Ogre::Real		wheelChange;	
		//! Bit field that holds status of Alt, Ctrl, Shift
		unsigned int	keyModifiers;
	};

	/** Key EventArgs
	*/
	class _QuickGUIExport KeyEventArgs : public WidgetEventArgs
	{
	public:
		KeyEventArgs(Widget* w) : WidgetEventArgs(w) 
		{
			type = TYPE_KEY;
		}

		// codepoint for the key (only used for Character inputs).
		Ogre::UTFString::unicode_char	codepoint;		
		// Scan code of key that caused event (only used for key up & down inputs.
		KeyCode							scancode;
		//! Bit field that holds status of Alt, Ctrl, Shift
		unsigned int					keyModifiers;
	};

	/** Text EventArgs
	*/
	class _QuickGUIExport TextEventArgs : public EventArgs
	{
	public:
		TextEventArgs(Text* t) : EventArgs(), text(t)
		{
			type = TYPE_TEXT;
			colorChanged = false;
			captionChanged = false;
			fontChanged = false;
		}

		bool colorChanged;
		bool captionChanged;
		bool fontChanged;

		Text* text;
	};

	/** Scroll EventArgs
	*/
	class _QuickGUIExport ScrollEventArgs : public WidgetEventArgs
	{
	public:
		ScrollEventArgs(Widget* w) : WidgetEventArgs(w), sliderIncreasedPosition(false)
		{
			type = TYPE_SCROLL;
		}

		bool sliderIncreasedPosition;
	};
}

#endif
