#ifndef QUICKGUIMOUSEBUTTONID_H
#define QUICKGUIMOUSEBUTTONID_H

#include "QuickGUIPrerequisites.h"

#define NUM_MOUSE_BUTTONS (MB_Button7 + 1)

namespace QuickGUI
{
	/* Copied from OISMouse.h */
	//! Button ID for mouse devices
	enum MouseButtonID
	{
		MB_Left		=  0, 
		MB_Right		, 
		MB_Middle		,
		MB_Button3		, 
		MB_Button4		,	
		MB_Button5		, 
		MB_Button6		,	
		MB_Button7
	};
}

#endif
